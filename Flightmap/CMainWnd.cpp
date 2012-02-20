
// CMainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "CGoogleEarthFile.h"
#include "CKitchen.h"
#include "CLoungeView.h"
#include "CMainWnd.h"
#include "CMapWnd.h"
#include "CGlobeWnd.h"
#include "Flightmap.h"


// CMainWnd
//

CMainWnd::CMainWnd()
	: CMainWindow()
{
	m_hIcon = NULL;
	m_pWndMainView = NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CMainWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CMainWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_pWndMainView)
		if (m_pWndMainView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	return CMainWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainWnd::AdjustLayout()
{
	CMainWindow::AdjustLayout();

	if (!m_pWndMainView)
		return;

	CRect rect;
	GetClientRect(rect);

	if (m_pDialogMenuBar)
		rect.top += m_pDialogMenuBar->GetPreferredHeight();

	m_pWndMainView->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CMainWnd::OpenMainView(BOOL Empty)
{
	if (m_pWndMainView)
	{
		m_pWndMainView->DestroyWindow();
		delete m_pWndMainView;
	}

	if (Empty)
	{
		m_pWndMainView = new CLoungeView();
		((CLoungeView*)m_pWndMainView)->Create(this, 2);
	}
	else
	{
		m_pWndMainView = NULL;	// TODO
	}

	AdjustLayout();
	SetFocus();
}

CKitchen* CMainWnd::GetKitchen(BOOL Selected)
{
	CKitchen* pKitchen = new CKitchen();

	pKitchen->AddFlight("DUS", "FRA", (COLORREF)-1);
	pKitchen->AddFlight("FRA", "JFK", 0x0000FF);
	pKitchen->AddFlight("EWR", "SFO", (COLORREF)-1);
	pKitchen->AddFlight("SFO", "MUC", (COLORREF)-1);
	pKitchen->AddFlight("SFO", "HNL", (COLORREF)-1);
	pKitchen->AddFlight("HNL", "NRT", (COLORREF)-1);
	pKitchen->AddFlight("NRT", "JNB", (COLORREF)-1);
	pKitchen->AddFlight("CPT", "JNB", (COLORREF)-1);
	pKitchen->AddFlight("EZE", "CPT", (COLORREF)-1);
	pKitchen->AddFlight("MUC", "DUS", (COLORREF)-1);

	return pKitchen;
}

BOOL CMainWnd::ExportKML(CString FileName, BOOL UseColors, BOOL Clamp, BOOL Selected)
{
	CGoogleEarthFile f;

	if (!f.Open(FileName, _T("Test")))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
		return FALSE;
	}
	else
	{
		BOOL Res = FALSE;
		CKitchen* pKitchen = GetKitchen(Selected);

		try
		{
			f.WriteRoutes(pKitchen, UseColors, Clamp, FALSE);

			CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc();
			while (pPair)
			{
				f.WriteAirport(pPair->value);

				pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair);
			}

			f.Close();
			Res = TRUE;
		}
		catch(CFileException ex)
		{
			FMErrorBox(IDS_DRIVENOTREADY);
			f.Close();
		}

		delete pKitchen;
		return Res;
	}
}


