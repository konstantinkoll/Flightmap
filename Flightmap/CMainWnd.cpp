
// CMainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "CCalendarFile.h"
#include "CDataGrid.h"
#include "CExcelFile.h"
#include "CGlobeWnd.h"
#include "CGoogleEarthFile.h"
#include "CKitchen.h"
#include "CLoungeView.h"
#include "CMainWnd.h"
#include "CMapFactory.h"
#include "CMapWnd.h"
#include "Flightmap.h"
#include "PropertiesDlg.h"


// CMainWnd
//

CMainWnd::CMainWnd()
	: CMainWindow()
{
	m_hIcon = NULL;
	m_pWndMainView = NULL;
	m_pItinerary = NULL;
	m_CurrentMainView = 0;
}

CMainWnd::~CMainWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	if (m_pItinerary)
		delete m_pItinerary;
}

BOOL CMainWnd::Create(CItinerary* pItinerary)
{
	m_pItinerary = pItinerary;

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

void CMainWnd::UpdateWindowStatus(BOOL AllowLoungeView)
{
	BOOL Change = (m_pWndMainView==NULL) ||
		((m_CurrentMainView==LoungeView) && (m_pItinerary!=NULL)) ||
		((m_CurrentMainView==DataGrid) && (m_pItinerary==NULL) && AllowLoungeView);

	if (Change)
	{
		if (m_pWndMainView)
		{
			m_pWndMainView->DestroyWindow();
			delete m_pWndMainView;
		}

		if (!m_pItinerary)
		{
			m_pWndMainView = new CLoungeView();
			((CLoungeView*)m_pWndMainView)->Create(this, 2);

			m_CurrentMainView = LoungeView;
		}
		else
		{
			m_pWndMainView = new CDataGrid();
			((CDataGrid*)m_pWndMainView)->Create(this, 2);

			m_CurrentMainView = DataGrid;
		}
	}

	if (m_CurrentMainView==DataGrid)
		((CDataGrid*)m_pWndMainView)->SetItinerary(m_pItinerary);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));
	if (m_pItinerary)
	{
		if (!m_pItinerary->m_DisplayName.IsEmpty())
		{
			caption.Insert(0, _T(" - "));
			caption.Insert(0, m_pItinerary->m_DisplayName);
		}
	}

	SetWindowText(caption);
	AdjustLayout();
	SetFocus();
}

CItinerary* CMainWnd::Load(CString FileName)
{
	CItinerary* pItinerary = new CItinerary();

	CString Ext = FileName;
	Ext.MakeLower();
	INT pos = Ext.ReverseFind(L'\\');
	if (pos!=-1)
		Ext.Delete(0, pos+1);
	pos = Ext.ReverseFind(L'.');
	if (pos!=-1)
		Ext.Delete(0, pos+1);

	if (Ext==_T("airx"))
		pItinerary->OpenAIRX(FileName);
	if (Ext==_T("air"))
		pItinerary->OpenAIR(FileName);
	if (Ext==_T("csv"))
		pItinerary->OpenCSV(FileName);

	return pItinerary;
}

void CMainWnd::Open(CString FileName)
{
	m_pItinerary = Load(FileName);

	UpdateWindowStatus();
}

BOOL CMainWnd::CloseFile(BOOL AllowLoungeView)
{
	if (m_pItinerary)
	{
		if (m_pItinerary->m_IsModified)
		{
			CString msg;
			ENSURE(msg.LoadString(IDS_NOTSAVED));

			switch (MessageBox(msg, m_pItinerary->m_DisplayName, MB_YESNOCANCEL | MB_ICONWARNING))
			{
			case IDCANCEL:
				return FALSE;
			case IDYES:
				OnFileSave();
				if (m_pItinerary->m_IsModified)
					return FALSE;
			}
		}

		delete m_pItinerary;
		m_pItinerary = NULL;

		UpdateWindowStatus(AllowLoungeView);
	}

	return TRUE;
}

CKitchen* CMainWnd::GetKitchen(BOOL Selected, BOOL MergeMetro)
{
	CKitchen* pKitchen = new CKitchen(m_pItinerary ? m_pItinerary->m_DisplayName : _T(""), MergeMetro);

	if (m_pItinerary)
		for (UINT a=0; a<m_pItinerary->m_Flights.m_ItemCount; a++)
			pKitchen->AddFlight(m_pItinerary->m_Flights.m_Items[a]);

	return pKitchen;
}

