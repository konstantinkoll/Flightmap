
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

#define WM_3DSETTINGSCHANGED           WM_USER+100
#define WM_DISTANCESETTINGSCHANGED     WM_USER+101

struct MapSettings
{
	UINT Width;
	UINT Height;

	INT Background;
	COLORREF BackgroundColor;
	BOOL CenterPacific;
	BOOL WideBorder;
	INT ForegroundScale;

	BOOL ShowLocations;
	COLORREF LocationsInnerColor;
	COLORREF LocationsOuterColor;
	BOOL ShowIATACodes;
	COLORREF IATACodesInnerColor;
	COLORREF IATACodesOuterColor;

	BOOL ShowRoutes;
	COLORREF RouteColor;
	BOOL UseColors;
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
};

class CFlightmapApp : public FMApplication
{
public:
	CFlightmapApp();

	virtual BOOL InitInstance();
	virtual BOOL OpenCommandLine(LPWSTR CmdLine=NULL);
	virtual INT ExitInstance();

	void Broadcast(UINT Message);
	static void AddStringToList(CList<CString>& List, const CString& Str);
	void AddToRecentFiles(const CString& Path);
	void AddToRecentSearchTerms(const CString& Str);
	void AddToRecentReplaceTerms(const CString& Str);
	void OpenAirportGoogleEarth(LPCAIRPORT lpcAirport);
	void OpenAirportGoogleEarth(LPCSTR Code);
	void OpenAirportLiquidFolders(LPCSTR Code);
	void PrintPageHeader(CDC& dc, CRect& rect, const DOUBLE Spacer, const DOCINFO& di);

	CList<CString> m_RecentFiles;
	WCHAR m_PathGoogleEarth[MAX_PATH];
	CString m_PathLiquidFolders;

	CLIPFORMAT CF_FLIGHTS;

	BOOL m_UseStatuteMiles;

	FindReplaceSettings m_FindReplaceSettings;
	CList<CString> m_RecentSearchTerms;
	CList<CString> m_RecentReplaceTerms;

	// Spreadsheet
	ViewParameters m_ViewParameters;

	// Map
	MapSettings m_MapSettings;
	BOOL m_MapMergeMetro;

	// Map view
	INT m_MapZoomFactor;

	// Globe view
	BOOL m_GlobeMergeMetro;

	INT m_GlobeLatitude;
	INT m_GlobeLongitude;
	INT m_GlobeZoom;

	BOOL m_GlobeShowLocations;
	BOOL m_GlobeShowAirportIATA;
	BOOL m_GlobeShowAirportNames;
	BOOL m_GlobeShowCoordinates;
	BOOL m_GlobeShowDescriptions;
	BOOL m_GlobeDarkBackground;

	// Google Earth
	BOOL m_GoogleEarthMergeMetro;
	BOOL m_GoogleEarthUseCount;
	BOOL m_GoogleEarthUseColors;
	BOOL m_GoogleEarthClampHeight;

	// Statistics
	BOOL m_StatisticsMergeMetro;
	BOOL m_StatisticsMergeDirections;
	BOOL m_StatisticsMergeAwards;
	BOOL m_StatisticsMergeClasses;

protected:
	afx_msg void OnBackstageAbout();
	DECLARE_MESSAGE_MAP()

	BOOL m_AppInitialized;
};

extern CFlightmapApp theApp;

inline void CFlightmapApp::AddToRecentFiles(const CString& Path)
{
	ASSERT(!Path.IsEmpty());

	theApp.AddStringToList(m_RecentFiles, Path);
}

inline void CFlightmapApp::AddToRecentSearchTerms(const CString& Str)
{
	ASSERT(!Str.IsEmpty());

	theApp.AddStringToList(m_RecentSearchTerms, Str);
}

inline void CFlightmapApp::AddToRecentReplaceTerms(const CString& Str)
{
	ASSERT(!Str.IsEmpty());

	theApp.AddStringToList(m_RecentReplaceTerms, Str);
}