BEGIN_MESSAGE_MAP(CMainWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
	ON_MESSAGE(WM_GALLERYCHANGED, OnGalleryChanged)
	ON_REGISTERED_MESSAGE(theApp.msgUseBgImagesChanged, OnUseBgImagesChanged)

	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_QUIT, OnFileQuit)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_NEW, IDM_FILE_QUIT, OnUpdateFileCommands)

	ON_COMMAND(IDM_MAP_OPEN, OnMapOpen)
	ON_COMMAND(IDM_MAP_CENTERATLANTIC, OnMapCenterAtlantic)
	ON_COMMAND(IDM_MAP_CENTERPACIFIC, OnMapCenterPacific)
	ON_COMMAND(IDM_MAP_SHOWFLIGHTROUTES, OnMapShowFlightRoutes)
	ON_COMMAND(IDM_MAP_STRAIGHTLINES, OnMapStraightLines)
	ON_COMMAND(IDM_MAP_USECOLORS, OnMapUseColors)
	ON_COMMAND(IDM_MAP_SHOWLOCATIONS, OnMapShowLocations)
	ON_COMMAND(IDM_MAP_SHOWIATACODES, OnMapShowIATACodes)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAP_OPEN, IDM_MAP_IATAOUTERCOLOR, OnUpdateMapCommands)

	ON_COMMAND(IDM_GLOBE_OPEN, OnGlobeOpen)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBE_OPEN, IDM_GLOBE_OPEN, OnUpdateGlobeCommands)

	ON_COMMAND(IDM_GOOGLEEARTH_OPEN, OnGoogleEarthOpen)
	ON_COMMAND(IDM_GOOGLEEARTH_EXPORT, OnGoogleEarthExport)
	ON_COMMAND(IDM_GOOGLEEARTH_COLORS, OnGoogleEarthColors)
	ON_COMMAND(IDM_GOOGLEEARTH_CLAMP, OnGoogleEarthClamp)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GOOGLEEARTH_OPEN, IDM_GOOGLEEARTH_CLAMP, OnUpdateGoogleEarthCommands)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMainWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS, 1);

	m_pDialogMenuBar->AddMenuLeft(IDM_FILE);
	m_pDialogMenuBar->AddMenuLeft(IDM_EDIT);
	m_pDialogMenuBar->AddMenuLeft(IDM_MAP);
	m_pDialogMenuBar->AddMenuLeft(IDM_GLOBE);
	m_pDialogMenuBar->AddMenuLeft(IDM_GOOGLEEARTH);
	m_pDialogMenuBar->AddMenuLeft(IDM_STATISTICS);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 0);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 1);
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 2);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 3);

	theApp.AddFrame(this);

	OpenMainView(TRUE);

	return 0;
}

void CMainWnd::OnDestroy()
{
	if (m_pWndMainView)
	{
		m_pWndMainView->DestroyWindow();
		delete m_pWndMainView;
	}

	CMainWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (m_pWndMainView)
		m_pWndMainView->SetFocus();
}

LRESULT CMainWnd::OnRequestSubmenu(WPARAM wParam, LPARAM /*lParam*/)
{
	CDialogMenuPopup* pPopup = new CDialogMenuPopup();

	switch ((UINT)wParam)
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
		pPopup->AddCommand(IDM_EDIT_GOTO, 6, CDMB_SMALL);
		break;
	case IDM_EDIT_PASTE:
		pPopup->Create(this, IDB_MENUEDIT_32, IDB_MENUEDIT_16);
		pPopup->AddCommand(IDM_EDIT_INSERT_FLIGHT, 3, CDMB_LARGE);
		pPopup->AddCommand(IDM_EDIT_INSERT_ROUTE, 3, CDMB_LARGE);
		pPopup->AddFileType(IDM_EDIT_INSERT_ITINERARY, _T(".airx"), CDMB_LARGE);
		break;
	case IDM_MAP:
		pPopup->Create(this, IDB_MENUMAP_32, IDB_MENUMAP_16);
		pPopup->AddSubmenu(IDM_MAP_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_MAP_SELECTEDONLY);
		pPopup->AddCaption(IDS_BACKGROUND);
		pPopup->AddGallery(IDM_MAP_BACKGROUND, IDB_BACKGROUNDS, CSize(96, 48), IDS_BACKGROUND_DEFAULT, 4, 2, theApp.m_MapSettings.BackgroundColor, FALSE);
		pPopup->AddColor(IDM_MAP_BACKGROUNDCOLOR, &theApp.m_MapSettings.BackgroundColor);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_MAP_CENTERATLANTIC, TRUE);
		pPopup->AddCheckbox(IDM_MAP_CENTERPACIFIC, TRUE);
		pPopup->AddCaption(IDS_FLIGHTROUTES);
		pPopup->AddCheckbox(IDM_MAP_SHOWFLIGHTROUTES);
		pPopup->AddCheckbox(IDM_MAP_STRAIGHTLINES);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_MAP_USECOLORS);
		pPopup->AddColor(IDM_MAP_ROUTECOLOR, &theApp.m_MapSettings.RouteColor);
		pPopup->AddCaption(IDS_LOCATIONS);
		pPopup->AddCheckbox(IDM_MAP_SHOWLOCATIONS);
		pPopup->AddColor(IDM_MAP_LOCATIONINNERCOLOR, &theApp.m_MapSettings.LocationInnerColor);
		pPopup->AddColor(IDM_MAP_LOCATIONOUTERCOLOR, &theApp.m_MapSettings.LocationOuterColor);
		pPopup->AddCaption(IDS_IATACODES);
		pPopup->AddCheckbox(IDM_MAP_SHOWIATACODES);
		pPopup->AddColor(IDM_MAP_IATAINNERCOLOR, &theApp.m_MapSettings.IATAInnerColor);
		pPopup->AddColor(IDM_MAP_IATAOUTERCOLOR, &theApp.m_MapSettings.IATAOuterColor);
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
		pPopup->AddCheckbox(IDM_GLOBE_SELECTEDONLY);
		break;
	case IDM_GOOGLEEARTH:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddSubmenu(IDM_GOOGLEEARTH_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_GOOGLEEARTH_SELECTEDONLY);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_GOOGLEEARTH_COLORS);
		pPopup->AddCheckbox(IDM_GOOGLEEARTH_CLAMP);
		break;
	case IDM_GOOGLEEARTH_OPEN:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_GOOGLEEARTH_EXPORT, _T(".kml"), CDMB_LARGE);
		break;
	}

	if (!pPopup->HasItems())
	{
		delete pPopup;
		pPopup = NULL;
	}

	return (LRESULT)pPopup;
}

