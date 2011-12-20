
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
	if (m_GlobeView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
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

	m_GlobeView.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CGlobeWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
END_MESSAGE_MAP()

INT CGlobeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMainWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS, 1);

	/*m_pDialogMenuBar->AddMenuLeft(IDM_FILE);
	m_pDialogMenuBar->AddMenuLeft(IDM_EDIT);
	m_pDialogMenuBar->AddMenuLeft(IDM_MAP);
	m_pDialogMenuBar->AddMenuLeft(IDM_GLOBE);
	m_pDialogMenuBar->AddMenuLeft(IDM_GOOGLEEARTH);
	m_pDialogMenuBar->AddMenuLeft(IDM_STATISTICS);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 0);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 1);*/
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 2);
	//m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 3);

	m_GlobeView.Create(this, 3);

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

	m_GlobeView.SetFocus();
}

LRESULT CGlobeWnd::OnRequestSubmenu(WPARAM wParam, LPARAM /*lParam*/)
{
	CDialogMenuPopup* pPopup = new CDialogMenuPopup();

	/*switch ((UINT)wParam)
	{
	case IDM_FILE:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCommand(IDM_FILE_NEW, 0, CDMB_MEDIUM);
		pPopup->AddSubmenu(IDM_FILE_OPEN, 2, CDMB_MEDIUM, TRUE);
		pPopup->AddCommand(IDM_FILE_SAVE, 3, CDMB_MEDIUM);
		pPopup->AddSubmenu(IDM_FILE_SAVEAS, 4, CDMB_MEDIUM, TRUE);
		pPopup->AddSeparator();
		pPopup->AddSubmenu(IDM_FILE_PRINT, 5, CDMB_MEDIUM, TRUE);
		pPopup->AddSubmenu(IDM_FILE_PREPARE, 9, CDMB_MEDIUM);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_FILE_CLOSE, 13, CDMB_MEDIUM);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCommand(IDM_FILE_QUIT, 14, CDMB_SMALL);
		break;
	case IDM_FILE_OPEN:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCaption(IDS_SAMPLEITINERARIES);
		pPopup->AddCommand(IDM_FILE_NEWSAMPLE1, 1, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_NEWSAMPLE2, 1, CDMB_LARGE);
		pPopup->AddCaption(IDS_RECENTFILES);
		break;
	case IDM_FILE_SAVEAS:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCaption(IDS_SAVECOPY);
		pPopup->AddFileType(IDM_FILE_SAVEAS_AIRX, _T(".airx"), CDMB_LARGE);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_FILE_SAVEAS_CSV, _T(".csv"), CDMB_LARGE);
		pPopup->AddFileType(IDM_FILE_SAVEAS_ICS, _T(".ics"), CDMB_LARGE);
		pPopup->AddFileType(IDM_FILE_SAVEAS_TXT, _T(".txt"), CDMB_LARGE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_FILE_SAVEAS_OTHER, 4, CDMB_LARGE);
		break;
	case IDM_FILE_PRINT:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCaption(IDS_PRINTPREVIEW);
		pPopup->AddCommand(IDM_FILE_PRINT, 5, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_PRINT_QUICK, 6, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_PRINT_PREVIEW, 7, CDMB_LARGE);
		break;
	case IDM_FILE_PREPARE:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCaption(IDS_PREPAREDISTRIBUTION);
		pPopup->AddCommand(IDM_FILE_PREPARE_PROPERTIES, 10, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_PREPARE_INSPECT, 11, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_PREPARE_ATTACHMENTS, 12, CDMB_LARGE);
		break;
	case IDM_EDIT:
		pPopup->Create(this, IDB_MENUEDIT_32, IDB_MENUEDIT_16);
		pPopup->AddCommand(IDM_EDIT_CUT, 0, CDMB_SMALL);
		pPopup->AddCommand(IDM_EDIT_COPY, 1, CDMB_SMALL);
		pPopup->AddSubmenu(IDM_EDIT_PASTE, 2, CDMB_SMALL, TRUE);
		pPopup->AddCommand(IDM_EDIT_DELETE, 4, CDMB_SMALL);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_EDIT_SELECTALL, 5, CDMB_SMALL);
		pPopup->AddSeparator();
		pPopup->AddSubmenu(IDM_EDIT_GOTO, 6, CDMB_SMALL, TRUE);
		break;
	case IDM_EDIT_PASTE:
		pPopup->Create(this, IDB_MENUEDIT_32, IDB_MENUEDIT_16);
		pPopup->AddCommand(IDM_EDIT_INSERT_FLIGHT, 3, CDMB_LARGE);
		pPopup->AddCommand(IDM_EDIT_INSERT_ROUTE, 3, CDMB_LARGE);
		pPopup->AddFileType(IDM_EDIT_INSERT_ITINERARY, _T(".airx"), CDMB_LARGE);
		break;
	case IDM_EDIT_GOTO:
		pPopup->Create(this, IDB_MENUEDIT_32, IDB_MENUEDIT_16);
		pPopup->AddCommand(IDM_EDIT_GOTO_FIRST, 7, CDMB_SMALL);
		pPopup->AddCommand(IDM_EDIT_GOTO_LAST, 8, CDMB_SMALL);
		break;
	case IDM_MAP:
		pPopup->Create(this, IDB_MENUGLOBE_32, IDB_MENUGLOBE_16);
		pPopup->AddSubmenu(IDM_MAP_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		break;
	case IDM_MAP_OPEN:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_MAP_EXPORT_BMP, _T(".bmp"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_JPG, _T(".jpg"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_PNG, _T(".png"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_TIFF, _T(".tiff"), CDMB_LARGE);
		break;
	case IDM_GLOBE:
		pPopup->Create(this, IDB_MENUGLOBE_32, IDB_MENUGLOBE_16);
		pPopup->AddCommand(IDM_GLOBE_OPEN, 0, CDMB_LARGE);
		pPopup->AddSeparator(TRUE);
		break;
	case IDM_GOOGLEEARTH:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddSubmenu(IDM_GOOGLEEARTH_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		break;
	case IDM_GOOGLEEARTH_OPEN:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_GOOGLEEARTH_EXPORT, _T(".kml"), CDMB_LARGE);
		break;
	}*/

	if (!pPopup->HasItems())
	{
		delete pPopup;
		pPopup = NULL;
	}

	return (LRESULT)pPopup;
}