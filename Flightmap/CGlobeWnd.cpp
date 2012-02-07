
// CGlobeWnd.cpp: Implementierung der Klasse CGlobeWnd
//

#include "stdafx.h"
#include "CLoungeView.h"
#include "CGlobeWnd.h"
#include "Flightmap.h"


// CGlobeWnd
//

CGlobeWnd::CGlobeWnd()
	: CMainWindow()
{
	m_hIcon = NULL;
}

CGlobeWnd::~CGlobeWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CGlobeWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_GLOBE);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CMainWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CGlobeWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndGlobeView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CMainWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CGlobeWnd::AdjustLayout()
{
	CMainWindow::AdjustLayout();

	CRect rect;
	GetClientRect(rect);

	if (m_pDialogMenuBar)
		rect.top += m_pDialogMenuBar->GetPreferredHeight();

	m_wndGlobeView.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CGlobeWnd::SetFlights(CKitchen* pKitchen)
{
	m_wndGlobeView.SetFlights(pKitchen);
}


BEGIN_MESSAGE_MAP(CGlobeWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
	ON_MESSAGE_VOID(WM_3DSETTINGSCHANGED, On3DSettingsChanged)

	ON_COMMAND(IDM_GLOBEWND_CLOSE, OnGlobeWndClose)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBEWND_CLOSE, IDM_GLOBEWND_CLOSE, OnUpdateGlobeWndCommands)
END_MESSAGE_MAP()

INT CGlobeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMainWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS, 1);

	m_pDialogMenuBar->AddMenuLeft(IDM_GLOBEWND);
	m_pDialogMenuBar->AddMenuLeft(IDM_GLOBEVIEW);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 0);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 1);
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 2);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 3);

	m_wndGlobeView.Create(this, 2);

	theApp.AddFrame(this);

	return 0;
}

void CGlobeWnd::OnDestroy()
{
	CMainWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CGlobeWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	m_wndGlobeView.SetFocus();
}

LRESULT CGlobeWnd::OnRequestSubmenu(WPARAM wParam, LPARAM /*lParam*/)
{
	CDialogMenuPopup* pPopup = new CDialogMenuPopup();

	switch ((UINT)wParam)
	{
	case IDM_GLOBEWND:
		pPopup->Create(this, IDB_MENUGLOBEWND_32, IDB_MENUGLOBEWND_16);
		pPopup->AddCommand(IDM_GLOBEWND_CLOSE, 0, CDMB_MEDIUM);
		break;
	case IDM_GLOBEVIEW:
		pPopup->Create(this, IDB_MENUGLOBEVIEW_32, IDB_MENUGLOBEVIEW_16);
		pPopup->AddCommand(IDM_GLOBEVIEW_JUMPTOLOCATION, 0, CDMB_LARGE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_GLOBEVIEW_ZOOMIN, 1, CDMB_SMALL, FALSE);
		pPopup->AddCommand(IDM_GLOBEVIEW_ZOOMOUT, 2, CDMB_SMALL, FALSE);
		pPopup->AddCommand(IDM_GLOBEVIEW_AUTOSIZE, 3, CDMB_SMALL);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_COLORS);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_SPOTS);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_AIRPORTNAMES);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_GPS);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_FLIGHTCOUNT);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_GLOBEVIEW_VIEWPORT);
		pPopup->AddCheckbox(IDM_GLOBEVIEW_CROSSHAIRS);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_GLOBEVIEW_3DSETTINGS, 4, CDMB_SMALL);
		break;
	}

	if (!pPopup->HasItems())
	{
		delete pPopup;
		pPopup = NULL;
	}

	return (LRESULT)pPopup;
}

void CGlobeWnd::On3DSettingsChanged()
{
	m_wndGlobeView.UpdateViewOptions();
}


void CGlobeWnd::OnGlobeWndClose()
{
	SendMessage(WM_CLOSE);
}

void CGlobeWnd::OnUpdateGlobeWndCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
