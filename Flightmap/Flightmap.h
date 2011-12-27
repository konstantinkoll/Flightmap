
// Flightmap.h: Hauptheaderdatei für die Flightmap-Anwendung
//

#pragma once
#include "Database.h"
#include "FMCommDlg.h"
#include "resource.h"
#include <hash_map>


// Globe textures

#define FMTextureNone     -1
#define FMTextureAuto     0
#define FMTexture1024     1
#define FMTexture2048     2
#define FMTexture4096     3
#define FMTexture8192     4


// CFlightmapApp:
// Siehe Flightmap.cpp für die Implementierung dieser Klasse
//

#define WM_3DSETTINGSCHANGED      WM_USER+100

class CFlightmapApp : public FMApplication
{
public:
	CFlightmapApp();

	virtual BOOL InitInstance();
	virtual INT ExitInstance();

	void AddFrame(CMainWindow* pFrame);
	void KillFrame(CMainWindow* pVictim);
	void Quit();
	void Broadcast(UINT message);
	void OpenAirportGoogleEarth(FMAirport* pAirport);
	void OpenAirportGoogleEarth(CHAR* Code);

	CList<CMainWindow*> m_MainFrames;
	CString m_PathGoogleEarth;

	BOOL m_UseStatuteMiles;

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	INT m_GlobeLatitude;
	INT m_GlobeLongitude;
	INT m_GlobeZoom;
	BOOL m_GlobeHQModel;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShadows;
	BOOL m_GlobeShowSpots;
	BOOL m_GlobeShowAirportNames;
	BOOL m_GlobeShowGPS;
	BOOL m_GlobeShowFlightCount;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;

protected:
	BOOL m_AppInitialized;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFlightmapApp theApp;
