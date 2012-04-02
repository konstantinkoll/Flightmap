
// Flightmap.h: Hauptheaderdatei f�r die Flightmap-Anwendung
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
// Siehe Flightmap.cpp f�r die Implementierung dieser Klasse
//

#define WM_3DSETTINGSCHANGED     WM_USER+100

struct MapSettings
{
	INT Background;
	COLORREF BackgroundColor;
	UINT Width;
	UINT Height;
	BOOL CenterPacific;
	BOOL ShowFlightRoutes;
	BOOL StraightLines;
	BOOL Arrows;
	BOOL UseColors;
	COLORREF RouteColor;
	BOOL ShowLocations;
	COLORREF LocationInnerColor;
	COLORREF LocationOuterColor;
	BOOL ShowIATACodes;
	COLORREF IATAInnerColor;
	COLORREF IATAOuterColor;
};

struct AIRX_Header
{
	DWORD Magic;
	UINT MetadataRecordSize;
	UINT FlightCount;
	UINT FlightRecordSize;
	UINT AttachmentCount;
};

struct AIRX_Metadata
{
	WCHAR Author[256];
	WCHAR Title[256];
	WCHAR Comments[256];
};

struct AIRX_Location
{
	CHAR Code[4];
	SYSTEMTIME Time;
	WCHAR Gate[8];
};

#define AIRX_AwardFlight      1
#define AIRX_BusinessTrip     2
#define AIRX_LeisureTrip      4

#define AIRX_Economy          'Y'
#define AIRX_EconomyPlus      '+'
#define AIRX_Business         'J'
#define AIRX_First            'F'
#define AIRX_Crew             'C'

struct AIRX_Flight
{
	DWORD Flags;
	AIRX_Location From;
	AIRX_Location To;
	FMGeoCoordinates Waypoint;
	DOUBLE DistanceNM;
	WCHAR Carrier[64];
	WCHAR Equipment[64];
	WCHAR Comments[256];
	WCHAR Registration[16];
	WCHAR Name[64];
	WCHAR Flight[7];
	WCHAR EtixCode[7];
	CHAR Class;
	WCHAR Seat[4];
	COLORREF Color;
	UINT MilesAward;
	UINT MilesStatus;
	WCHAR Fare[16];
	WCHAR Codeshare[64];
};

struct AIRX_Attachment
{
	WCHAR Name[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	INT64 Size;
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

	CList<CMainWindow*> m_MainFrames;
	CString m_PathGoogleEarth;

	BOOL m_UseStatuteMiles;

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	MapSettings m_MapSettings;

	// Map view
	INT m_MapZoomFactor;
	BOOL m_MapAutosize;
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
