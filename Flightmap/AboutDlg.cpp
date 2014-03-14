
// AboutDlg.cpp: Implementierung der Klasse AboutDlg
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "ThreeDSettingsDlg.h"
#include "Flightmap.h"
#include <wininet.h>


// AboutDlg
//

AboutDlg::AboutDlg(CWnd* pParentWnd)
	: FMDialog(IDD_ABOUT, FMDS_Blue, pParentWnd)
{
	m_UseStatuteMiles = theApp.m_UseStatuteMiles;
	m_UseBgImages = theApp.m_UseBgImages;
	m_CaptionTop = 0;

	ENSURE(m_Logo.Load(IDB_FLIGHTMAP_128, _T("PNG")));

	SYSTEMTIME st;
	GetSystemTime(&st);
	m_pSanta = (st.wMonth==12) ? new CGdiPlusBitmapResource(IDB_SANTA, _T("PNG")) : NULL;

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
	m_Copyright.Replace(_T(" liquidFOLDERS"), _T(""));
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);
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

void AboutDlg::CheckLicenseKey()
{
	FMLicense l;

	if (FMIsLicensed(&l))
		GetDlgItem(IDC_ENTERLICENSEKEY)->ShowWindow(SW_HIDE);

	// Lizenzinformationen
	GetDlgItem(IDC_NAME)->SetWindowText(l.RegName);
	GetDlgItem(IDC_PURCHASEDATE)->SetWindowText(l.PurchaseDate);
	GetDlgItem(IDC_ID)->SetWindowText(l.PurchaseID);
	GetDlgItem(IDC_PRODUCT)->SetWindowText(l.ProductID);

	GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUANTITY)->SetWindowText(l.Quantity);
}

void AboutDlg::CheckInternetConnection()
{
	DWORD Flags;
	BOOL Connected = InternetGetConnectedState(&Flags, 0);

	GetDlgItem(IDC_EXCLUSIVE)->EnableWindow(Connected);
	GetDlgItem(IDC_UPDATENOW)->EnableWindow(Connected);
}


BEGIN_MESSAGE_MAP(AboutDlg, FMDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDD_3DSETTINGS, On3DSettings)
	ON_BN_CLICKED(IDC_EXCLUSIVE, OnExclusive)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
	ON_NOTIFY(NM_CLICK, IDC_VERSIONINFO, OnVersionInfo)
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

BOOL AboutDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	// Version
#ifdef _M_X64
#define ISET _T(" (x64)")
#else
#define ISET _T(" (x86)")
#endif

	TIMESTAMP;

	CString caption;
	m_wndVersionInfo.GetWindowText(caption);
	CString text;
	text.Format(caption, m_Version+ISET, Timestamp, m_Copyright);
	m_wndVersionInfo.SetWindowText(text);

	// Hintergrund
	m_CaptionFont.CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, FMGetApp()->GetDefaultFontFace());

	m_VersionFont.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, FMGetApp()->GetDefaultFontFace());
	m_wndVersionInfo.SetFont(&m_VersionFont);

	CDC* dc = GetDC();
	CFont* pOldFont = dc->SelectObject(&m_CaptionFont);
	INT HeightCaption = dc->GetTextExtent(_T("Wy")).cy+8;
	dc->SelectObject(&m_VersionFont);
	INT HeightVersion = dc->GetTextExtent(_T("Wy")).cy*3;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_CaptionTop = (128+(FMGetApp()->OSVersion==OS_XP ? 20 : 12)-HeightCaption-HeightVersion)/2;

	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);
	rectWnd.left = m_pSanta ? 178 : 148;
	rectWnd.top = m_CaptionTop+HeightCaption;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// Lizenz
	CheckLicenseKey();

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void AboutDlg::OnDestroy()
{
	KillTimer(1);

	if (m_pSanta)
		delete m_pSanta;

	FMDialog::OnDestroy();
}

void AboutDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	FMDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void AboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	FMDialog::OnEraseBkgnd(dc, g, rect);

	g.DrawImage(m_Logo.m_pBitmap, m_pSanta ? 39 : 9, 12, 128, 128);
	if (m_pSanta)
		g.DrawImage(m_pSanta->m_pBitmap, -6, 2);

	CRect r(rect);
	r.top = m_CaptionTop;
	r.left = m_pSanta ? 178 : 148;

	CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

	const UINT fmt = DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS;
	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Flightmap"), r, fmt);

	dc.SelectObject(pOldFont);
}

void AboutDlg::On3DSettings()
{
	ThreeDSettingsDlg dlg(this);
	dlg.DoModal();
}

void AboutDlg::OnExclusive()
{
	CString url;
	ENSURE(url.LoadString(IDS_EXCLUSIVEURL));

	ShellExecute(GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
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

void AboutDlg::OnVersionInfo(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CString url;
	ENSURE(url.LoadString(IDS_ABOUTURL));

	ShellExecute(GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);

	*pResult = 0;
}

void AboutDlg::OnEnterLicenseKey()
{
	FMLicenseDlg dlg(this);
	dlg.DoModal();

	CheckLicenseKey();
}
