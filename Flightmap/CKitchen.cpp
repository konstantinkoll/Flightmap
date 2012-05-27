
// CKitchen.cpp: Implementierung der Klasse CKitchen
//

#pragma once
#include "stdafx.h"
#include "CKitchen.h"
#include <math.h>


// CKitchen
//

CKitchen::CKitchen(CString DisplayName, BOOL MergeMetro)
{
	m_DisplayName = DisplayName;
	m_MaxRouteCount = 0;
	m_MergeMetro = MergeMetro;

	m_FlightAirports.InitHashTable(2048);
	m_FlightAirportCounts.InitHashTable(2048);
	m_FlightRoutes.InitHashTable(4096);
}

FMAirport* CKitchen::AddAirport(CHAR* Code)
{
	ASSERT(Code);

	if (strlen(Code)!=3)
		return NULL;

	FMAirport* pAirport = NULL;
	if (!FMIATAGetAirportByCode(Code, &pAirport))
		return NULL;

	if ((m_MergeMetro) && (pAirport->MetroCode[0]!='\0'))
		if (strcmp(pAirport->Code, pAirport->MetroCode)!=0)
			FMIATAGetAirportByCode(pAirport->MetroCode, &pAirport);

	FlightAirport Airport;
	if (m_FlightAirports.Lookup(pAirport->Code, Airport))
	{
		m_FlightAirportCounts[pAirport->Code]++;
		return Airport.pAirport;
	}

	ZeroMemory(&Airport, sizeof(Airport));
	Airport.pAirport = pAirport;

	m_FlightAirports[pAirport->Code] = Airport;
	m_FlightAirportCounts[pAirport->Code] = 1;

	return pAirport;
}

void CKitchen::AddFlight(CHAR* From, CHAR* To, COLORREF Color)
{
	FMAirport* pFrom = AddAirport(From);
	FMAirport* pTo = AddAirport(To);
	BYTE Arrow = ARROW_FT;

	if ((pFrom!=NULL) && (pTo!=NULL))
	{
		if (pFrom>pTo)
		{
			std::swap(pFrom, pTo);
			Arrow <<= 1;
		}

		CHAR ID[7];
		strcpy_s(ID, 7, pFrom->Code);
		strcat_s(ID, 7, pTo->Code);

		FlightRoute Route;
		if (m_FlightRoutes.Lookup(ID, Route))
		{
			Route.Count++;
			Route.Arrows |= Arrow;
			if (Color!=Route.Color)
				Route.Color = (COLORREF)-1;
		}
		else
		{
			ZeroMemory(&Route, sizeof(Route));
			Route.pFrom = pFrom;
			Route.pTo = pTo;
			Route.Count = 1;
			Route.Color = Color;
			Route.Arrows = Arrow;
		}

		m_FlightRoutes[ID] = Route;

		if (Route.Count>m_MaxRouteCount)
			m_MaxRouteCount = Route.Count;
	}
}

void CKitchen::AddFlight(AIRX_Flight& Flight)
{
	AddFlight(Flight.From.Code, Flight.To.Code, Flight.Color);
}

FlightSegments* CKitchen::Tesselate(FlightRoute& Route)
{
	ASSERT(Route.pFrom);
	ASSERT(Route.pTo);

	DOUBLE Lat1 = (Route.pFrom->Location.Latitude*PI)/180;
	DOUBLE Lon1 = (Route.pFrom->Location.Longitude*PI)/180;
	DOUBLE Lat2 = (Route.pTo->Location.Latitude*PI)/180;
	DOUBLE Lon2 = (Route.pTo->Location.Longitude*PI)/180;

	DOUBLE D = 2*asin(sqrt(pow(sin((Lat1-Lat2)/2),2)+cos(Lat1)*cos(Lat2)*pow(sin((Lon1-Lon2)/2),2)));
	UINT PointCount = (D<=0.1) ? 10 : (D<=0.5) ? 40 : 100;

	FlightSegments* pSegments = (FlightSegments*)malloc(sizeof(FlightSegments)+3*sizeof(DOUBLE)*(PointCount-1));
	pSegments->Route = Route;
	pSegments->PointCount = PointCount;

	DOUBLE MinH = 1.01;
	DOUBLE Elevation = min(D/2, 0.25);

	for (UINT a=0; a<PointCount; a++)
	{
		DOUBLE V = sin((PointCount-a-1)*D/(PointCount-1))/sin(D);
		DOUBLE W = sin(a*D/(PointCount-1))/sin(D);

		DOUBLE X = V*cos(Lat1)*cos(Lon1)+W*cos(Lat2)*cos(Lon2);
		DOUBLE Y = V*cos(Lat1)*sin(Lon1)+W*cos(Lat2)*sin(Lon2);
		DOUBLE Z = V*sin(Lat1)+W*sin(Lat2);

		pSegments->Points[a][0] = atan2(Z, sqrt(X*X+Y*Y));
		pSegments->Points[a][1] = atan2(Y, X);
		pSegments->Points[a][2] = MinH+Elevation*sin(PI*a/(PointCount-1));
	}

	return pSegments;
}
