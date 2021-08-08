
// FMAboutDialog.cpp: Implementierung der Klasse FMAboutDialog
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <wininet.h>


// FMAboutDialog
//

#define COMPILE_HOUR       (((__TIME__[0]-'0')*10)+(__TIME__[1]-'0'))
#define COMPILE_MINUTE     (((__TIME__[3]-'0')*10)+(__TIME__[4]-'0'))
#define COMPILE_SECOND     (((__TIME__[6]-'0')*10)+(__TIME__[7]-'0'))
#define COMPILE_YEAR       ((((__DATE__[7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__[9]-'0'))*10+(__DATE__[10]-'0'))
#define COMPILE_MONTH      ((__DATE__[2]=='n' ? (__DATE__[1] == 'a' ? 0 : 5)\
							: __DATE__[2]=='b' ? 1 \
							: __DATE__[2]=='r' ? (__DATE__[0] == 'M' ? 2 : 3) \
							: __DATE__[2]=='y' ? 4 \
							: __DATE__[2]=='l' ? 6 \
							: __DATE__[2]=='g' ? 7 \
							: __DATE__[2]=='p' ? 8 \
							: __DATE__[2]=='t' ? 9 \
							: __DATE__[2]=='v' ? 10 : 11)+1)
#define COMPILE_DAY        ((__DATE__[4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))

UINT FMAboutDialog::m_LastTab = 0;

FMAboutDialog::FMAboutDialog(USHORT BackgroundTabMask, CWnd* pParentWnd)
	: FMTabbedDialog(IDS_ABOUT, pParentWnd, &m_LastTab, TRUE)
{
	m_BackgroundTabMask = BackgroundTabMask;

	// Compile time
	SYSTEMTIME SystemTime;
	ZeroMemory(&SystemTime, sizeof(SystemTime));

	SystemTime.wDay = COMPILE_DAY;
	SystemTime.wMonth = COMPILE_MONTH;
	SystemTime.wYear = COMPILE_YEAR;
	SystemTime.wHour = COMPILE_HOUR;
	SystemTime.wMinute = COMPILE_MINUTE;
	SystemTime.wSecond = COMPILE_SECOND;

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &SystemTime, NULL, m_BuildInfo, 256);
	wcscat_s(m_BuildInfo, 256, L", ");

	WCHAR tmpStr[256];
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &SystemTime, NULL, tmpStr, 256);
	wcscat_s(m_BuildInfo, 256, tmpStr);

	// Special icons
	GetLocalTime(&SystemTime);

	p_AppLogo = FMGetApp()->GetCachedResourceImage((SystemTime.wMonth==3) && (SystemTime.wDay==17) ? IDB_STPATRICK : IDB_FLIGHTMAP_128);
	p_SantaHat = (SystemTime.wMonth==12) ? FMGetApp()->GetCachedResourceImage(IDB_SANTA) : NULL;

	// Application name
	ENSURE(m_AppName.LoadString(IDR_APPLICATION));

	// Copyright
	GetFileVersion(AfxGetInstanceHandle(), m_Version, &m_Copyright);

	m_Copyright.Replace(_T(" liquidFOLDERS"), _T(""));
}

void FMAboutDialog::DoDataExchange(CDataExchange* pDX)
{
	FMTabbedDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);
	DDX_Control(pDX, IDC_ENABLEAUTOUPDATE, m_wndAutoUpdate);

	// Update settings
	BOOL EnableAutoUpdate;
	INT UpdateCheckInterval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		FMGetApp()->WriteUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);
	}
	else
	{
		FMGetApp()->GetUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		OnEnableAutoUpdate();
	}
}

void FMAboutDialog::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	FMTabbedDialog::PaintOnBackground(dc, g, rectLayout);

	// Only draw custom background on "General" and "License" tab
	if (ShowBackgroundOnTab(m_CurrentTab))
	{
		// Icon
		g.DrawImage(p_AppLogo, rectLayout.left+m_ptAppLogo.x, rectLayout.top+m_ptAppLogo.y);

		if (p_SantaHat)
			g.DrawImage(p_SantaHat, rectLayout.left+m_ptAppLogo.x-39, rectLayout.top+m_ptAppLogo.y-8);

		// Caption
		CRect rectText(rectLayout);
		rectText.top += BACKSTAGEBORDER+128+2;

		CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

		dc.SetTextColor(IsCtrlThemed() ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_AppName, rectText, DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		dc.SelectObject(pOldFont);
	}
}

BOOL FMAboutDialog::InitSidebar(LPSIZE pszTabArea)
{
	if (!FMTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_ABOUT_GENERAL, pszTabArea);

	return TRUE;
}

BOOL FMAboutDialog::InitDialog()
{
	// Version info
#ifdef _M_X64
	const UINT Bit = 64;
#else
	const UINT Bit = 32;
#endif

	CString Caption;
	m_wndVersionInfo.GetWindowText(Caption);

	CString Text;
	Text.Format(Caption, m_Version, Bit, m_Copyright, m_BuildInfo);

	m_wndVersionInfo.SetWindowText(Text);

	// Clone version info to other tabs with background
	ShowControlOnTabs(m_wndVersionInfo, m_BackgroundTabMask);

	// Background
	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(rectWnd);
	ScreenToClient(rectWnd);

	const INT WindowHeight = rectWnd.Height();

	const INT LineHeight = WindowHeight/24;
	const INT CaptionHeight = 4*LineHeight;
	const INT VersionHeight = 2*LineHeight;

	CRect rectLayout;
	GetLayoutRect(rectLayout);

	m_CaptionFont.CreateFont(CaptionHeight, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("DIN Mittelschrift"));
	m_VersionFont.CreateFont(VersionHeight);
	m_wndVersionInfo.SetFont(&m_VersionFont);

	m_ptAppLogo.x = rectWnd.left-rectLayout.left+(rectWnd.Width()-p_AppLogo->GetWidth())/2;
	m_ptAppLogo.y = BACKSTAGEBORDER;

	rectWnd.top = rectLayout.top+2*BACKSTAGEBORDER+128+CaptionHeight+4;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return FMTabbedDialog::InitDialog();
}

void FMAboutDialog::CheckInternetConnection()
{
	DWORD Flags;
	GetDlgItem(IDC_UPDATENOW)->EnableWindow(InternetGetConnectedState(&Flags, 0));
}


BEGIN_MESSAGE_MAP(FMAboutDialog, FMTabbedDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()

	ON_NOTIFY(NM_CLICK, IDC_VERSIONINFO, OnVersionInfo)
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
END_MESSAGE_MAP()

void FMAboutDialog::OnDestroy()
{
	KillTimer(1);

	FMTabbedDialog::OnDestroy();
}

void FMAboutDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	FMTabbedDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void FMAboutDialog::OnVersionInfo(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ShellExecute(GetSafeHwnd(), _T("open"), CString((LPCSTR)IDS_ABOUTURL), NULL, NULL, SW_SHOWNORMAL);

	*pResult = 0;
}

void FMAboutDialog::OnEnableAutoUpdate()
{
	const BOOL Enabled = m_wndAutoUpdate.GetCheck();

	GetDlgItem(IDC_CHECKDAILY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKWEEKLY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKMONTHLY)->EnableWindow(Enabled);
}

void FMAboutDialog::OnUpdateNow()
{
	FMGetApp()->CheckForUpdate(TRUE, this);
}
