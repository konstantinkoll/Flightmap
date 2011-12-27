
// Flightmap.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "Flightmap.h"
#include "CMainWnd.h"


void CookAttributeString(CString& tmpStr)
{
	tmpStr.Replace(_T("<"), _T("_"));
	tmpStr.Replace(_T(">"), _T("_"));
	tmpStr.Replace(_T("&"), _T("&amp;"));
	tmpStr.Replace(_T("–"), _T("&#8211;"));
	tmpStr.Replace(_T("—"), _T("&#8212;"));
}

void WriteGoogleAttribute(CStdioFile* f, UINT ResID, CString Value)
{
	if (!Value.IsEmpty())
	{
		CString Name;
		ENSURE(Name.LoadString(ResID));
		CookAttributeString(Name);
		CookAttributeString(Value);

		f->WriteString(_T("&lt;b&gt;"));
		f->WriteString(Name);
		f->WriteString(_T("&lt;/b&gt;: "));
		f->WriteString(Value);
		f->WriteString(_T("&lt;br&gt;"));
	}
}

void WriteGoogleAttribute(CStdioFile* f, UINT ResID, CHAR* Value)
{
	CString tmpStr(Value);
	WriteGoogleAttribute(f, ResID, tmpStr);
}


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
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), FMTexture4096);
	m_GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShadows = GetInt(_T("GlobeShadows"), TRUE);
	m_GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobeShowGPS = GetInt(_T("GlobeShowGPS"), FALSE);
	m_GlobeShowFlightCount = GetInt(_T("GlobeShowFlightCount"), FALSE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);

	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	CMainWnd* pFrame = new CMainWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);

	if (!FMIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, pFrame);

	//OnAppAbout();

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
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeShadows"), m_GlobeShadows);
		WriteInt(_T("GlobeShowSpots"), m_GlobeShowSpots);
		WriteInt(_T("GlobeShowAirportNames"), m_GlobeShowAirportNames);
		WriteInt(_T("GlobeShowGPS"), m_GlobeShowGPS);
		WriteInt(_T("GlobeShowFlightCount"), m_GlobeShowFlightCount);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
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
	szTempName.Format(_T("%sFlightmapS%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	// Datei erzeugen
	CStdioFile f;
	if (!f.Open(szTempName, CFile::modeCreate | CFile::modeWrite))
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
	else
	{
		try
		{
			f.WriteString(_T("<?xml version=\"1.0\"?>\n<kml xmlns=\"http://earth.google.com/kml/2.0\">\n<Document>\n"));
			f.WriteString(_T("<Style id=\"A\"><IconStyle><scale>0.8</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<Style id=\"B\"><IconStyle><scale>1.0</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>1</scale></LabelStyle></Style>\n"));
			f.WriteString(_T("<StyleMap id=\"C\"><Pair><key>normal</key><styleUrl>#A</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#B</styleUrl></Pair></StyleMap>\n"));
			f.WriteString(_T("<Placemark>\n<name>"));

			CString tmpStr(pAirport->Code);
			CookAttributeString(tmpStr);
			f.WriteString(tmpStr);

			f.WriteString(_T("</name>\n<description>"));
			WriteGoogleAttribute(&f, IDS_AIRPORT_NAME, pAirport->Name);
			WriteGoogleAttribute(&f, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
			WriteGoogleAttribute(&f, IDS_AIRPORT_CODE, tmpStr);
			FMGeoCoordinatesToString(pAirport->Location, tmpStr);
			WriteGoogleAttribute(&f, IDS_AIRPORT_LOCATION, tmpStr);
			f.WriteString(_T("&lt;div&gt;</description>\n"));

			f.WriteString(_T("<styleUrl>#C</styleUrl>\n"));
			tmpStr.Format(_T("<Point><coordinates>%.6lf,%.6lf,-5000</coordinates></Point>\n"), pAirport->Location.Longitude, -pAirport->Location.Latitude);
			f.WriteString(tmpStr);
			f.WriteString(_T("</Placemark>\n"));

			f.WriteString(_T("</Document>\n</kml>\n"));
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
