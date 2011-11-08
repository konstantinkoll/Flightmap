
// Flightmap.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Flightmap.h"
#include "CAboutDlg.h"
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
	FMApplication::InitInstance();

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
	m_ReduceVisuals = GetInt(_T("ReduceVisuals"), FALSE);
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), FMTexture4096);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShadows = GetInt(_T("GlobeShadows"), TRUE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);

	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	if (!FMIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE);

	CMainWnd* pFrame = new CMainWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);

	//OnAppAbout();

	m_AppInitialized = TRUE;

	return TRUE;
}

INT CFlightmapApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		WriteInt(_T("UseStatuteMiles"), m_UseStatuteMiles);
		WriteInt(_T("ReduceVisuals"), m_ReduceVisuals);
		WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeShadows"), m_GlobeShadows);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
		WriteInt(_T("TextureSize"), m_nTextureSize);
		WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
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

void CFlightmapApp::OnAppAbout()
{
	CAboutDlg dlg(m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
	{
		m_UseStatuteMiles = dlg.m_UseStatuteMiles;
		m_ReduceVisuals = dlg.m_ReduceVisuals;
	}
}
