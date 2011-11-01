
// CAboutDlg.cpp: Implementierung der Klasse CAboutDlg
//

#include "stdafx.h"
#include "CAboutDlg.h"
#include "Flightmap.h"


// CAboutDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CAboutDlg::CAboutDlg(CWnd* pParent)
	: FMDialog(IDD_ABOUT, FMDS_Blue, pParent)
{
	m_UseStatuteMiles = theApp.m_UseStatuteMiles;
	m_ReduceVisuals = theApp.m_ReduceVisuals;

	m_pLogo = new CGdiPlusBitmapResource();
	ENSURE(m_pLogo->Load(IDB_FLIGHTMAP, _T("PNG"), AfxGetResourceHandle()));

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_NAUTICALMILES, m_UseStatuteMiles);
	DDX_Check(pDX, IDC_REDUCEVISUALS, m_ReduceVisuals);

	BOOL EnableAutoUpdate;
	INT Interval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

		theApp.SetUpdateSettings(EnableAutoUpdate, Interval);
	}
	else
	{
		theApp.GetUpdateSettings(&EnableAutoUpdate, &Interval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

		OnEnableAutoUpdate();
	}
}

void CAboutDlg::CheckLicenseKey(FMLicense* License)
{
	FMLicense l;
	if (!License)
		License = &l;

	FMDialog::CheckLicenseKey(License);

	// Lizenzinformationen
	GetDlgItem(IDC_NAME)->SetWindowText(License->RegName);
	GetDlgItem(IDC_PURCHASEDATE)->SetWindowText(License->PurchaseDate);
	GetDlgItem(IDC_ID)->SetWindowText(License->PurchaseID);
	GetDlgItem(IDC_PRODUCT)->SetWindowText(License->ProductID);

	GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUANTITY)->SetWindowText(License->Quantity);
}


BEGIN_MESSAGE_MAP(CAboutDlg, FMDialog)
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	// Lizenz
	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CAboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	FMDialog::OnEraseBkgnd(dc, g, rect);

	CRect r(rect);
	r.top = 16;
	r.left = 160;

	CFont font1;
	font1.CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, theApp.GetDefaultFontFace());
	CFont* pOldFont = dc.SelectObject(&font1);

	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Flightmap"), r, 0);
	r.top += 45;

	CFont font2;
	font2.CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, theApp.GetDefaultFontFace());
	dc.SelectObject(&font2);

#ifdef _M_X64
#define ISET _T(" (x64)")
#else
#define ISET _T(" (x32)")
#endif

	dc.DrawText(_T("Version ")+m_Version+ISET, r, 0);
	r.top += 26;

	TIMESTAMP;
	dc.DrawText(_T("Built ")+Timestamp, r, 0);
	r.top += 26;

	dc.DrawText(m_Copyright, r, 0);

	dc.SelectObject(pOldFont);
}

void CAboutDlg::OnEnableAutoUpdate()
{
	BOOL Enabled = ((CButton*)GetDlgItem(IDC_ENABLEAUTOUPDATE))->GetCheck();

	GetDlgItem(IDC_CHECKDAILY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKWEEKLY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKMONTHLY)->EnableWindow(Enabled);
}

void CAboutDlg::OnUpdateNow()
{
	//LFCheckForUpdate(TRUE, this);
}
