
// CKitchen.cpp: Implementierung der Klasse CKitchen
//

#include "stdafx.h"
#include "CKitchen.h"
#include <math.h>


// CKitchen
//

CKitchen::CKitchen(const CString& DisplayName, BOOL MergeMetro)
{
	m_DisplayName = DisplayName;
	m_MaxRouteCount = m_MinRouteCount = m_WaypointCount = 0;
	m_MergeMetro = MergeMetro;

	m_FlightAirports.InitHashTable(2048);
	m_FlightAirportCounts.InitHashTable(2048);
	m_FlightRoutes.InitHashTable(4096);
}

FMAirport* CKitchen::AddAirport(const LPCSTR Code)
{
	ASSERT(Code);

	if (strlen(Code)!=3)
		return NULL;

	FMAirport* pAirport;
	if (!FMIATAGetAirportByCode(Code, pAirport))
		return NULL;

	if ((m_MergeMetro) && (pAirport->MetroCode[0]!='\0'))
		if (strcmp(pAirport->Code, pAirport->MetroCode)!=0)
			FMIATAGetAirportByCode(pAirport->MetroCode, pAirport);

	FlightAirport Airport;
	if (m_FlightAirports.Lookup(pAirport->Code, Airport))
	{
		m_FlightAirportCounts[pAirport->Code]++;
	}
	else
	{
		ZeroMemory(&Airport, sizeof(Airport));
		Airport.pAirport = pAirport;

		m_FlightAirports[pAirport->Code] = Airport;
		m_FlightAirportCounts[pAirport->Code] = 1;
	}

	return pAirport;
}

void CKitchen::AddFlight(const AIRX_Flight& Flight, AIRX_Attachment* pGPSPath)
{
	FMAirport* pFrom = AddAirport(Flight.From.Code);
	FMAirport* pTo = AddAirport(Flight.To.Code);
	BYTE Arrow = ARROW_FT;

	if (pFrom && pTo)
	{
		if (pFrom>pTo)
		{
			std::swap(pFrom, pTo);
			Arrow <<= 1;
		}

		CHAR ID[16];
		if ((pFrom==pTo) && ((Flight.Waypoint.Latitude!=0.0) || (Flight.Waypoint.Longitude!=0.0)))
		{
			sprintf_s(ID, 16, "%s%012d", pFrom->Code, m_WaypointCount++);
		}
		else
		{
			strcpy_s(ID, 7, pFrom->Code);
			strcat_s(ID, 7, pTo->Code);
		}

		FlightRoute Route;
		if (m_FlightRoutes.Lookup(ID, Route))
		{
			Route.Count++;
			Route.Arrows |= Arrow;

			if (Flight.Color!=Route.Color)
				Route.Color = (COLORREF)-1;

			if (Flight.FlightTime)
			{
				Route.FlightTime += Flight.FlightTime;
				Route.FlightTimeCount++;
			}

			if ((!Route.CarrierMultiple) && (Flight.Carrier[0]!=L'\0'))
				if (Route.Carrier[0]==L'\0')
				{
					wcscpy_s(Route.Carrier, 256, Flight.Carrier);
				}
				else
				{
					Route.CarrierMultiple = (_wcsicmp(Route.Carrier, Flight.Carrier)!=0);
				}

			if ((!Route.EquipmentMultiple) && (Flight.Equipment[0]!=L'\0'))
				if (Route.Equipment[0]==L'\0')
				{
					wcscpy_s(Route.Equipment, 256, Flight.Equipment);
				}
				else
				{
					Route.EquipmentMultiple = (_wcsicmp(Route.Equipment, Flight.Equipment)!=0);
				}

			if (!Route.GPSPathMultiple && pGPSPath)
				if (!Route.pGPSPath)
				{
					Route.pGPSPath = pGPSPath;
				}
				else
				{
					Route.GPSPathMultiple = TRUE;
				}
		}
		else
		{
			ZeroMemory(&Route, sizeof(Route));
			Route.pFrom = pFrom;
			Route.pTo = pTo;
			Route.Waypoint = Flight.Waypoint;
			Route.Count = 1;
			Route.Color = Flight.Color;
			Route.Arrows = Arrow;
			Route.LabelX = Route.LabelY = -1.0f;
			Route.DistanceNM = 0.0;

			if (Flight.FlightTime)
			{
				Route.FlightTime = Flight.FlightTime;
				Route.FlightTimeCount = 1;
			}

			if (Flight.Carrier[0]!=L'\0')
				wcscpy_s(Route.Carrier, 256, Flight.Carrier);

			if (Flight.Equipment[0]!=L'\0')
				wcscpy_s(Route.Equipment, 256, Flight.Equipment);

			Route.pGPSPath = pGPSPath;
		}

		if ((Route.DistanceNM==0.0) && (Flight.Flags & AIRX_DistanceValid))
			Route.DistanceNM = Flight.DistanceNM;

		m_FlightRoutes[ID] = Route;

		if (Route.Count>m_MaxRouteCount)
			m_MaxRouteCount = Route.Count;

		if ((Route.Count<m_MinRouteCount) || (m_MinRouteCount==0))
			m_MinRouteCount = Route.Count;
	}
}

