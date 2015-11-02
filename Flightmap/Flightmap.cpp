
// Flightmap.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "CGoogleEarthFile.h"
#include "CMainWnd.h"
#include "Flightmap.h"


GUID theAppID =	// {8269ADBF-A534-469d-A58D-7EBA84634B70}
	{ 0x8269ADBF, 0xA534, 0x469D, { 0xA5, 0x8D, 0x7E, 0xBA, 0x84, 0x63, 0x4B, 0x70 } };

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetWindow(hWnd, GW_OWNER))
		return TRUE;

	DWORD_PTR Result;
	if (SendMessageTimeout(hWnd, theApp.m_WakeupMsg, NULL, NULL, SMTO_NORMAL, 500, &Result))
		if (Result==24878)
		{
			CDS_Wakeup cdsw;
			ZeroMemory(&cdsw, sizeof(cdsw));
			cdsw.AppID = theAppID;
			if (lParam)
				wcscpy_s(cdsw.Command, MAX_PATH, (WCHAR*)lParam);

			COPYDATASTRUCT cds;
			cds.cbData = sizeof(cdsw);
			cds.lpData = &cdsw;
			if (SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&cds))
				return FALSE;
		}

	return TRUE;
}


// CFlightmapApp

BEGIN_MESSAGE_MAP(CFlightmapApp, FMApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CFlightmapApp-Erstellung

CFlightmapApp::CFlightmapApp()
	: FMApplication(theAppID)
{
	m_AppInitialized = FALSE;
	hFlagIcons[0] = hFlagIcons[1] = NULL;

	CF_FLIGHTS = (CLIPFORMAT)RegisterClipboardFormat(_T("liquidFOLDERS.Flightmap"));
}


// Das einzige CFlightmapApp-Objekt

CFlightmapApp theApp;


// CFlightmapApp-Initialisierung

BOOL CFlightmapApp::InitInstance()
{
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)(__argc==2 ? __wargv[1] : NULL)))
		return FALSE;

	if (!FMApplication::InitInstance())
		return FALSE;

	// AppID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"liquidFOLDERS.Flightmap");

	// RestartManager
	if (m_KernelLibLoaded)
		zRegisterApplicationRestart(L"", 11);	// RESTART_NO_CRASH | RESTART_NO_HANG | RESTART_NO_REBOOT

	// Pfad zu Google Earth
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Google\\Google Earth Plus"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		WCHAR lszValue[256];
		DWORD dwSize = sizeof(lszValue);

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			m_PathGoogleEarth = lszValue;

		RegCloseKey(hKey);
	}

	// Pfad zu liquidFOLDERS
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\liquidFOLDERS"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		WCHAR lszValue[256];
		DWORD dwSize = sizeof(lszValue);

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			m_PathLiquidFolders = lszValue;

		RegCloseKey(hKey);
	}

	// Icons
	hFlagIcons[0] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_FLAGS_16i));
	hFlagIcons[1] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_FLAGS_16));

	// Registry auslesen
	SetRegistryBase();
	m_UseStatuteMiles = GetInt(_T("UseStatuteMiles"), FALSE);
	m_UseBgImages = GetInt(_T("UseBgImages"), OSVersion!=OS_Eight);
	m_nTextureSize = GetInt(_T("TextureSize"), FMTextureAuto);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), FMTexture4096);
	m_MapMergeMetro = GetInt(_T("MapMergeMetro"), FALSE);
	m_MapZoomFactor = GetInt(_T("MapZoomFactor"), 6);
	m_MapAutosize= GetInt(_T("MapAutosize"), TRUE);
	m_GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_GlobeAntialising = GetInt(_T("GlobeAntialising"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeMergeMetro = GetInt(_T("GlobeMergeMetro"), FALSE);
	m_GlobeUseColors = GetInt(_T("GlobeUseColors"), TRUE);
	m_GlobeClamp = GetInt(_T("GlobeClamp"), FALSE);
	m_GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_GlobeShowAirportIATA = GetInt(_T("GlobeShowAirportIATA"), TRUE);
	m_GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobeShowGPS = GetInt(_T("GlobeShowGPS"), FALSE);
	m_GlobeShowFlightCount = GetInt(_T("GlobeShowFlightCount"), FALSE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);
	m_GoogleEarthMergeMetro = GetInt(_T("GoogleEarthMergeMetro"), FALSE);
	m_GoogleEarthUseCount = GetInt(_T("GoogleEarthUseCount"), FALSE);
	m_GoogleEarthUseColors = GetInt(_T("GoogleEarthUseColors"), TRUE);
	m_GoogleEarthClamp = GetInt(_T("GoogleEarthClamp"), FALSE);
	m_MergeDirections = GetInt(_T("MergeDirections"), TRUE);
	m_MergeAwards = GetInt(_T("MergeAwards"), FALSE);
	m_MergeClasses = GetInt(_T("MergeClasses"), TRUE);
	GetBinary(_T("CustomColors"), &m_CustomColors, sizeof(m_CustomColors));
	m_MapSettings.Background = GetInt(_T("MapBackground"), 0);
	m_MapSettings.BackgroundColor = GetInt(_T("MapBackgroundColor"), 0xF0F0F0);
	m_MapSettings.Width = GetInt(_T("MapWidth"), 640);
	m_MapSettings.Height = GetInt(_T("MapHeight"), 640);
	m_MapSettings.CenterPacific = GetInt(_T("MapCenterPacific"), FALSE);
	m_MapSettings.WideBorder = GetInt(_T("MapWideBorder"), FALSE);
	m_MapSettings.ShowFlightRoutes = GetInt(_T("MapShowFlightRoutes"), TRUE);
	m_MapSettings.StraightLines = GetInt(_T("MapStraightLines"), FALSE);
	m_MapSettings.Arrows = GetInt(_T("MapArrows"), FALSE);
	m_MapSettings.UseCountWidth = GetInt(_T("MapUseCountWidth"), FALSE);
	m_MapSettings.UseCountOpacity = GetInt(_T("MapUseCountOpacity"), FALSE);
	m_MapSettings.NoteDistance = GetInt(_T("MapNoteDistance"), FALSE);
	m_MapSettings.NoteFlightTime = GetInt(_T("MapNoteFlightTime"), FALSE);
	m_MapSettings.NoteFlightCount = GetInt(_T("MapNoteFlightCount"), FALSE);
	m_MapSettings.NoteCarrier = GetInt(_T("MapNoteCarrier"), FALSE);
	m_MapSettings.NoteEquipment = GetInt(_T("MapNoteEquipment"), FALSE);
	m_MapSettings.NoteSmallFont = GetInt(_T("MapNoteSmallFont"), TRUE);
	m_MapSettings.NoteInnerColor = GetInt(_T("MapNoteInnerColor"), 0xFFFFFF);
	m_MapSettings.NoteOuterColor = GetInt(_T("MapNoteOuterColor"), 0x000000);
	m_MapSettings.UseColors = GetInt(_T("MapUseColors"), TRUE);
	m_MapSettings.RouteColor = GetInt(_T("MapRouteColor"), 0xFFFFFF);
	m_MapSettings.ShowLocations = GetInt(_T("MapShowLocations"), TRUE);
	m_MapSettings.LocationInnerColor = GetInt(_T("MapLocationInnerColor"), 0x0000FF);
	m_MapSettings.LocationOuterColor = GetInt(_T("MapLocationOuterColor"), 0xFFFFFF);
	m_MapSettings.ShowIATACodes = GetInt(_T("MapShowIATACodes"), TRUE);
	m_MapSettings.IATAInnerColor = GetInt(_T("MapIATAInnerColor"), 0xFFFFFF);
	m_MapSettings.IATAOuterColor = GetInt(_T("MapIATAOuterColor"), 0x000000);

	m_FindReplaceSettings.Flags = GetInt(_T("FindReplaceFlags"), 0);
	m_FindReplaceSettings.SearchTerm[0] = m_FindReplaceSettings.ReplaceTerm[0] = L'\0';

	for (UINT a=0; a<FMAttributeCount; a++)
	{
		m_ViewParameters.ColumnOrder[a] = a;
		m_ViewParameters.ColumnWidth[a] = FMAttributes[a].DefaultVisible ? FMAttributes[a].RecommendedWidth : 0;
	}
	GetBinary(_T("ColumnOrder"), &m_ViewParameters.ColumnOrder, sizeof(m_ViewParameters.ColumnOrder));
	GetBinary(_T("ColumnWidth"), &m_ViewParameters.ColumnWidth, sizeof(m_ViewParameters.ColumnWidth));
	for (UINT a=0; a<FMAttributeCount; a++)
		if (((m_ViewParameters.ColumnWidth[a]!=0) && ((FMAttributes[a].Type==FMTypeRating) || (FMAttributes[a].Type==FMTypeColor)) || (FMAttributes[a].Type==FMTypeFlags)) || ((m_ViewParameters.ColumnWidth[a]==0) && ((a==0) || (a==3))))
			m_ViewParameters.ColumnWidth[a] = FMAttributes[a].RecommendedWidth;

	for (UINT a=0; a<10; a++)
	{
		CString tmpName;
		CString tmpValue;

		tmpName.Format(_T("RecentFile%u"), a);
		tmpValue = GetString(tmpName);
		if (!tmpValue.IsEmpty())
			m_RecentFiles.AddTail(tmpValue);

		tmpName.Format(_T("RecentSearchTerm%u"), a);
		tmpValue = GetString(tmpName);
		if (!tmpValue.IsEmpty())
			m_RecentSearchTerms.AddTail(tmpValue);

		tmpName.Format(_T("RecentReplaceTerm%u"), a);
		tmpValue = GetString(tmpName);
		if (!tmpValue.IsEmpty())
			m_RecentReplaceTerms.AddTail(tmpValue);
	}

	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	if (m_MapSettings.Width<400)
		m_MapSettings.Width = 400;
	if (m_MapSettings.Width>8192)
		m_MapSettings.Width = 8192;

	if (m_MapSettings.Height<300)
		m_MapSettings.Height = 300;
	if (m_MapSettings.Height>4096)
		m_MapSettings.Height = 4096;

	CWnd* pFrame = OpenCommandLine(__argc==2 ? __wargv[1] : NULL);
	if (pFrame)
	{
		if (!FMIsLicensed())
			ShowNagScreen(NAG_FORCE, pFrame);

		FMCheckForUpdate();
	}

	m_AppInitialized = TRUE;

	return TRUE;
}

CWnd* CFlightmapApp::OpenCommandLine(WCHAR* CmdLine)
{
	if (CmdLine)
		if (_wcsicmp(CmdLine, L"/CHECKUPDATE")==0)
		{
			FMCheckForUpdate();
			return NULL;
		}

	CMainWnd* pFrame = new CMainWnd();
	pFrame->Create(new CItinerary(CmdLine));
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return pFrame;
}

INT CFlightmapApp::ExitInstance()
{
	DeleteObject(hFlagIcons[0]);
	DeleteObject(hFlagIcons[1]);

	if (m_AppInitialized)
	{
		WriteInt(_T("UseStatuteMiles"), m_UseStatuteMiles);
		WriteInt(_T("UseBgImages"), m_UseBgImages);
		WriteInt(_T("TextureSize"), m_nTextureSize);
		WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
		WriteInt(_T("MapMergeMetro"), m_MapMergeMetro);
		WriteInt(_T("MapZoomFactor"), m_MapZoomFactor);
		WriteInt(_T("MapAutosize"), m_MapAutosize);
		WriteInt(_T("GlobeLatitude"), m_GlobeLatitude);
		WriteInt(_T("GlobeLongitude"), m_GlobeLongitude);
		WriteInt(_T("GlobeZoom"), m_GlobeZoom);
		WriteInt(_T("GlobeAntialising"), m_GlobeAntialising);
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeMergeMetro"), m_GlobeMergeMetro);
		WriteInt(_T("GlobeUseColors"), m_GlobeUseColors);
		WriteInt(_T("GlobeClamp"), m_GlobeClamp);
		WriteInt(_T("GlobeShowSpots"), m_GlobeShowSpots);
		WriteInt(_T("GlobeShowAirportIATA"), m_GlobeShowAirportIATA);
		WriteInt(_T("GlobeShowAirportNames"), m_GlobeShowAirportNames);
		WriteInt(_T("GlobeShowGPS"), m_GlobeShowGPS);
		WriteInt(_T("GlobeShowFlightCount"), m_GlobeShowFlightCount);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
		WriteInt(_T("GoogleEarthMergeMetro"), m_GoogleEarthMergeMetro);
		WriteInt(_T("GoogleEarthUseCount"), m_GoogleEarthUseCount);
		WriteInt(_T("GoogleEarthUseColors"), m_GoogleEarthUseColors);
		WriteInt(_T("GoogleEarthClamp"), m_GoogleEarthClamp);
		WriteInt(_T("MergeDirections"), m_MergeDirections);
		WriteInt(_T("MergeAwards"), m_MergeAwards);
		WriteInt(_T("MergeClasses"), m_MergeClasses);
		WriteBinary(_T("CustomColors"), (LPBYTE)&m_CustomColors, sizeof(m_CustomColors));
		WriteInt(_T("MapBackground"), m_MapSettings.Background);
		WriteInt(_T("MapBackgroundColor"), m_MapSettings.BackgroundColor);
		WriteInt(_T("MapWidth"), m_MapSettings.Width);
		WriteInt(_T("MapHeight"), m_MapSettings.Height);
		WriteInt(_T("MapCenterPacific"), m_MapSettings.CenterPacific);
		WriteInt(_T("MapWideBorder"), m_MapSettings.WideBorder);
		WriteInt(_T("MapShowFlightRoutes"), m_MapSettings.ShowFlightRoutes);
		WriteInt(_T("MapStraightLines"), m_MapSettings.StraightLines);
		WriteInt(_T("MapArrows"), m_MapSettings.Arrows);
		WriteInt(_T("MapUseCountWidth"), m_MapSettings.UseCountWidth);
		WriteInt(_T("MapUseCountOpacity"), m_MapSettings.UseCountOpacity);
		WriteInt(_T("MapNoteDistance"), m_MapSettings.NoteDistance);
		WriteInt(_T("MapNoteFlightTime"), m_MapSettings.NoteFlightTime);
		WriteInt(_T("MapNoteFlightCount"), m_MapSettings.NoteFlightCount);
		WriteInt(_T("MapNoteCarrier"), m_MapSettings.NoteCarrier);
		WriteInt(_T("MapNoteEquipment"), m_MapSettings.NoteEquipment);
		WriteInt(_T("MapNoteSmallFont"), m_MapSettings.NoteSmallFont);
		WriteInt(_T("MapNoteInnerColor"), m_MapSettings.NoteInnerColor);
		WriteInt(_T("MapNoteOuterColor"), m_MapSettings.NoteOuterColor);
		WriteInt(_T("MapUseColors"), m_MapSettings.UseColors);
		WriteInt(_T("MapRouteColor"), m_MapSettings.RouteColor);
		WriteInt(_T("MapShowLocations"), m_MapSettings.ShowLocations);
		WriteInt(_T("MapLocationInnerColor"), m_MapSettings.LocationInnerColor);
		WriteInt(_T("MapLocationOuterColor"), m_MapSettings.LocationOuterColor);
		WriteInt(_T("MapShowIATACodes"), m_MapSettings.ShowIATACodes);
		WriteInt(_T("MapIATAInnerColor"), m_MapSettings.IATAInnerColor);
		WriteInt(_T("MapIATAOuterColor"), m_MapSettings.IATAOuterColor);

		WriteInt(_T("FindReplaceFlags"), m_FindReplaceSettings.Flags);

		WriteBinary(_T("ColumnOrder"), (LPBYTE)&m_ViewParameters.ColumnOrder, sizeof(m_ViewParameters.ColumnOrder));
		WriteBinary(_T("ColumnWidth"), (LPBYTE)&m_ViewParameters.ColumnWidth, sizeof(m_ViewParameters.ColumnWidth));

		CString tmpName;
		UINT a;

		a = 0;
		for (POSITION p=m_RecentFiles.GetHeadPosition(); p && (a<10); a++)
		{
			tmpName.Format(_T("RecentFile%u"), a);
			WriteString(tmpName, m_RecentFiles.GetNext(p));
		}

		a = 0;
		for (POSITION p=m_RecentSearchTerms.GetHeadPosition(); p && (a<10); a++)
		{
			tmpName.Format(_T("RecentSearchTerm%u"), a);
			WriteString(tmpName, m_RecentSearchTerms.GetNext(p));
		}

		a = 0;
		for (POSITION p=m_RecentReplaceTerms.GetHeadPosition(); p && (a<10); a++)
		{
			tmpName.Format(_T("RecentReplaceTerm%u"), a);
			WriteString(tmpName, m_RecentReplaceTerms.GetNext(p));
		}
	}

	return FMApplication::ExitInstance();
}


void CFlightmapApp::Quit()
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->SendMessage(WM_CLOSE);
}

