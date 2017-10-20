
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
				wcscpy_s(cdsw.Command, MAX_PATH, (LPCWSTR)lParam);

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
	ON_COMMAND(IDM_BACKSTAGE_ABOUT, OnBackstageAbout)
END_MESSAGE_MAP()


// CFlightmapApp-Erstellung

CFlightmapApp::CFlightmapApp()
	: FMApplication(theAppID)
{
	m_AppInitialized = FALSE;

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

	// RestartManager
	if (m_KernelLibLoaded)
		zRegisterApplicationRestart(L"", 11);	// RESTART_NO_CRASH | RESTART_NO_HANG | RESTART_NO_REBOOT

	// Pfad zu Google Earth
	DWORD dwSize = sizeof(m_PathGoogleEarth)/sizeof(WCHAR);
	if (FAILED(AssocQueryString(ASSOCF_REMAPRUNDLL | ASSOCF_INIT_IGNOREUNKNOWN, ASSOCSTR_EXECUTABLE, L".kml", NULL, m_PathGoogleEarth, &dwSize)))
		m_PathGoogleEarth[0] = L'\0';

	// Pfad zu liquidFOLDERS
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\liquidFOLDERS"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		WCHAR lszValue[256];
		DWORD dwSize = sizeof(lszValue);

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			m_PathLiquidFolders = lszValue;

		RegCloseKey(hKey);
	}

	// Registry auslesen
	SetRegistryBase();

	m_ModelQuality = (GLModelQuality)GetInt(_T("ModelQuality"), MODELULTRA);
	m_TextureQuality = (GLTextureQuality)GetInt(_T("TextureQuality"), TEXTUREMEDIUM);
	m_TextureCompress = GetInt(_T("TextureCompress"), FALSE);

	m_UseStatuteMiles = GetInt(_T("UseStatuteMiles"), FALSE);
	GetBinary(_T("ColorHistory"), &m_ColorHistory, sizeof(m_ColorHistory));

	m_MapMergeMetro = GetInt(_T("MapMergeMetro"), FALSE);
	m_MapZoomFactor = GetInt(_T("MapZoomFactor"), 6);
	m_GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_GlobeMergeMetro = GetInt(_T("GlobeMergeMetro"), FALSE);
	m_GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_GlobeShowAirportIATA = GetInt(_T("GlobeShowAirportIATA"), TRUE);
	m_GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobeShowGPS = GetInt(_T("GlobeShowGPS"), FALSE);
	m_GlobeShowMovements = GetInt(_T("GlobeShowMovements"), FALSE);
	m_GlobeDarkBackground = GetInt(_T("GlobeDarkBackground"), FALSE);
	m_GoogleEarthMergeMetro = GetInt(_T("GoogleEarthMergeMetro"), FALSE);
	m_GoogleEarthUseCount = GetInt(_T("GoogleEarthUseCount"), FALSE);
	m_GoogleEarthUseColors = GetInt(_T("GoogleEarthUseColors"), TRUE);
	m_GoogleEarthClampHeight = GetInt(_T("GoogleEarthClampHeight"), FALSE);
	m_StatisticsMergeMetro = GetInt(_T("StatisticsMergeMetro"), FALSE);
	m_StatisticsMergeDirections = GetInt(_T("StatisticsMergeDirections"), TRUE);
	m_StatisticsMergeAwards = GetInt(_T("StatisticsMergeAwards"), FALSE);
	m_StatisticsMergeClasses = GetInt(_T("StatisticsMergeClasses"), TRUE);
	m_MapSettings.Background = GetInt(_T("MapBackground"), 0);
	m_MapSettings.BackgroundColor = GetInt(_T("MapBackgroundColor"), 0xF0F0F0);
	m_MapSettings.CenterPacific = GetInt(_T("MapCenterPacific"), FALSE);
	m_MapSettings.WideBorder = GetInt(_T("MapWideBorder"), FALSE);
	m_MapSettings.ForegroundScale = GetInt(_T("MapForegroundScale"), 0);
	m_MapSettings.Width = GetInt(_T("MapWidth"), 640);
	m_MapSettings.Height = GetInt(_T("MapHeight"), 640);
	m_MapSettings.ShowRoutes = GetInt(_T("MapShowRoutes"), TRUE);
	m_MapSettings.RouteColor = GetInt(_T("MapRouteColor"), 0xFFFFFF);
	m_MapSettings.UseColors = GetInt(_T("MapUseColors"), TRUE);
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
	m_MapSettings.ShowLocations = GetInt(_T("MapShowLocations"), TRUE);
	m_MapSettings.LocationsInnerColor = GetInt(_T("MapLocationsInnerColor"), 0x0000FF);
	m_MapSettings.LocationsOuterColor = GetInt(_T("MapLocationsOuterColor"), 0xFFFFFF);
	m_MapSettings.ShowIATACodes = GetInt(_T("MapShowIATACodes"), TRUE);
	m_MapSettings.IATACodesInnerColor = GetInt(_T("MapIATACodesInnerColor"), 0xFFFFFF);
	m_MapSettings.IATACodesOuterColor = GetInt(_T("MapIATACodesOuterColor"), 0x000000);

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

	for (UINT a=0; a<20; a++)
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

	if (m_MapSettings.Width<320)
		m_MapSettings.Width = 320;
	if (m_MapSettings.Width>8192)
		m_MapSettings.Width = 8192;

	if (m_MapSettings.Height<300)
		m_MapSettings.Height = 300;
	if (m_MapSettings.Height>4096)
		m_MapSettings.Height = 4096;

	// Execute
	CheckForUpdate();

	CWnd* pFrameWnd = OpenCommandLine(__argc==2 ? __wargv[1] : NULL);
	if (pFrameWnd)
		ShowNagScreen(NAG_FORCE, pFrameWnd);

	m_AppInitialized = TRUE;

	return TRUE;
}

