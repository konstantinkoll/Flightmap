
// CGoogleEarthFile.cpp: Implementierung der Klasse CGoogleEarthFile
//

#pragma once
#include "stdafx.h"
#include "CGoogleEarthFile.h"


void CookAttributeString(CString& tmpStr)
{
	tmpStr.Replace(_T("<"), _T("_"));
	tmpStr.Replace(_T(">"), _T("_"));
	tmpStr.Replace(_T("&"), _T("&amp;"));
	tmpStr.Replace(_T("�"), _T("&#8211;"));
	tmpStr.Replace(_T("�"), _T("&#8212;"));
}


// CGoogleEarthFile
//

CGoogleEarthFile::CGoogleEarthFile()
{
	m_IsOpen = FALSE;
}

BOOL CGoogleEarthFile::Open(LPCTSTR lpszFileName, LPCTSTR lpszDisplayName)
{
	if (m_IsOpen)
		return FALSE;

	m_IsOpen = CStdioFile::Open(lpszFileName, CFile::modeCreate | CFile::modeWrite);
	if (m_IsOpen)
	{
		WriteString(_T("<?xml version=\"1.0\"?>\n<kml xmlns=\"http://earth.google.com/kml/2.0\">\n<Document>\n"));

		if (lpszDisplayName)
		{
			WriteString(_T("<name>"));
			WriteString(lpszDisplayName);
			WriteString(_T("</name>\n"));
		}

		WriteString(_T("<open>1</open>\n"));
		WriteString(_T("<Style id=\"A\"><IconStyle><scale>0.8</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle></Style>\n"));
		WriteString(_T("<Style id=\"B\"><IconStyle><scale>1.0</scale><Icon><href>http://maps.google.com/mapfiles/kml/pal4/icon57.png</href></Icon></IconStyle><LabelStyle><scale>1</scale></LabelStyle></Style>\n"));
		WriteString(_T("<StyleMap id=\"C\"><Pair><key>normal</key><styleUrl>#A</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#B</styleUrl></Pair></StyleMap>\n"));
	}

	return m_IsOpen;
}

void CGoogleEarthFile::WriteAirport(FMAirport* pAirport)
{
	WriteString(_T("<Placemark>\n<name>"));

	CString tmpStr(pAirport->Code);
	CookAttributeString(tmpStr);
	WriteString(tmpStr);

	WriteString(_T("</name>\n<description>"));
	WriteAttribute(IDS_AIRPORT_NAME, pAirport->Name);
	WriteAttribute(IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
	WriteAttribute(IDS_AIRPORT_CODE, tmpStr);
	FMGeoCoordinatesToString(pAirport->Location, tmpStr);
	WriteAttribute(IDS_AIRPORT_LOCATION, tmpStr);
	WriteString(_T("&lt;div&gt;</description>\n"));

	WriteString(_T("<styleUrl>#C</styleUrl>\n"));
	tmpStr.Format(_T("<Point><coordinates>%.6lf,%.6lf,-5000</coordinates></Point>\n"), pAirport->Location.Longitude, -pAirport->Location.Latitude);
	WriteString(tmpStr);
	WriteString(_T("</Placemark>\n"));
}

void CGoogleEarthFile::WriteRoute(FlightSegments* pSegments, BOOL UseColors, BOOL Clamp, BOOL FreeSegments)
{
	CString tmpStr;
	CHAR tmpBuf[256];

	WriteString(_T("<Folder>\n<name>"));

	sprintf_s(tmpBuf, 256, "%s&#8211;%s", pSegments->Route.pFrom->Code, pSegments->Route.pTo->Code);
	tmpStr = tmpBuf;
	WriteString(tmpStr);

	WriteString(_T("</name>\n<description>"));

	sprintf_s(tmpBuf, 256, "%s &#8212; %s", pSegments->Route.pFrom->Name, pSegments->Route.pTo->Name);
	tmpStr = tmpBuf;
	WriteString(tmpStr);

	WriteString(_T("</description>\n<Placemark>\n<Style><LineStyle><color>ff"));

	const COLORREF Color = UseColors ? pSegments->Route.Color : 0xFFFFFF;
	tmpStr.Format(_T("%02X%02X%02X"), (Color>>16) & 0xFF, (Color>>8) & 0xFF, Color & 0xFF);
	WriteString(tmpStr);

	WriteString(_T("</color><width>2.5</width></LineStyle></Style>\n"));
	WriteString(_T("<LineString>\n"));
	WriteString(_T("<tessellate>0</tessellate>\n"));
	WriteString(_T("<altitudeMode>absolute</altitudeMode>\n"));
	WriteString(_T("<coordinates>"));

	DOUBLE* pPoints = &pSegments->Points[0][0];
	for (UINT a=0; a<pSegments->PointCount; a++)
	{
		const DOUBLE H = Clamp ? min(pPoints[2], 1.01) : pPoints[2];
		tmpStr.Format(_T("%.6lf,%.6lf,%.6lf "), 180*pPoints[1]/PI, -180*pPoints[0]/PI, 2500000*(H-1.0095));
		WriteString(tmpStr);
		pPoints += 3;
	}

	WriteString(_T("</coordinates>\n</LineString>\n</Placemark>\n"));

	WriteString(_T("</Folder>\n"));

	if (FreeSegments)
		free(pSegments);
}

void CGoogleEarthFile::WriteRoutes(CKitchen* pKitchen, BOOL UseColors, BOOL Clamp, BOOL FreeKitchen)
{
	CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc();
	while (pPair)
	{
		WriteRoute(pKitchen->Tesselate(pPair->value), UseColors, Clamp);

		pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair);
	}

	if (FreeKitchen)
		delete pKitchen;
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

void CGoogleEarthFile::WriteAttribute(UINT ResID, CString Value)
{
	if (!Value.IsEmpty())
	{
		CString Name;
		ENSURE(Name.LoadString(ResID));
		CookAttributeString(Name);
		CookAttributeString(Value);

		WriteString(_T("&lt;b&gt;"));
		WriteString(Name);
		WriteString(_T("&lt;/b&gt;: "));
		WriteString(Value);
		WriteString(_T("&lt;br&gt;"));
	}
}

void CGoogleEarthFile::WriteAttribute(UINT ResID, CHAR* Value)
{
	CString tmpStr(Value);
	WriteAttribute(ResID, tmpStr);
}