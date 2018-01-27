
// CGoogleEarthFile.cpp: Implementierung der Klasse CGoogleEarthFile
//

#include "stdafx.h"
#include "CGoogleEarthFile.h"
#include <io.h>


// CGoogleEarthFile
//

CGoogleEarthFile::CGoogleEarthFile()
	: CStdioFile()
{
	m_IsOpen = FALSE;
}

BOOL CGoogleEarthFile::Open(LPCTSTR lpszPath, LPCTSTR lpszDisplayName)
{
	if (m_IsOpen)
		return FALSE;

	if (_tfopen_s(&m_pStream, lpszPath, _T("wt,ccs=UTF-8")))
		return FALSE;

	m_hFile = (HANDLE)_get_osfhandle(_fileno(m_pStream));
	m_IsOpen = TRUE;

	WriteString(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml xmlns=\"http://earth.google.com/kml/2.0\">\n<Document>\n"));

	if (lpszDisplayName)
	{
		WriteString(_T("<name>"));
		WriteString(lpszDisplayName);
		WriteString(_T("</name>\n"));
	}

	WriteString(_T("<open>1</open>\n"));
	WriteString(_T("<Style id=\"A\"><IconStyle><scale>0.8</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle></Style>\n"));
	WriteString(_T("<Style id=\"B\"><IconStyle><scale>1.0</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>1</scale></LabelStyle></Style>\n"));
	WriteString(_T("<StyleMap id=\"Location\"><Pair><key>normal</key><styleUrl>#A</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#B</styleUrl></Pair></StyleMap>\n"));

	return m_IsOpen;
}

void CGoogleEarthFile::WriteAirport(LPCAIRPORT lpcAirport)
{
	WriteString(_T("<Placemark>\n<name>"));

	CString tmpStr(lpcAirport->Code);
	WriteString(tmpStr);

	WriteString(_T("</name>\n<description>"));
	WriteAttribute(IDS_AIRPORT_NAME, lpcAirport->Name);
	WriteAttribute(IDS_AIRPORT_COUNTRY, FMIATAGetCountry(lpcAirport->CountryID)->Name);
	WriteAttribute(IDS_AIRPORT_CODE, tmpStr);
	WriteAttribute(IDS_AIRPORT_LOCATION, FMGeoCoordinatesToString(lpcAirport->Location));
	WriteString(_T("&lt;div&gt;</description>\n"));

	WriteString(_T("<styleUrl>#Location</styleUrl>\n"));
	tmpStr.Format(_T("<Point><coordinates>%.6lf,%.6lf,-5000</coordinates></Point>\n"), lpcAirport->Location.Longitude, -lpcAirport->Location.Latitude);
	WriteString(tmpStr);
	WriteString(_T("</Placemark>\n"));
}

void CGoogleEarthFile::WriteRoute(const FlightRoute& Route, UINT MinRouteCount, UINT MaxRouteCount, BOOL UseCount, BOOL UseColors, BOOL ClampHeight)
{
	WriteString(_T("<Folder>\n<name>"));

	CHAR tmpBuf[256];
	sprintf_s(tmpBuf, 256, "%s&#8211;%s", Route.lpcFrom->Code, Route.lpcTo->Code);

	WriteString(CString(tmpBuf));

	WriteString(_T("</name>\n<description>"));

	sprintf_s(tmpBuf, 256, "%s &#8211; %s", Route.lpcFrom->Name, Route.lpcTo->Name);
	WriteString(CString(tmpBuf));

	WriteString(_T("</description>\n<Placemark>\n<Style><LineStyle><color>ff"));

	const COLORREF Color = UseColors ? Route.Color : 0xFFFFFF;

	CString tmpStr;
	tmpStr.Format(_T("%02X%02X%02X"), (Color>>16) & 0xFF, (Color>>8) & 0xFF, Color & 0xFF);

	WriteString(tmpStr);

	const DOUBLE Width = (UseCount && (MaxRouteCount!=0)) ? (0.2+(3.0*((DOUBLE)(Route.Count-MinRouteCount))/((DOUBLE)(MaxRouteCount-MinRouteCount+1)))) : 3.2;
	
	tmpStr.Format(_T("</color><width>%.1lf</width></LineStyle></Style>\n"), Width);
	WriteString(tmpStr);

	WriteString(_T("<LineString>\n<tessellate>0</tessellate>\n<altitudeMode>absolute</altitudeMode>\n<coordinates>"));

	ASSERT(Route.pSegments);
	DOUBLE* pPoints = &Route.pSegments->Points[0][0];
	for (UINT a=0; a<Route.pSegments->PointCount; a++)
	{
		const DOUBLE H = ClampHeight ? min(pPoints[2], 1.01) : pPoints[2];

		tmpStr.Format(_T("%.6lf,%.6lf,%.6lf "), 180*pPoints[1]/PI, -180*pPoints[0]/PI, 2500000*(H-1.0095));
		WriteString(tmpStr);

		pPoints += 3;
	}

	WriteString(_T("</coordinates>\n</LineString>\n</Placemark>\n</Folder>\n"));
}

void CGoogleEarthFile::WriteRoutes(CKitchen* pKitchen, BOOL UseCount, BOOL UseColors, BOOL ClampHeight)
{
	const RouteList* pRouteList = pKitchen->GetRoutes();

	for (UINT a=0; a<pRouteList->m_ItemCount; a++)
		WriteRoute((*pRouteList)[a], pKitchen->m_MinRouteCount, pKitchen->m_MaxRouteCount, UseCount, UseColors, ClampHeight);
}

void CGoogleEarthFile::WriteAirports(CKitchen* pKitchen)
{
	AirportList* pAirportList = pKitchen->GetAirports();

	for (UINT a=0; a<pAirportList->m_ItemCount; a++)
		WriteAirport((*pAirportList)[a].lpcAirport);
}

void CGoogleEarthFile::Close()
{
	if (m_IsOpen)
	{
		WriteString(_T("</Document>\n</kml>\n"));

		CStdioFile::Close();

		m_IsOpen = FALSE;
	}
}

void CGoogleEarthFile::WriteAttribute(UINT ResID, const CString& Value)
{
	if (!Value.IsEmpty())
	{
		WriteString(_T("&lt;b&gt;"));
		WriteString(CString((LPCSTR)ResID));
		WriteString(_T("&lt;/b&gt;: "));
		WriteString(Value);
		WriteString(_T("&lt;br&gt;"));
	}
}

void CGoogleEarthFile::WriteAttribute(UINT ResID, LPCSTR pValue)
{
	WriteAttribute(ResID, CString(pValue));
}