CWnd* CFlightmapApp::OpenCommandLine(LPWSTR pCmdLine)
{
	if (pCmdLine)
		if (_wcsicmp(pCmdLine, L"/CHECKUPDATE")==0)
			return NULL;

	CMainWnd* pFrameWnd = new CMainWnd();
	pFrameWnd->Create(new CItinerary(pCmdLine));
	pFrameWnd->ShowWindow(SW_SHOW);

	return pFrameWnd;
}

INT CFlightmapApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		WriteInt(_T("ModelQuality"), m_ModelQuality);
		WriteInt(_T("TextureQuality"), m_TextureQuality);
		WriteInt(_T("TextureCompress"), m_TextureCompress);

		WriteInt(_T("UseStatuteMiles"), m_UseStatuteMiles);
		WriteBinary(_T("ColorHistory"), (LPBYTE)&m_ColorHistory, sizeof(m_ColorHistory));

		WriteInt(_T("MapMergeMetro"), m_MapMergeMetro);
		WriteInt(_T("MapZoomFactor"), m_MapZoomFactor);
		WriteInt(_T("GlobeLatitude"), m_GlobeLatitude);
		WriteInt(_T("GlobeLongitude"), m_GlobeLongitude);
		WriteInt(_T("GlobeZoom"), m_GlobeZoom);
		WriteInt(_T("GlobeMergeMetro"), m_GlobeMergeMetro);
		WriteInt(_T("GlobeShowSpots"), m_GlobeShowSpots);
		WriteInt(_T("GlobeShowAirportIATA"), m_GlobeShowAirportIATA);
		WriteInt(_T("GlobeShowAirportNames"), m_GlobeShowAirportNames);
		WriteInt(_T("GlobeShowGPS"), m_GlobeShowGPS);
		WriteInt(_T("GlobeShowMovements"), m_GlobeShowMovements);
		WriteInt(_T("GlobeDarkBackground"), m_GlobeDarkBackground);
		WriteInt(_T("GoogleEarthMergeMetro"), m_GoogleEarthMergeMetro);
		WriteInt(_T("GoogleEarthUseCount"), m_GoogleEarthUseCount);
		WriteInt(_T("GoogleEarthUseColors"), m_GoogleEarthUseColors);
		WriteInt(_T("GoogleEarthClampHeight"), m_GoogleEarthClampHeight);
		WriteInt(_T("StatisticsMergeMetro"), m_StatisticsMergeMetro);
		WriteInt(_T("StatisticsMergeDirections"), m_StatisticsMergeDirections);
		WriteInt(_T("StatisticsMergeAwards"), m_StatisticsMergeAwards);
		WriteInt(_T("StatisticsMergeClasses"), m_StatisticsMergeClasses);
		WriteInt(_T("MapBackground"), m_MapSettings.Background);
		WriteInt(_T("MapBackgroundColor"), m_MapSettings.BackgroundColor);
		WriteInt(_T("MapCenterPacific"), m_MapSettings.CenterPacific);
		WriteInt(_T("MapWideBorder"), m_MapSettings.WideBorder);
		WriteInt(_T("MapForegroundScale"), m_MapSettings.ForegroundScale);
		WriteInt(_T("MapWidth"), m_MapSettings.Width);
		WriteInt(_T("MapHeight"), m_MapSettings.Height);
		WriteInt(_T("MapShowRoutes"), m_MapSettings.ShowRoutes);
		WriteInt(_T("MapRouteColor"), m_MapSettings.RouteColor);
		WriteInt(_T("MapUseColors"), m_MapSettings.UseColors);
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
		WriteInt(_T("MapShowLocations"), m_MapSettings.ShowLocations);
		WriteInt(_T("MapLocationsInnerColor"), m_MapSettings.LocationsInnerColor);
		WriteInt(_T("MapLocationsOuterColor"), m_MapSettings.LocationsOuterColor);
		WriteInt(_T("MapShowIATACodes"), m_MapSettings.ShowIATACodes);
		WriteInt(_T("MapIATACodesInnerColor"), m_MapSettings.IATACodesInnerColor);
		WriteInt(_T("MapIATACodesOuterColor"), m_MapSettings.IATACodesOuterColor);

		WriteInt(_T("FindReplaceFlags"), m_FindReplaceSettings.Flags);

		WriteBinary(_T("ColumnOrder"), (LPBYTE)&m_ViewParameters.ColumnOrder, sizeof(m_ViewParameters.ColumnOrder));
		WriteBinary(_T("ColumnWidth"), (LPBYTE)&m_ViewParameters.ColumnWidth, sizeof(m_ViewParameters.ColumnWidth));

		CString tmpName;
		UINT a;

		a = 0;
		for (POSITION p=m_RecentFiles.GetHeadPosition(); p && (a<20); )
		{
			tmpName.Format(_T("RecentFile%u"), a++);
			WriteString(tmpName, m_RecentFiles.GetNext(p));
		}

		while (a<20)
		{
			tmpName.Format(_T("RecentFile%u"), a++);
			WriteString(tmpName, _T(""));
		}

		a = 0;
		for (POSITION p=m_RecentSearchTerms.GetHeadPosition(); p && (a<20); )
		{
			tmpName.Format(_T("RecentSearchTerm%u"), a++);
			WriteString(tmpName, m_RecentSearchTerms.GetNext(p));
		}

		a = 0;
		for (POSITION p=m_RecentReplaceTerms.GetHeadPosition(); p && (a<20); )
		{
			tmpName.Format(_T("RecentReplaceTerm%u"), a++);
			WriteString(tmpName, m_RecentReplaceTerms.GetNext(p));
		}
	}

	return FMApplication::ExitInstance();
}


