
// CGoogleEarthFile.cpp: Implementierung der Klasse CGoogleEarthFile
//

#pragma once
#include "stdafx.h"
#include "CGoogleEarthFile.h"


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
	}

	return m_IsOpen;
}

void CGoogleEarthFile::WriteRoute(FlightSegments* pSegments, BOOL UseColors)
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
		tmpStr.Format(_T("%.6lf,%.6lf,%.6lf "), 180*pPoints[1]/PI, -180*pPoints[0]/PI, 2500000*(pPoints[2]-1.0095));
		WriteString(tmpStr);
		pPoints += 3;
	}

	WriteString(_T("</coordinates>\n</LineString>\n</Placemark>\n"));

	WriteString(_T("</Folder>\n"));

	free(pSegments);
}

void CGoogleEarthFile::WriteRoutes(CKitchen* pKitchen, BOOL UseColors)
{
	CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc();
	while (pPair)
	{
		WriteRoute(pKitchen->Tesselate(pPair->value), UseColors);

		pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair);
	}

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
