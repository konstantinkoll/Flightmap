
// CAchievementNotification.cpp: Implementierung der Klasse CAchievementNotification
//

#include "stdafx.h"
#include "CAchievementNotification.h"
#include "Flightmap.h"


// CAchievementNotification
//

CAchievementNotification::CAchievementNotification()
	: CWnd()
{
}

BOOL CAchievementNotification::Create()
{
	CRect rect(100, 100, 250, 250);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	return CWnd::CreateEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE, className, _T(""),
		WS_BORDER | WS_DISABLED | WS_THICKFRAME | WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, NULL, 0);
}

void CAchievementNotification::UpdateFrame()
{
	// Client rectangle
	CRect rectClient;
	GetClientRect(rectClient);

	// Shadow
	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);

	// Glass frame
	BOOL IsAeroWindow = FALSE;
	if (theApp.m_AeroLibLoaded)
		theApp.zDwmIsCompositionEnabled(&IsAeroWindow);

	// Settings
	LONG cl = GetClassLong(GetSafeHwnd(), GCL_STYLE);
	cl &= ~CS_DROPSHADOW;
	if (!IsAeroWindow && bDropShadow)
		cl |= CS_DROPSHADOW;
	SetClassLong(GetSafeHwnd(), GCL_STYLE, cl);

	LONG ws = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	ws &= ~WS_THICKFRAME;
	if (IsAeroWindow)
		ws |= WS_THICKFRAME;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, ws);

	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);

	AdjustWindowRect(rectClient, ws, FALSE);

	SetWindowPos(&wndTopMost, 0, 0, rectClient.Width(), rectClient.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}


BEGIN_MESSAGE_MAP(CAchievementNotification, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCDESTROY()
	ON_WM_TIMER()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_REGISTERED_MESSAGE(theApp.m_WakeupMsg, OnWakeup)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

INT CAchievementNotification::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	UpdateFrame();

	theApp.AddFrame(this);

	AnimateWindow(1000, AW_BLEND);
	SetTimer(1, 6000, NULL);
	SetTimer(2, 7000, NULL);

	return 0;
}

void CAchievementNotification::OnDestroy()
{
	KillTimer(1);

	CWnd::OnDestroy();

	theApp.KillFrame(this);
}

void CAchievementNotification::PostNcDestroy()
{
	delete this;
}

void CAchievementNotification::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1:
		AnimateWindow(1000, AW_BLEND | AW_HIDE);
		break;
	case 2:
		PostMessage(WM_CLOSE);
		break;
	}

	CWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

LRESULT CAchievementNotification::OnThemeChanged()
{
	UpdateFrame();

	return TRUE;
}

void CAchievementNotification::OnCompositionChanged()
{
	UpdateFrame();
}

LRESULT CAchievementNotification::OnDisplayChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateFrame();

	return NULL;
}

LRESULT CAchievementNotification::OnWakeup(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 24878;
}

BOOL CAchievementNotification::OnCopyData(CWnd* /*pWnd*/, COPYDATASTRUCT* pCopyDataStruct)
{
	if (pCopyDataStruct->cbData!=sizeof(CDS_Wakeup))
		return FALSE;

	CDS_Wakeup cds = *((CDS_Wakeup*)pCopyDataStruct->lpData);
	if (cds.AppID!=theApp.m_AppID)
		return FALSE;

	theApp.OpenCommandLine(cds.Command[0] ? cds.Command : NULL);

	return TRUE;
}