LRESULT CMainWnd::OnGalleryChanged(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)wParam)
	{
	case IDM_MAP_BACKGROUND:
		theApp.m_MapSettings.Background = (INT)lParam;
		break;
	}

	return NULL;
}

LRESULT CMainWnd::OnUseBgImagesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return m_pWndMainView ? m_pWndMainView->SendMessage(theApp.msgUseBgImagesChanged) : NULL;
}


// File commands

void CMainWnd::OnFileOpen()
{
	CFileDialog dlg(TRUE, _T(".airx"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, _T("Flightmap itinerary (*.airx; *.air)|*.airx; *.air||"), this);
	dlg.DoModal();
}

void CMainWnd::OnFileQuit()
{
	theApp.Quit();
}

void CMainWnd::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_SAVE:
	case IDM_FILE_SAVEAS:
	case IDM_FILE_CLOSE:
		pCmdUI->Enable(TRUE);
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}


// Map commands

void CMainWnd::OnMapOpen()
{
	CMapWnd* pFrame = new CMapWnd();

	pFrame->Create();
//	pFrame->SetFlights(GetKitchen());
	pFrame->ShowWindow(SW_SHOW);
}

void CMainWnd::OnMapCenterAtlantic()
{
	theApp.m_MapSettings.CenterPacific = FALSE;
}

void CMainWnd::OnMapCenterPacific()
{
	theApp.m_MapSettings.CenterPacific = TRUE;
}

void CMainWnd::OnMapShowFlightRoutes()
{
	theApp.m_MapSettings.ShowFlightRoutes = !theApp.m_MapSettings.ShowFlightRoutes;
	if (theApp.m_MapSettings.ShowFlightRoutes)
		theApp.m_MapSettings.ShowLocations = TRUE;
}

void CMainWnd::OnMapStraightLines()
{
	theApp.m_MapSettings.StraightLines = !theApp.m_MapSettings.StraightLines;
}

void CMainWnd::OnMapUseColors()
{
	theApp.m_MapSettings.UseColors = !theApp.m_MapSettings.UseColors;
}

void CMainWnd::OnMapShowLocations()
{
	theApp.m_MapSettings.ShowLocations = !theApp.m_MapSettings.ShowLocations;
}

