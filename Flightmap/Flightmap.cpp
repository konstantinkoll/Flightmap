
// Flightmap.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "Flightmap.h"
#include "CGoogleEarthFile.h"
#include "CMainWnd.h"


// CFlightmapApp

BEGIN_MESSAGE_MAP(CFlightmapApp, FMApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CFlightmapApp-Erstellung

CFlightmapApp::CFlightmapApp()
	: FMApplication()
{
	m_NagCounter = 3;
	m_AppInitialized = FALSE;
}


// Das einzige CFlightmapApp-Objekt

CFlightmapApp theApp;


// CFlightmapApp-Initialisierung

BOOL CFlightmapApp::InitInstance()
{
	if (!FMApplication::InitInstance())
		return FALSE;

	// Pfad zu Google Earth
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Google\\Google Earth Plus"), 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		CHAR lszValue[255];
		DWORD dwSize = 255;

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			m_PathGoogleEarth = lszValue;

		RegCloseKey(hKey);
	}

	// Registry auslesen
	SetRegistryBase();
	m_UseStatuteMiles = GetInt(_T("UseStatuteMiles"), FALSE);
	m_UseBgImages = GetInt(_T("UseBgImages"), TRUE);
	m_nTextureSize = GetInt(_T("TextureSize"), FMTextureAuto);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), FMTexture4096);
	m_GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeAntialising = GetInt(_T("GlobeAntialising"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShadows = GetInt(_T("GlobeShadows"), TRUE);
	m_GlobeUseColors = GetInt(_T("GlobeUseColors"), TRUE);
	m_GlobeClamp = GetInt(_T("GlobeClamp"), FALSE);
	m_GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobeShowGPS = GetInt(_T("GlobeShowGPS"), FALSE);
	m_GlobeShowFlightCount = GetInt(_T("GlobeShowFlightCount"), FALSE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);
	m_GoogleEarthUseColors = GetInt(_T("GoogleEarthUseColors"), TRUE);
	m_GoogleEarthClamp = GetInt(_T("GoogleEarthClamp"), FALSE);
	GetBinary(_T("CustomColors"), &m_CustomColors, sizeof(m_CustomColors));
	m_MapSettings.Background = GetInt(_T("MapBackground"), 0);
	m_MapSettings.BackgroundColor = GetInt(_T("MapBackgroundColor"), 0xF0F0F0);
	m_MapSettings.CenterPacific = GetInt(_T("MapCenterPacific"), FALSE);
	m_MapSettings.ShowFlightRoutes = GetInt(_T("MapShowFlightRoutes"), TRUE);
	m_MapSettings.ShowFlightRoutes = GetInt(_T("MapStraightLines"), FALSE);
	m_MapSettings.UseColors = GetInt(_T("MapUseColors"), TRUE);
	m_MapSettings.RouteColor = GetInt(_T("MapRouteColor"), 0xFFFFFF);
	m_MapSettings.ShowLocations = GetInt(_T("MapShowLocations"), TRUE);
	m_MapSettings.LocationInnerColor = GetInt(_T("MapLocationInnerColor"), 0x0000FF);
	m_MapSettings.LocationOuterColor = GetInt(_T("MapLocationOuterColor"), 0xFFFFFF);
	m_MapSettings.ShowIATACodes = GetInt(_T("MapShowIATACodes"), TRUE);
	m_MapSettings.IATAInnerColor = GetInt(_T("MapIATAInnerColor"), 0xFFFFFF);
	m_MapSettings.IATAOuterColor = GetInt(_T("MapIATAOuterColor"), 0x000000);

	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	CMainWnd* pFrame = new CMainWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	if (!FMIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, pFrame);

	FMCheckForUpdate(FALSE, pFrame);

	//FMProgressDlg dlg(NULL, NULL, NULL);
	//dlg.DoModal();

	m_AppInitialized = TRUE;

	return TRUE;
}

INT CFlightmapApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		WriteInt(_T("UseStatuteMiles"), m_UseStatuteMiles);
		WriteInt(_T("UseBgImages"), m_UseBgImages);
		WriteInt(_T("TextureSize"), m_nTextureSize);
		WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
		WriteInt(_T("GlobeLatitude"), m_GlobeLatitude);
		WriteInt(_T("GlobeLongitude"), m_GlobeLongitude);
		WriteInt(_T("GlobeZoom"), m_GlobeZoom);
		WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
		WriteInt(_T("GlobeAntialising"), m_GlobeAntialising);
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeShadows"), m_GlobeShadows);
		WriteInt(_T("GlobeUseColors"), m_GlobeUseColors);
		WriteInt(_T("GlobeClamp"), m_GlobeClamp);
		WriteInt(_T("GlobeShowSpots"), m_GlobeShowSpots);
		WriteInt(_T("GlobeShowAirportNames"), m_GlobeShowAirportNames);
		WriteInt(_T("GlobeShowGPS"), m_GlobeShowGPS);
		WriteInt(_T("GlobeShowFlightCount"), m_GlobeShowFlightCount);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
		WriteInt(_T("GoogleEarthUseColors"), m_GoogleEarthUseColors);
		WriteInt(_T("GoogleEarthClamp"), m_GoogleEarthClamp);
		WriteBinary(_T("CustomColors"), (LPBYTE)&m_CustomColors, sizeof(m_CustomColors));
		WriteInt(_T("MapBackground"), m_MapSettings.Background);
		WriteInt(_T("MapBackgroundColor"), m_MapSettings.BackgroundColor);
		WriteInt(_T("MapCenterPacific"), m_MapSettings.CenterPacific);
		WriteInt(_T("MapShowFlightRoutes"), m_MapSettings.ShowFlightRoutes);
		WriteInt(_T("MapStraightLines"), m_MapSettings.ShowFlightRoutes);
		WriteInt(_T("MapUseColors"), m_MapSettings.UseColors);
		WriteInt(_T("MapRouteColor"), m_MapSettings.RouteColor);
		WriteInt(_T("MapShowLocations"), m_MapSettings.ShowLocations);
		WriteInt(_T("MapLocationInnerColor"), m_MapSettings.LocationInnerColor);
		WriteInt(_T("MapLocationOuterColor"), m_MapSettings.LocationOuterColor);
		WriteInt(_T("MapShowIATACodes"), m_MapSettings.ShowIATACodes);
		WriteInt(_T("MapIATAInnerColor"), m_MapSettings.IATAInnerColor);
		WriteInt(_T("MapIATAOuterColor"), m_MapSettings.IATAOuterColor);
	}

	return FMApplication::ExitInstance();
}