void CFlightmapApp::Broadcast(UINT Message)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->PostMessage(Message);
}

void CFlightmapApp::AddToRecentList(const CString& FileName)
{
	for (POSITION p=m_RecentFiles.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		if (m_RecentFiles.GetNext(p)==FileName)
			m_RecentFiles.RemoveAt(pl);
	}

	m_RecentFiles.AddHead(FileName);
}

void CFlightmapApp::AddRecentList(CDialogMenuPopup* pPopup)
{
	if (!m_RecentFiles.IsEmpty())
	{
		pPopup->AddCaption(IDS_RECENTFILES);

		UINT a=0;
		for (POSITION p=m_RecentFiles.GetHeadPosition(); p; a++)
			pPopup->AddFile(IDM_FILE_RECENT+a, m_RecentFiles.GetNext(p), m_RecentFiles.GetCount()<=5 ? CDMB_LARGE : CDMB_SMALL);
	}
}

void CFlightmapApp::OpenAirportGoogleEarth(FMAirport* pAirport)
{
	ASSERT(pAirport);

	if (m_PathGoogleEarth.IsEmpty())
		return;

	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sFlightmap%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	// Datei erzeugen
	CGoogleEarthFile f;
	if (!f.Open(szTempName))
	{
		FMErrorBox(IDS_DRIVENOTREADY, GetActiveWindow());
	}
	else
	{
		try
		{
			f.WriteAirport(pAirport);
			f.Close();

			ShellExecute(GetForegroundWindow(), _T("open"), szTempName, NULL, NULL, SW_SHOWNORMAL);
		}
		catch(CFileException ex)
		{
			FMErrorBox(IDS_DRIVENOTREADY, GetActiveWindow());
			f.Close();
		}
	}
}