void CMainWnd::OnMapShowIATACodes()
{
	theApp.m_MapSettings.ShowIATACodes = !theApp.m_MapSettings.ShowIATACodes;
}

void CMainWnd::OnUpdateMapCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_MAP_SELECTEDONLY:
		b = FALSE;
		break;
	case IDM_MAP_BACKGROUND:
		pCmdUI->SetCheck(theApp.m_MapSettings.Background);
		break;
	case IDM_MAP_BACKGROUNDCOLOR:
		b = theApp.m_MapSettings.Background==3;
		break;
	case IDM_MAP_CENTERATLANTIC:
		pCmdUI->SetCheck(!theApp.m_MapSettings.CenterPacific);
		break;
	case IDM_MAP_CENTERPACIFIC:
		pCmdUI->SetCheck(theApp.m_MapSettings.CenterPacific);
		break;
	case IDM_MAP_SHOWFLIGHTROUTES:
		pCmdUI->SetCheck(theApp.m_MapSettings.ShowFlightRoutes);
		break;
	case IDM_MAP_STRAIGHTLINES:
		pCmdUI->SetCheck(theApp.m_MapSettings.StraightLines);
		b = theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_USECOLORS:
		pCmdUI->SetCheck(theApp.m_MapSettings.UseColors);
	case IDM_MAP_ROUTECOLOR:
		b = theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_SHOWLOCATIONS:
		pCmdUI->SetCheck(theApp.m_MapSettings.ShowLocations);
		b = !theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_LOCATIONINNERCOLOR:
	case IDM_MAP_LOCATIONOUTERCOLOR:
		b = theApp.m_MapSettings.ShowLocations;
		break;
	case IDM_MAP_SHOWIATACODES:
		pCmdUI->SetCheck(theApp.m_MapSettings.ShowIATACodes);
		break;
	case IDM_MAP_IATAINNERCOLOR:
	case IDM_MAP_IATAOUTERCOLOR:
		b = theApp.m_MapSettings.ShowIATACodes;
		break;
	}

	pCmdUI->Enable(b);
}


// Globe commands

void CMainWnd::OnGlobeOpen()
{
	CGlobeWnd* pFrame = new CGlobeWnd();

	pFrame->Create();
	pFrame->SetFlights(GetKitchen());
	pFrame->ShowWindow(SW_SHOW);
}

void CMainWnd::OnUpdateGlobeCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_OPEN:
		pCmdUI->Enable(TRUE);
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}


// Google Earth commands

void CMainWnd::OnGoogleEarthOpen()
{
	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sFlightmap%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	if (ExportKML(szTempName, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClamp))
		ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOW);
}

void CMainWnd::OnGoogleEarthExport()
{
	CString Extensions;
	ENSURE(Extensions.LoadString(IDS_FILEFILTER_KML));
	Extensions += _T(" (*.kml)|*.kml||");

	CFileDialog dlg(FALSE, _T(".kml"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportKML(dlg.GetPathName(), theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClamp);
}

void CMainWnd::OnGoogleEarthColors()
{
	theApp.m_GoogleEarthUseColors = !theApp.m_GoogleEarthUseColors;
}

void CMainWnd::OnGoogleEarthClamp()
{
	theApp.m_GoogleEarthClamp = !theApp.m_GoogleEarthClamp;
}

void CMainWnd::OnUpdateGoogleEarthCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_GOOGLEEARTH_OPEN:
		b = !theApp.m_PathGoogleEarth.IsEmpty();
		break;
	case IDM_GOOGLEEARTH_SELECTEDONLY:
		b = FALSE;
		break;
	case IDM_GOOGLEEARTH_COLORS:
		pCmdUI->SetCheck(theApp.m_GoogleEarthUseColors);
		break;
	case IDM_GOOGLEEARTH_CLAMP:
		pCmdUI->SetCheck(theApp.m_GoogleEarthClamp);
		break;
	}

	pCmdUI->Enable(b);
}