CBitmap* CMainWnd::GetMap(BOOL Selected, BOOL MergeMetro)
{
	CMapFactory f(&theApp.m_MapSettings);
	return f.RenderMap(GetKitchen(Selected, MergeMetro));
}

void CMainWnd::ExportMap(CString Filename, GUID guidFileType, BOOL Selected, BOOL MergeMetro)
{
	CWaitCursor csr;

	theApp.SaveBitmap(GetMap(Selected, MergeMetro), Filename, guidFileType);
}

void CMainWnd::ExportExcel(CString FileName)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CExcelFile f;

	if (!f.Open(FileName))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		for (UINT a=0; a<Limit; a++)
			f.WriteRoute(m_pItinerary->m_Flights.m_Items[a]);

		f.Close();
	}
}

void CMainWnd::ExportCalendar(CString FileName)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CCalendarFile f;

	if (!f.Open(FileName, m_pItinerary->m_Metadata.Comments, m_pItinerary->m_Metadata.Title))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		for (UINT a=0; a<Limit; a++)
			f.WriteRoute(m_pItinerary->m_Flights.m_Items[a]);

		f.Close();
	}
}

BOOL CMainWnd::ExportGoogleEarth(CString FileName, BOOL UseColors, BOOL Clamp, BOOL Selected, BOOL MergeMetro)
{
	CGoogleEarthFile f;

	if (!f.Open(FileName, m_pItinerary ? m_pItinerary->m_DisplayName : NULL))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
		return FALSE;
	}
	else
	{
		BOOL Res = FALSE;
		CKitchen* pKitchen = GetKitchen(Selected, MergeMetro);

		try
		{
			f.WriteRoutes(pKitchen, UseColors, Clamp, FALSE);

			CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc();
			while (pPair)
			{
				f.WriteAirport(pPair->value.pAirport);

				pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair);
			}

			f.Close();
			Res = TRUE;
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}

		delete pKitchen;
		return Res;
	}
}

void CMainWnd::ExportText(CString FileName)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CStdioFile f;

	if (!f.Open(FileName, CFile::modeCreate | CFile::modeWrite))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		try
		{
			for (UINT a=0; a<Limit; a++)
				f.WriteString(m_pItinerary->Flight2Text(a));

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}
	}
}


