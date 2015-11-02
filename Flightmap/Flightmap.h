
// Flightmap.h: Hauptheaderdatei für die Flightmap-Anwendung
//

#pragma once
#include "CDataGrid.h"
#include "CFileView.h"
#include "CItinerary.h"
#include "Database.h"
#include "FMCommDlg.h"
#include "resource.h"


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
	BOOL UseCountWidth;
	BOOL UseCountOpacity;
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
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	void Quit();
	void Broadcast(UINT Message);
	void AddToRecentList(const CString& FileName);
	void AddRecentList(CDialogMenuPopup* pPopup);
	void OpenAirportGoogleEarth(FMAirport* pAirport);
	void OpenAirportGoogleEarth(CHAR* Code);
	void OpenAirportLiquidFolders(CHAR* Code);
	void PrintPageHeader(CDC& dc, CRect& rect, const DOUBLE Spacer, const DOCINFO& di);

	CList<CString> m_RecentFiles;
	CString m_PathGoogleEarth;
	CString m_PathLiquidFolders;

	HBITMAP hFlagIcons[2];

	CLIPFORMAT CF_FLIGHTS;

	BOOL m_UseStatuteMiles;

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	MapSettings m_MapSettings;

	FindReplaceSettings m_FindReplaceSettings;
	CList<CString> m_RecentSearchTerms;
	CList<CString> m_RecentReplaceTerms;

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
	BOOL m_GoogleEarthUseCount;
	BOOL m_GoogleEarthUseColors;
	BOOL m_GoogleEarthClamp;
	// Statistics
	BOOL m_MergeDirections;
	BOOL m_MergeAwards;
	BOOL m_MergeClasses;

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	BOOL m_AppInitialized;
};

extern CFlightmapApp theApp;
