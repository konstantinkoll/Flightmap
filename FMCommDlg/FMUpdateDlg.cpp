
// FMUpdateDlg.cpp: Implementierung der Klasse FMUpdateDlg
//

#include "stdafx.h"
#include "FMUpdateDlg.h"
#include "MenuIcons.h"
#include <wininet.h>


// Use a GUID to uniquely identify the tray icon: {FD604358-411B-4D4C-88F9-D5AAF961A411}
static const GUID TrayIcon = { 0xFD604358, 0x411B, 0x4D4C, { 0x88, 0xF9, 0xD5, 0xAA, 0xF9, 0x61, 0xA4, 0x11 } };


// FMUpdateDlg
//

#define WM_TRAYMENU     WM_USER+10

FMUpdateDlg::FMUpdateDlg(CString Version, CString MSN, CWnd* pParentWnd)
	: FMDialog(IDD_UPDATE, pParentWnd)
{
	m_NotificationWindow = (pParentWnd==NULL);
	m_CaptionTop = m_IconTop = 0;
	m_Connected = TRUE;

	p_Logo = p_App->GetCachedResourceImage(IDB_FLIGHTMAP_64, _T("PNG"));

	m_Version = Version;
	m_MSN = MSN;
	ENSURE(m_AppName.LoadString(IDR_APPLICATION));
}

void FMUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);
	DDX_Control(pDX, IDC_IGNOREUPDATE, m_wndIgnoreUpdate);
}

void FMUpdateDlg::UpdateDownloadButton()
{
	GetDlgItem(IDOK)->EnableWindow(m_Connected && !m_wndIgnoreUpdate.GetCheck());
}

void FMUpdateDlg::CheckInternetConnection()
{
	DWORD Flags;
	m_Connected = InternetGetConnectedState(&Flags, 0);

	UpdateDownloadButton();
}

void FMUpdateDlg::UpdateFrame(BOOL bMove)
{
	if (!m_NotificationWindow)
		return;

	// Client rectangle
	CRect rectClient;
	GetClientRect(rectClient);

	// Shadow
	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);

	// Glass frame
	BOOL IsAeroWindow = FALSE;
	if (p_App->m_AeroLibLoaded)
		p_App->zDwmIsCompositionEnabled(&IsAeroWindow);

	// Settings
	LONG cl = GetClassLong(GetSafeHwnd(), GCL_STYLE);
	cl &= ~CS_DROPSHADOW;
	if (!IsAeroWindow && bDropShadow)
		cl |= CS_DROPSHADOW;
	SetClassLong(GetSafeHwnd(), GCL_STYLE, cl);

	LONG ws = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	ws &= ~(WS_CAPTION | WS_DLGFRAME | WS_THICKFRAME);
	ws |= WS_POPUPWINDOW;
	if (IsAeroWindow)
		ws |= WS_THICKFRAME;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, ws);

	LONG es = GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
	es &= ~WS_EX_DLGMODALFRAME;
	es |= WS_EX_TOOLWINDOW;
	SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, es);

	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	AdjustWindowRectEx(rectClient, ws, FALSE, es);

	if (bMove)
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);

		CRect rectScreen;
		if (GetMonitorInfo(MonitorFromPoint(CPoint(0, 0), MONITOR_DEFAULTTONEAREST), &mi))
		{
			rectScreen = mi.rcWork;
		}
		else
		{
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
		}

		const INT PosX = rectScreen.right-rectClient.Width()+1;
		const INT PosY = rectScreen.bottom-rectClient.Height()+1;
		SetWindowPos(&wndTopMost, PosX, PosY, rectClient.Width(), rectClient.Height(), SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(&wndTopMost, 0, 0, rectClient.Width(), rectClient.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}
}

BOOL FMUpdateDlg::AddTrayIcon()
{
	INT sz = GetSystemMetrics(SM_CXSMICON);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetSafeHwnd();
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID | NIF_SHOWTIP;
	nid.guidItem = TrayIcon;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uCallbackMessage = WM_TRAYMENU;
	nid.hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, sz, sz, LR_DEFAULTCOLOR);
	GetWindowText(nid.szTip, 128);
	Shell_NotifyIcon(NIM_ADD, &nid);

	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL FMUpdateDlg::RemoveTrayIcon()
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetSafeHwnd();
	nid.uFlags = NIF_GUID;
	nid.guidItem = TrayIcon;

	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

void FMUpdateDlg::ShowMenu()
{
	CMenu Menu;
	Menu.LoadMenu(IDM_UPDATE);
	ASSERT_VALID(&Menu);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	SetMenuItemBitmap(*pPopup, 0, HBMMENU_POPUP_RESTORE);
	pPopup->SetDefaultItem(0, TRUE);

	POINT pos;
	GetCursorPos(&pos);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
}