BEGIN_MESSAGE_MAP(CMainWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
	ON_MESSAGE(WM_GALLERYCHANGED, OnGalleryChanged)
	ON_REGISTERED_MESSAGE(theApp.msgUseBgImagesChanged, OnUseBgImagesChanged)
	ON_REGISTERED_MESSAGE(theApp.msgDistanceSettingChanged, OnDistanceSettingChanged)

	ON_COMMAND(IDM_FILE_NEW, OnFileNew)
	ON_COMMAND(IDM_FILE_NEWSAMPLE1, OnFileNewSample1)
	ON_COMMAND(IDM_FILE_NEWSAMPLE2, OnFileNewSample2)
	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND_RANGE(IDM_FILE_RECENT, IDM_FILE_RECENT+9, OnFileOpenRecent)
	ON_COMMAND(IDM_FILE_SAVE, OnFileSave)
	ON_COMMAND(IDM_FILE_SAVEAS, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_SAVEAS_CSV, OnFileSaveCSV)
	ON_COMMAND(IDM_FILE_SAVEAS_ICS, OnFileSaveICS)
	ON_COMMAND(IDM_FILE_SAVEAS_TXT, OnFileSaveTXT)
	ON_COMMAND(IDM_FILE_SAVEAS_OTHER, OnFileSaveOther)
	ON_COMMAND(IDM_FILE_PREPARE_PROPERTIES, OnFileProperties)
	ON_COMMAND(IDM_FILE_CLOSE, OnFileClose)
	ON_COMMAND(IDM_FILE_QUIT, OnFileQuit)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_NEW, IDM_FILE_QUIT, OnUpdateFileCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_RECENT, IDM_FILE_RECENT+9, OnUpdateFileCommands)

	ON_COMMAND(IDM_MAP_OPEN, OnMapOpen)
	ON_COMMAND(IDM_MAP_MERGEMETRO, OnMapMergeMetro)
	ON_COMMAND(IDM_MAP_CENTERATLANTIC, OnMapCenterAtlantic)
	ON_COMMAND(IDM_MAP_CENTERPACIFIC, OnMapCenterPacific)
	ON_COMMAND(IDM_MAP_SHOWFLIGHTROUTES, OnMapShowFlightRoutes)
	ON_COMMAND(IDM_MAP_STRAIGHTLINES, OnMapStraightLines)
	ON_COMMAND(IDM_MAP_ARROWS, OnMapArrows)
	ON_COMMAND(IDM_MAP_USECOUNT, OnMapUseCount)
	ON_COMMAND(IDM_MAP_USECOLORS, OnMapUseColors)
	ON_COMMAND(IDM_MAP_SHOWLOCATIONS, OnMapShowLocations)
	ON_COMMAND(IDM_MAP_SHOWIATACODES, OnMapShowIATACodes)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAP_OPEN, IDM_MAP_IATAOUTERCOLOR, OnUpdateMapCommands)

	ON_COMMAND(IDM_MAP_EXPORT_BMP, OnMapExportBMP)
	ON_COMMAND(IDM_MAP_EXPORT_JPEG, OnMapExportJPEG)
	ON_COMMAND(IDM_MAP_EXPORT_PNG, OnMapExportPNG)
	ON_COMMAND(IDM_MAP_EXPORT_TIFF, OnMapExportTIFF)

	ON_COMMAND(IDM_GLOBE_OPEN, OnGlobeOpen)
	ON_COMMAND(IDM_GLOBE_MERGEMETRO, OnGlobeMergeMetro)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBE_OPEN, IDM_GLOBE_MERGEMETRO, OnUpdateGlobeCommands)

	ON_COMMAND(IDM_GOOGLEEARTH_OPEN, OnGoogleEarthOpen)
	ON_COMMAND(IDM_GOOGLEEARTH_EXPORT, OnGoogleEarthExport)
	ON_COMMAND(IDM_GOOGLEEARTH_MERGEMETRO, OnGoogleEarthMergeMetro)
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

	UpdateWindowStatus(TRUE);

	return 0;
}