void CFlightmapApp::Broadcast(UINT Message)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->PostMessage(Message);
}

void CFlightmapApp::AddStringToList(CList<CString>& List, const CString& Str)
{
	for (POSITION p=List.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		if (List.GetNext(p)==Str)
			List.RemoveAt(pl);
	}

	List.AddHead(Str);
}

void CFlightmapApp::OpenAirportGoogleEarth(FMAirport* pAirport)
{
	ASSERT(pAirport);

	if (m_PathGoogleEarth[0]==L'\0')
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
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
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
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
			f.Close();
		}
	}
}

void CFlightmapApp::OpenAirportGoogleEarth(LPCSTR Code)
{
	FMAirport* pAirport;
	if (FMIATAGetAirportByCode(Code, &pAirport))
		OpenAirportGoogleEarth(pAirport);
}

void CFlightmapApp::OpenAirportLiquidFolders(LPCSTR Code)
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
	g.DrawImage(GetCachedResourceImage(IDB_FLIGHTMAP_256), (REAL)(rect.left*0.89), (REAL)rect.top, (REAL)(Spacer*2.18), (REAL)(Spacer*2.0));

	CFont fntTitle;
	fntTitle.CreateFont((INT)Spacer, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("DIN Mittelschrift"));

	CFont fntSubtitle;
	fntSubtitle.CreateFont((INT)(Spacer*0.75), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Tahoma"));

	CFont* pOldFont = dc.SelectObject(&fntTitle);

	CRect rectTitle((INT)(Spacer*3.5), (INT)Spacer, rect.right, (INT)(Spacer*3.0));
	dc.SetTextColor(0x404040);
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


void CFlightmapApp::OnBackstageAbout()
{
	CWaitCursor csr;

	AboutDlg dlg(m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
	{
		if (m_UseStatuteMiles!=dlg.m_UseStatuteMiles)
		{
			m_UseStatuteMiles = dlg.m_UseStatuteMiles;
			PostMessage(HWND_BROADCAST, m_DistanceSettingChangedMsg, (WPARAM)m_UseStatuteMiles, NULL);
		}
	}
}