FlightSegments* CKitchen::ParseGPX(FlightRoute& Route, CGPXFile* pGPXFile)
{
	if (!pGPXFile)
		return NULL;

	xml_node<>* pRootNode = pGPXFile->first_node("gpx");
	if (!pRootNode)
		return NULL;

	// Anzahl Wegpunkte
	UINT PointCount = 0;

	for (xml_node<>* pTrkNode = pRootNode->first_node("trk"); pTrkNode; pTrkNode = pTrkNode->next_sibling())
		for (xml_node<>* pTrkSegNode = pTrkNode->first_node("trkseg"); pTrkSegNode; pTrkSegNode = pTrkSegNode->next_sibling())
			for (xml_node<>* pTrkPtNode = pTrkSegNode->first_node("trkpt"); pTrkPtNode; pTrkPtNode = pTrkPtNode->next_sibling())
				PointCount++;

	if (!PointCount)
		return NULL;

	// FlightSegments allokieren
	FlightSegments* pSegments = (FlightSegments*)malloc(sizeof(FlightSegments)+3*sizeof(DOUBLE)*(PointCount-1));
	pSegments->Route = Route;
	pSegments->PointCount = PointCount;

	// Punkte extrahieren
	UINT Ptr = 0;

	for (xml_node<>* pTrkNode = pRootNode->first_node("trk"); pTrkNode; pTrkNode = pTrkNode->next_sibling())
		for (xml_node<>* pTrkSegNode = pTrkNode->first_node("trkseg"); pTrkSegNode; pTrkSegNode = pTrkSegNode->next_sibling())
			for (xml_node<>* pTrkPtNode = pTrkSegNode->first_node("trkpt"); pTrkPtNode; pTrkPtNode = pTrkPtNode->next_sibling())
			{
				if (Ptr<PointCount)
				{
					xml_attribute<>* pAttributeLat = pTrkPtNode->first_attribute("lat");
					xml_attribute<>* pAttributeLon = pTrkPtNode->first_attribute("lon");

					if (pAttributeLat && pAttributeLon)
					{
						DOUBLE Latitude = 0.0;
						DOUBLE Longitude = 0.0;

						sscanf_s(pAttributeLat->value(), "%lf", &Latitude);
						sscanf_s(pAttributeLon->value(), "%lf", &Longitude);

						pSegments->Points[Ptr][0] = -PI*Latitude/180;
						pSegments->Points[Ptr][1] = PI*Longitude/180;
						pSegments->Points[Ptr][2] = 1.01;

						Ptr++;
					}
				}
			}

	pSegments->PointCount = Ptr;

	return pSegments;
}

FlightSegments* CKitchen::Tesselate(FlightRoute& Route)
{
	ASSERT(Route.pFrom);
	ASSERT(Route.pTo);

	const BOOL UseWaypoint = (Route.pFrom==Route.pTo) && ((Route.Waypoint.Latitude!=0) || (Route.Waypoint.Longitude!=0));

	if (!Route.GPSPathMultiple && Route.pGPSPath)
	{
		CGPXFile* pGPXFile = CItinerary::DecodeGPXAttachment(*Route.pGPSPath);
		if (pGPXFile)
		{
			FlightSegments* pSegments = ParseGPX(Route, pGPXFile);
			delete pGPXFile;

			if (pSegments)
				return pSegments;
		}
	}

	const DOUBLE Lat1 = PI*Route.pFrom->Location.Latitude/180;
	const DOUBLE Lon1 = PI*Route.pFrom->Location.Longitude/180;
	const DOUBLE Lat2 = PI*(UseWaypoint ? Route.Waypoint.Latitude : Route.pTo->Location.Latitude)/180;
	const DOUBLE Lon2 = PI*(UseWaypoint ? Route.Waypoint.Longitude : Route.pTo->Location.Longitude)/180;

	const DOUBLE D = 2*asin(sqrt(pow(sin((Lat1-Lat2)/2),2)+cos(Lat1)*cos(Lat2)*pow(sin((Lon1-Lon2)/2),2)));
	const UINT PointCount = (D<=0.1) ? 10 : (D<=0.5) ? 40 : 100;

	// FlightSegments allokieren
	FlightSegments* pSegments = (FlightSegments*)malloc(sizeof(FlightSegments)+3*sizeof(DOUBLE)*(PointCount-1));
	pSegments->Route = Route;
	pSegments->PointCount = PointCount;

	const DOUBLE MinH = 1.01;
	const DOUBLE Elevation = min(D/2, 0.25);
	const DOUBLE ElevationEase = UseWaypoint ? PI/2 : PI;

	const DOUBLE Matrix[] = { cos(Lat1)*cos(Lon1), cos(Lat2)*cos(Lon2), cos(Lat1)*sin(Lon1), cos(Lat2)*sin(Lon2), sin(Lat1), sin(Lat2) };

	for (UINT a=0; a<PointCount; a++)
	{
		const DOUBLE V = sin((PointCount-a-1)*D/(PointCount-1))/sin(D);
		const DOUBLE W = sin(a*D/(PointCount-1))/sin(D);

		const DOUBLE X = V*Matrix[0]+W*Matrix[1];
		const DOUBLE Y = V*Matrix[2]+W*Matrix[3];
		const DOUBLE Z = V*Matrix[4]+W*Matrix[5];

		pSegments->Points[a][0] = atan2(Z, sqrt(X*X+Y*Y));
		pSegments->Points[a][1] = atan2(Y, X);
		pSegments->Points[a][2] = MinH+Elevation*sin(ElevationEase*a/(PointCount-1));
	}

	return pSegments;
}