void CFlightmapApp::OpenAirportGoogleEarth(CHAR* Code)
{
	FMAirport* pAirport = NULL;
	if (FMIATAGetAirportByCode(Code, &pAirport))
		OpenAirportGoogleEarth(pAirport);
}

void CFlightmapApp::OpenAirportLiquidFolders(CHAR* Code)
{
	WCHAR Parameter[4];
	MultiByteToWideChar(CP_ACP, 0, Code, -1, Parameter, 4);

	ShellExecute(GetForegroundWindow(), _T("open"), m_PathLiquidFolders, Parameter, NULL, SW_SHOWNORMAL);
}


void CFlightmapApp::PrintPageHeader(CDC& dc, CRect& rect, const DOUBLE Spacer, const DOCINFO& di)
{
	Graphics g(dc);
	g.SetPageUnit(UnitPixel);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.DrawImage(GetCachedResourceImage(IDB_FLIGHTMAP_256), (REAL)(rect.left*0.95), (REAL)rect.top, (REAL)(Spacer*2.0), (REAL)(Spacer*2.0));

	CFont fntTitle;
	fntTitle.CreateFont((INT)Spacer, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Letter Gothic"));

	CFont fntSubtitle;
	fntSubtitle.CreateFont((INT)(Spacer*0.75), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Tahoma"));

	CFont* pOldFont = dc.SelectObject(&fntTitle);

	CRect rectTitle((INT)(Spacer*3.5), (INT)Spacer, rect.right, (INT)(Spacer*3.0));
	dc.SetTextColor(0x606060);
	dc.DrawText(di.lpszDocName, -1, rectTitle, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_TOP);

	dc.SelectObject(&fntSubtitle);
	CString Subtitle;
	if (FMIsLicensed())
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		WCHAR Date[256] = L"";
		WCHAR Time[256] = L"";
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, Date, 256);
		GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, Time, 256);

		Subtitle.Format(IDS_PRINTED_REGISTERED, Date, Time);
	}
	else
	{
		ENSURE(Subtitle.LoadString(IDS_PRINTED_UNREGISTERED));
	}

	CRect rectSubtitle((INT)(Spacer*3.5), (INT)(Spacer*2.15), rect.right, (INT)(Spacer*3.0));
	dc.SetTextColor(0x000000);
	dc.DrawText(Subtitle, rectSubtitle, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_TOP);

	dc.SelectObject(pOldFont);

	rect.top += (INT)(Spacer*3.0);
}


void CFlightmapApp::OnAppAbout()
{
	AboutDlg dlg(m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
	{
		if (m_UseBgImages!=dlg.m_UseBgImages)
		{
			m_UseBgImages = dlg.m_UseBgImages;
			PostMessage(HWND_BROADCAST, m_UseBgImagesChangedMsg, (WPARAM)m_UseBgImages, NULL);
		}

		if (m_UseStatuteMiles!=dlg.m_UseStatuteMiles)
		{
			m_UseStatuteMiles = dlg.m_UseStatuteMiles;
			PostMessage(HWND_BROADCAST, m_DistanceSettingChangedMsg, (WPARAM)m_UseStatuteMiles, NULL);
		}
	}
}
