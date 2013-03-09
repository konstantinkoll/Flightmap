
// Flightmap.h: Hauptheaderdatei für die Flightmap-Anwendung
//

#pragma once
#include "CDataGrid.h"
#include "CItinerary.h"
#include "Database.h"
#include "FMCommDlg.h"
#include "resource.h"

#define PI     3.14159265358979323846


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
	INT Background;
	COLORREF BackgroundColor;
	UINT Width;
	UINT Height;
	BOOL CenterPacific;
	BOOL WideBorder;
	BOOL ShowFlightRoutes;
	BOOL StraightLines;
	BOOL Arrows;
	BOOL UseCount;
	BOOL NoteDistance;
	BOOL NoteFlightTime;
	BOOL NoteFlightCount;
	BOOL NoteCarrier;
	BOOL NoteEquipment;
	BOOL NoteSmallFont;
	COLORREF NoteInnerColor;
	COLORREF NoteOuterColor;
	BOOL UseColors;
	COLORREF RouteColor;
	BOOL ShowLocations;
	COLORREF LocationInnerColor;
	COLORREF LocationOuterColor;
	BOOL ShowIATACodes;
	COLORREF IATAInnerColor;
	COLORREF IATAOuterColor;
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
	void AddToRecentList(CString FileName);
	void AddRecentList(CDialogMenuPopup* pPopup);
	void OpenAirportGoogleEarth(FMAirport* pAirport);
	void OpenAirportGoogleEarth(CHAR* Code);
	static void PrintPageHeader(CDC& dc, CRect& rect, const DOUBLE Spacer, const DOCINFO& di);

	CList<CMainWindow*> m_MainFrames;
	CList<CString> m_RecentFiles;
	CString m_PathGoogleEarth;

	HBITMAP m_FlagIcons16[2];
	CImageListTransparent m_FlagIcons32;

	CLIPFORMAT CF_FLIGHTS;

	BOOL m_UseStatuteMiles;

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	MapSettings m_MapSettings;

	// Spreadsheet
	ViewParameters m_ViewParameters;
	// Map
	BOOL m_MapMergeMetro;
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
	// User view settings
	BOOL m_GlobeMergeMetro;
	BOOL m_GlobeUseColors;
	BOOL m_GlobeClamp;
	BOOL m_GlobeShowSpots;
	BOOL m_GlobeShowAirportIATA;
	BOOL m_GlobeShowAirportNames;
	BOOL m_GlobeShowGPS;
	BOOL m_GlobeShowFlightCount;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;
	// Google Earth
	BOOL m_GoogleEarthMergeMetro;
	BOOL m_GoogleEarthUseColors;
	BOOL m_GoogleEarthClamp;
	// Statistics
	BOOL m_MergeDirections;
	BOOL m_MergeAwards;
	BOOL m_MergeClasses;

protected:
	BOOL m_AppInitialized;

	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT size);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFlightmapApp theApp;