void CMainWnd::OnClose()
{
	if (m_pItinerary)
		if (!CloseFile())
			return;

	CMainWindow::OnClose();
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
		theApp.AddRecentList(pPopup);
		pPopup->AddCaption(IDS_SAMPLEITINERARIES);
		pPopup->AddCommand(IDM_FILE_NEWSAMPLE1, 1, CDMB_LARGE);
		pPopup->AddCommand(IDM_FILE_NEWSAMPLE2, 1, CDMB_LARGE);
		break;
	case IDM_FILE_SAVEAS:
		pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);
		pPopup->AddCaption(IDS_SAVECOPY);
		pPopup->AddFileType(IDM_FILE_SAVEAS_AIRX, _T("airx"), CDMB_LARGE);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_FILE_SAVEAS_CSV, _T("csv"), CDMB_LARGE);
		pPopup->AddFileType(IDM_FILE_SAVEAS_ICS, _T("ics"), CDMB_LARGE);
		pPopup->AddFileType(IDM_FILE_SAVEAS_TXT, _T("txt"), CDMB_LARGE);
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
		break;
	case IDM_EDIT_PASTE:
		pPopup->Create(this, IDB_MENUEDIT_32, IDB_MENUEDIT_16);
		pPopup->AddCommand(IDM_EDIT_INSERT_FLIGHT, 3, CDMB_LARGE);
		pPopup->AddCommand(IDM_EDIT_INSERT_ROUTE, 3, CDMB_LARGE);
		pPopup->AddFileType(IDM_EDIT_INSERT_ITINERARY, _T("airx"), CDMB_LARGE);
		break;
	case IDM_MAP:
		pPopup->Create(this, IDB_MENUMAP_32, IDB_MENUMAP_16);
		pPopup->AddSubmenu(IDM_MAP_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_MAP_MERGEMETRO);
		pPopup->AddCaption(IDS_BACKGROUND);
		pPopup->AddGallery(IDM_MAP_BACKGROUND, IDB_BACKGROUNDS, CSize(96, 48), IDS_BACKGROUND_DEFAULT, 4, 2, theApp.m_MapSettings.BackgroundColor, FALSE);
		pPopup->AddColor(IDM_MAP_BACKGROUNDCOLOR, &theApp.m_MapSettings.BackgroundColor);
		pPopup->AddResolution(IDM_MAP_RESOLUTION, 1, &theApp.m_MapSettings.Width, &theApp.m_MapSettings.Height);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_MAP_CENTERATLANTIC, TRUE);
		pPopup->AddCheckbox(IDM_MAP_CENTERPACIFIC, TRUE);
		pPopup->AddCaption(IDS_FLIGHTROUTES);
		pPopup->AddCheckbox(IDM_MAP_SHOWFLIGHTROUTES);
		pPopup->AddCheckbox(IDM_MAP_STRAIGHTLINES);
		pPopup->AddCheckbox(IDM_MAP_ARROWS);
		pPopup->AddCheckbox(IDM_MAP_USECOUNT);
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
		pPopup->AddFileType(IDM_MAP_EXPORT_BMP, _T("bmp"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_JPEG, _T("jpg"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_PNG, _T("png"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_TIFF, _T("tif"), CDMB_LARGE);
		break;
	case IDM_GLOBE:
		pPopup->Create(this, IDB_MENUGLOBE_32, IDB_MENUGLOBE_16);
		pPopup->AddCommand(IDM_GLOBE_OPEN, 0, CDMB_LARGE);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_GLOBE_MERGEMETRO);
		break;
	case IDM_GOOGLEEARTH:
		pPopup->Create(this, IDB_MENUGOOGLEEARTH_32, IDB_MENUGOOGLEEARTH_16);
		pPopup->AddSubmenu(IDM_GOOGLEEARTH_OPEN, 0, CDMB_LARGE, TRUE);
		pPopup->AddSeparator(TRUE);
		pPopup->AddCheckbox(IDM_GOOGLEEARTH_MERGEMETRO);
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

LRESULT CMainWnd::OnUseBgImagesChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	theApp.m_UseBgImages = (BOOL)wParam;

	return m_pWndMainView ? m_pWndMainView->SendMessage(theApp.msgUseBgImagesChanged) : NULL;
}

LRESULT CMainWnd::OnDistanceSettingChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	theApp.m_UseStatuteMiles = (BOOL)wParam;

	if (m_pWndMainView)
		m_pWndMainView->Invalidate();

	return NULL;
}


// File commands

void CMainWnd::OnFileNew()
{
	if (CloseFile())
	{
		m_pItinerary = new CItinerary(TRUE);
		UpdateWindowStatus();
	}
}

void CMainWnd::OnFileNewSample1()
{
	if (CloseFile())
	{
		m_pItinerary = new CItinerary();
		m_pItinerary->NewSampleAtlantic();
		UpdateWindowStatus();
	}
}

void CMainWnd::OnFileNewSample2()
{
	if (CloseFile())
	{
		m_pItinerary = new CItinerary();
		m_pItinerary->NewSamplePacific();
		UpdateWindowStatus();
	}
}

void CMainWnd::OnFileOpen()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_AIRX, _T("airx; *.air"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_CSV, _T("csv"), TRUE);

	CFileDialog dlg(TRUE, _T("airx"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		if (!CloseFile())
			return;

		Open(dlg.GetPathName());
	}
}

void CMainWnd::OnFileOpenRecent(UINT CmdID)
{
	ASSERT(CmdID>=IDM_FILE_RECENT);

	if (!CloseFile())
		return;

	CmdID -= IDM_FILE_RECENT;

	UINT a=0;
	for (POSITION p=theApp.m_RecentFiles.GetHeadPosition(); p; a++)
	{
		CString FileName = theApp.m_RecentFiles.GetNext(p);
		if (CmdID==a)
		{
			Open(FileName);
			break;
		}
	}
}

void CMainWnd::OnFileSave()
{
	ASSERT(m_pItinerary);

	if (m_pItinerary->m_FileName.IsEmpty())
	{
		OnFileSaveAs();
	}
	else
	{
		m_pItinerary->SaveAIRX(m_pItinerary->m_FileName);

		UpdateWindowStatus();
	}
}