void CFlightmapApp::AddFrame(CMainWindow* pFrame)
{
	m_MainFrames.AddTail(pFrame);
	m_pMainWnd = pFrame;
	m_pActiveWnd = NULL;
}

void CFlightmapApp::KillFrame(CMainWindow* pVictim)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CMainWindow* pFrame = m_MainFrames.GetNext(p);
		if (pFrame==pVictim)
		{
			m_MainFrames.RemoveAt(pl);
		}
		else
		{
			m_pMainWnd = pFrame;
		}
	}
}

void CFlightmapApp::Quit()
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CMainWindow* pFrame = m_MainFrames.GetNext(p);
		m_MainFrames.RemoveAt(pl);

		pFrame->SendMessage(WM_CLOSE);
	}

	m_pMainWnd = m_pActiveWnd = NULL;
}

void CFlightmapApp::Broadcast(UINT message)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
		m_MainFrames.GetNext(p)->PostMessage(message);
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
		FMErrorBox(IDS_DRIVENOTREADY);
	}
	else
	{
		try
		{
			f.WriteAirport(pAirport);
			f.Close();

			ShellExecute(GetForegroundWindow(), _T("open"), szTempName, NULL, NULL, SW_SHOW);
		}
		catch(CFileException ex)
		{
			FMErrorBox(IDS_DRIVENOTREADY);
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

void CFlightmapApp::GetBinary(LPCTSTR lpszEntry, void* pData, UINT size)
{
	UINT sz;
	LPBYTE buf = NULL;
	CWinAppEx::GetBinary(lpszEntry, &buf, &sz);
	if (buf)
	{
		if (sz<size)
			size = sz;
		memcpy_s(pData, size, buf, size);
		free(buf);
	}
}


void CFlightmapApp::OnAppAbout()
{
	AboutDlg dlg(m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
	{
		m_UseStatuteMiles = dlg.m_UseStatuteMiles;

		if (m_UseBgImages!=dlg.m_UseBgImages)
		{
			m_UseBgImages = dlg.m_UseBgImages;
			SendMessage(HWND_BROADCAST, msgUseBgImagesChanged, NULL, NULL);
		}
	}
}
