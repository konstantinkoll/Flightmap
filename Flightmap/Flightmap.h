
// Flightmap.h: Hauptheaderdatei für die Flightmap-Anwendung
//

#pragma once
#include "Database.h"
#include "FMCommDlg.h"
#include "resource.h"


#define PI                3.14159265358979323846

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

#define WM_3DSETTINGSCHANGED     WM_USER+100

struct MapSettings
{
	BOOL CenterPacific;
	BOOL ShowFlightRoutes;
	BOOL UseColors;
	BOOL StraightLines;
	BOOL ShowLocations;
	BOOL ShowIATACodes;
};

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
	BOOL ChooseColor(COLORREF& clr, CWnd* pParentWnd=NULL);

	CList<CMainWindow*> m_MainFrames;
	CString m_PathGoogleEarth;

	BOOL m_UseStatuteMiles;

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	MapSettings m_MapSettings;

	// Viewport
	INT m_GlobeLatitude;
	INT m_GlobeLongitude;
	INT m_GlobeZoom;
	// 3D settings
	BOOL m_GlobeHQModel;
	BOOL m_GlobeAntialising;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShadows;
	// User view settings
	BOOL m_GlobeUseColors;
	BOOL m_GlobeClamp;
	BOOL m_GlobeShowSpots;
	BOOL m_GlobeShowAirportNames;
	BOOL m_GlobeShowGPS;
	BOOL m_GlobeShowFlightCount;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;

	BOOL m_GoogleEarthUseColors;
	BOOL m_GoogleEarthClamp;

protected:
	BOOL m_AppInitialized;

	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFlightmapApp theApp;