void CMainWnd::OnFileSaveAs()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_AIRX, _T("airx"), TRUE);

	CFileDialog dlg(FALSE, _T("airx"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		m_pItinerary->SaveAIRX(dlg.GetPathName());

		UpdateWindowStatus();
	}
}

void CMainWnd::OnFileSaveCSV()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_CSV, _T("csv"), TRUE);

	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportExcel(dlg.GetPathName());
}

void CMainWnd::OnFileSaveICS()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_ICS, _T("ics"), TRUE);

	CFileDialog dlg(FALSE, _T("ics"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportCalendar(dlg.GetPathName());
}

void CMainWnd::OnFileSaveTXT()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TXT, _T("txt"), TRUE);

	CFileDialog dlg(FALSE, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportText(dlg.GetPathName());
}

void CMainWnd::OnFileSaveOther()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_AIRX, _T("airx"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_CSV, _T("csv"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_ICS, _T("ics"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_KML, _T(".kml"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TXT, _T("txt"), TRUE);

	CFileDialog dlg(FALSE, _T("airx"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		CString Ext = dlg.GetFileExt().MakeLower();
		if (Ext==_T("airx"))
		{
			m_pItinerary->SaveAIRX(m_pItinerary->m_FileName);
			UpdateWindowStatus();
		}
		if (Ext==_T("bmp"))
			ExportMap(dlg.GetPathName(), ImageFormatBMP);
		if (Ext==_T("csv"))
			ExportExcel(dlg.GetPathName());
		if (Ext==_T("ics"))
			ExportCalendar(dlg.GetPathName());
		if (Ext==_T("kml"))
			ExportGoogleEarth(dlg.GetPathName(), theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClamp);
		if (Ext==_T("jpg"))
			ExportMap(dlg.GetPathName(), ImageFormatJPEG);
		if (Ext==_T("png"))
			ExportMap(dlg.GetPathName(), ImageFormatPNG);
		if (Ext==_T("tif"))
			ExportMap(dlg.GetPathName(), ImageFormatTIFF);
		if (Ext==_T("txt"))
			ExportText(dlg.GetPathName());
	}
}

void CMainWnd::OnFileProperties()
{
	ASSERT(m_pItinerary);

	PropertiesDlg dlg(m_pItinerary, this);
	dlg.DoModal();
}

void CMainWnd::OnFileClose()
{
	CloseFile(TRUE);
}

void CMainWnd::OnFileQuit()
{
	theApp.Quit();
}

void CMainWnd::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_PRINT:
	case IDM_FILE_PRINT_PREVIEW:
	case IDM_FILE_PRINT_QUICK:
	case IDM_FILE_PREPARE_INSPECT:
	case IDM_FILE_PREPARE_ATTACHMENTS:
		pCmdUI->Enable(FALSE);	// TODO
		break;
	case IDM_FILE_SAVE:
	case IDM_FILE_SAVEAS:
	case IDM_FILE_SAVEAS_AIRX:
	case IDM_FILE_SAVEAS_CSV:
	case IDM_FILE_SAVEAS_ICS:
	case IDM_FILE_SAVEAS_TXT:
	case IDM_FILE_SAVEAS_OTHER:
	case IDM_FILE_PREPARE:
	case IDM_FILE_PREPARE_PROPERTIES:
	case IDM_FILE_CLOSE:
		pCmdUI->Enable(m_pItinerary!=NULL);
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}


// Map commands

void CMainWnd::OnMapOpen()
{
	theApp.ShowNagScreen(NAG_FORCE, this);

	CWaitCursor csr;

	CBitmap* pBitmap = GetMap(TRUE, theApp.m_MapMergeMetro);

	CMapWnd* pFrame = new CMapWnd();
	pFrame->Create();
	pFrame->SetBitmap(pBitmap, m_pItinerary->m_DisplayName);
	pFrame->ShowWindow(SW_SHOW);
}

void CMainWnd::OnMapMergeMetro()
{
	theApp.m_MapMergeMetro = !theApp.m_MapMergeMetro;
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
}

void CMainWnd::OnMapStraightLines()
{
	theApp.m_MapSettings.StraightLines = !theApp.m_MapSettings.StraightLines;
}

void CMainWnd::OnMapArrows()
{
	theApp.m_MapSettings.Arrows = !theApp.m_MapSettings.Arrows;
}

void CMainWnd::OnMapUseCount()
{
	theApp.m_MapSettings.UseCount = !theApp.m_MapSettings.UseCount;
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
	case IDM_MAP_OPEN:
	case IDM_MAP_EXPORT_BMP:
	case IDM_MAP_EXPORT_JPEG:
	case IDM_MAP_EXPORT_PNG:
	case IDM_MAP_EXPORT_TIFF:
		b = (m_pItinerary!=NULL);
		break;
	case IDM_MAP_MERGEMETRO:
		pCmdUI->SetCheck(theApp.m_MapMergeMetro);
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
	case IDM_MAP_ARROWS:
		pCmdUI->SetCheck(theApp.m_MapSettings.Arrows);
		b = theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_USECOUNT:
		pCmdUI->SetCheck(theApp.m_MapSettings.UseCount);
		b = theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_USECOLORS:
		pCmdUI->SetCheck(theApp.m_MapSettings.UseColors);
	case IDM_MAP_ROUTECOLOR:
		b = theApp.m_MapSettings.ShowFlightRoutes;
		break;
	case IDM_MAP_SHOWLOCATIONS:
		pCmdUI->SetCheck(theApp.m_MapSettings.ShowLocations);
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


// Map export commands

void CMainWnd::OnMapExportBMP()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"), TRUE);

	CFileDialog dlg(FALSE, _T("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatBMP, TRUE, theApp.m_MapMergeMetro);
}

void CMainWnd::OnMapExportJPEG()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"), TRUE);

	CFileDialog dlg(FALSE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatJPEG, TRUE, theApp.m_MapMergeMetro);
}

void CMainWnd::OnMapExportPNG()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatPNG, TRUE, theApp.m_MapMergeMetro);
}

void CMainWnd::OnMapExportTIFF()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("tif"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatTIFF, TRUE, theApp.m_MapMergeMetro);
}