BEGIN_MESSAGE_MAP(FMUpdateDlg, FMDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_BN_CLICKED(IDC_IGNOREUPDATE, OnIgnoreUpdate)
	ON_NOTIFY(NM_CLICK, IDC_HIDE, OnHide)
	ON_BN_CLICKED(IDOK, OnDownload)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_TRAYMENU, OnTrayMenu)
	ON_COMMAND(IDM_UPDATE_RESTORE, OnRestore)
END_MESSAGE_MAP()

BOOL FMUpdateDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	// Stil
	if (m_NotificationWindow)
	{
		p_App->PlayWarningSound();

		UpdateFrame(TRUE);
	}
	else
	{
		p_App->PlayStandardSound();

		GetDlgItem(IDC_IGNOREUPDATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HIDE)->ShowWindow(SW_HIDE);
	}

	// Version
	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);

	CString caption;
	m_wndVersionInfo.GetWindowText(caption);
	CString text;
	text.Format(caption, m_Version, m_MSN);
	m_wndVersionInfo.SetWindowText(text);

	// Hintergrund
	const INT Height = rectWnd.Height();
	const INT LineGap = Height/6;
	const INT HeightCaption = 4*LineGap;
	const INT HeightVersion = 2*LineGap;

	m_CaptionFont.CreateFont(HeightCaption, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Letter Gothic"));

	m_VersionFont.CreateFont(HeightVersion, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, p_App->GetDefaultFontFace());
	m_wndVersionInfo.SetFont(&m_VersionFont);

	m_CaptionTop = rectWnd.top+(rectWnd.bottom-HeightCaption-HeightVersion)/2-9;
	m_IconTop = rectWnd.top+(rectWnd.bottom-62)/2-8;

	rectWnd.left = 82;
	rectWnd.top = m_CaptionTop+HeightCaption;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMUpdateDlg::OnDestroy()
{
	KillTimer(1);
	RemoveTrayIcon();

	FMDialog::OnDestroy();
}

void FMUpdateDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	FMDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void FMUpdateDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	FMDialog::OnEraseBkgnd(dc, g, rect);

	g.DrawImage(p_Logo->m_pBitmap, 9, m_IconTop);

	CRect r(rect);
	r.top = m_CaptionTop;
	r.left = 82-2;

	CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

	const UINT fmt = DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS;
	dc.SetTextColor((IsCtrlThemed() && p_App->m_UseBgImages) ? 0x000000 : 0x606060);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_AppName, r, fmt);

	dc.SelectObject(pOldFont);
}

LRESULT FMUpdateDlg::OnNcHitTest(CPoint point)
{
	SHORT LButtonDown = GetAsyncKeyState(VK_LBUTTON);
	LRESULT uHitTest = FMDialog::OnNcHitTest(point);
	return m_NotificationWindow ? ((uHitTest>=HTLEFT) && (uHitTest<=HTBOTTOMRIGHT)) ? HTCAPTION : ((uHitTest==HTCLIENT) && (LButtonDown & 0x8000)) ? HTCAPTION : uHitTest : uHitTest;
}

LRESULT FMUpdateDlg::OnThemeChanged()
{
	UpdateFrame();

	return FMDialog::OnThemeChanged();
}

void FMUpdateDlg::OnCompositionChanged()
{
	UpdateFrame();
}

LRESULT FMUpdateDlg::OnDisplayChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateFrame(TRUE);

	return NULL;
}

void FMUpdateDlg::OnIgnoreUpdate()
{
	UpdateDownloadButton();
}

void FMUpdateDlg::OnHide(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnCancel();

	*pResult = 0;
}

void FMUpdateDlg::OnDownload()
{
	CString url;
	ENSURE(url.LoadString(IDS_UPDATEURL));
	ShellExecute(GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);

	EndDialog(IDOK);
}

void FMUpdateDlg::OnCancel()
{
	if (((CButton*)GetDlgItem(IDC_IGNOREUPDATE))->GetCheck() || !m_NotificationWindow)
	{
		EndDialog(IDCANCEL);
	}
	else
		if (AddTrayIcon())
			ShowWindow(SW_HIDE);
}

LRESULT FMUpdateDlg::OnTrayMenu(WPARAM /*wParam*/, LPARAM lParam)
{
	switch (LOWORD(lParam))
	{
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		OnRestore();
		break;
	case WM_RBUTTONUP:
		ShowMenu();
		break;
	}

	return NULL;
}

void FMUpdateDlg::OnRestore()
{
	RemoveTrayIcon();
	ShowWindow(SW_SHOW);
}
