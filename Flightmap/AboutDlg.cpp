
// AboutDlg.cpp: Implementierung der Klasse AboutDlg
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "ThreeDSettingsDlg.h"
#include "Flightmap.h"


// AboutDlg
//

AboutDlg::AboutDlg(CWnd* pParent)
	: FMDialog(IDD_ABOUT, FMDS_Blue, pParent)
{
	m_UseStatuteMiles = theApp.m_UseStatuteMiles;
	m_UseBgImages = theApp.m_UseBgImages;

	m_pLogo = new CGdiPlusBitmapResource();
	ENSURE(m_pLogo->Load(IDB_FLIGHTMAP, _T("PNG"), AfxGetResourceHandle()));

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_NAUTICALMILES, m_UseStatuteMiles);
	DDX_Check(pDX, IDC_BGIMAGES, m_UseBgImages);

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

void AboutDlg::CheckLicenseKey(FMLicense* License)
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


BEGIN_MESSAGE_MAP(AboutDlg, FMDialog)
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDD_3DSETTINGS, On3DSettings)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
END_MESSAGE_MAP()

BOOL AboutDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	// Lizenz
	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void AboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	FMDialog::OnEraseBkgnd(dc, g, rect);

	CRect r(rect);
	r.top = 15;
	r.left = 148;

	CFont font1;
	font1.CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, theApp.GetDefaultFontFace());
	CFont* pOldFont = dc.SelectObject(&font1);

	const UINT fmt = DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS;
	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Flightmap"), r, fmt);
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

	dc.DrawText(_T("Version ")+m_Version+ISET, r, fmt);
	r.top += 25;

	TIMESTAMP;
	dc.DrawText(_T("Built ")+Timestamp, r, fmt);
	r.top += 25;

	dc.DrawText(m_Copyright, r, fmt);

	dc.SelectObject(pOldFont);
}

void AboutDlg::On3DSettings()
{
	ThreeDSettingsDlg dlg(this);
	dlg.DoModal();
}

void AboutDlg::OnEnableAutoUpdate()
{
	BOOL Enabled = ((CButton*)GetDlgItem(IDC_ENABLEAUTOUPDATE))->GetCheck();

	GetDlgItem(IDC_CHECKDAILY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKWEEKLY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKMONTHLY)->EnableWindow(Enabled);
}

void AboutDlg::OnUpdateNow()
{
	FMCheckForUpdate(TRUE, this);
}