// Globe commands

void CMainWnd::OnGlobeOpen()
{
	CGlobeWnd* pFrame = new CGlobeWnd();

	pFrame->Create();
	pFrame->SetFlights(GetKitchen(TRUE, theApp.m_GlobeMergeMetro));
	pFrame->ShowWindow(SW_SHOW);
}

void CMainWnd::OnGlobeMergeMetro()
{
	theApp.m_GlobeMergeMetro = !theApp.m_GlobeMergeMetro;
}

void CMainWnd::OnUpdateGlobeCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_OPEN:
		b = (m_pItinerary!=NULL);
		break;
	case IDM_GLOBE_MERGEMETRO:
		pCmdUI->SetCheck(theApp.m_GlobeMergeMetro);
		break;
	}

	pCmdUI->Enable(b);
}


// Google Earth commands

void CMainWnd::OnGoogleEarthOpen()
{
	ASSERT(m_pItinerary);

	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	theApp.ShowNagScreen(NAG_FORCE, this);

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sFlightmap%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	if (ExportGoogleEarth(szTempName, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClamp, theApp.m_GoogleEarthMergeMetro))
		ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOW);
}

void CMainWnd::OnGoogleEarthMergeMetro()
{
	theApp.m_GoogleEarthMergeMetro = !theApp.m_GoogleEarthMergeMetro;
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
		b = (m_pItinerary!=NULL) && !theApp.m_PathGoogleEarth.IsEmpty();
		break;
	case IDM_GOOGLEEARTH_EXPORT:
		b = m_pItinerary!=NULL;
		break;
	case IDM_GOOGLEEARTH_MERGEMETRO:
		pCmdUI->SetCheck(theApp.m_GoogleEarthMergeMetro);
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


// Google Earth export command

void CMainWnd::OnGoogleEarthExport()
{
	ASSERT(m_pItinerary);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_KML, _T(".kml"), TRUE);

	CFileDialog dlg(FALSE, _T(".kml"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportGoogleEarth(dlg.GetPathName(), theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClamp, theApp.m_GoogleEarthMergeMetro);
}
