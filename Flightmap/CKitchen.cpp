
// CKitchen.cpp: Implementierung der Klasse CKitchen
//

#include "stdafx.h"
#include "CKitchen.h"
#include <math.h>


// CKitchen
//

template <typename T>
FMDynArray<T, 128, 128>* ConvertToList(CMap<CStringA, LPCSTR, T, const T&>& Map)
{
	FMDynArray<T, 128, 128>* pDynArray = new FMDynArray<T, 128, 128>((UINT)Map.GetCount());

	for (const CMap<CStringA, LPCSTR, T, const T&>::CPair* pPair = Map.PGetFirstAssoc(); pPair; pPair = Map.PGetNextAssoc(pPair))
		pDynArray->AddItem(pPair->value);

	Map.RemoveAll();

	return pDynArray;
}

CKitchen::CKitchen(CItinerary* pItinerary, BOOL MergeMetro)
{
	if (pItinerary)
		m_DisplayName = pItinerary->m_DisplayName;

	m_AirportMap.InitHashTable(2048);
	m_RouteMap.InitHashTable(4096);
	m_MaxRouteCount = m_MinRouteCount = m_WaypointCount = 0;
	m_MergeMetro = MergeMetro;

	m_pAirportList = NULL;
	m_pRouteList = NULL;
}

CKitchen::~CKitchen()
{
	delete m_pAirportList;

	if (m_pRouteList)
	{
		for (UINT a=0; a<m_pRouteList->m_ItemCount; a++)
			delete (*m_pRouteList)[a].pSegments;

		delete m_pRouteList;
	}
}

LPCAIRPORT CKitchen::AddAirport(const LPCSTR Code)
{
	ASSERT(Code);

	if (strlen(Code)!=3)
		return NULL;

	LPCAIRPORT lpcAirport;
	if (!FMIATAGetAirportByCode(Code, lpcAirport))
		return NULL;

	if (m_MergeMetro && (lpcAirport->MetroCode[0]!='\0'))
		if (strcmp(lpcAirport->Code, lpcAirport->MetroCode)!=0)
			VERIFY(FMIATAGetAirportByCode(lpcAirport->MetroCode, lpcAirport));

	FlightAirport Airport;
	if (m_AirportMap.Lookup(lpcAirport->Code, Airport))
	{
		Airport.FlightCount++;
	}
	else
	{
		Airport.lpcAirport = lpcAirport;
		Airport.FlightCount = 1;
		Airport.X = Airport.Y = 0.0f;
	}

	m_AirportMap[lpcAirport->Code] = Airport;

	return lpcAirport;
}

void CKitchen::AddFlight(const AIRX_Flight& Flight, AIRX_Attachment* pGPSPath)
{
	LPCAIRPORT pFrom = AddAirport(Flight.From.Code);
	LPCAIRPORT pTo = AddAirport(Flight.To.Code);

	if (pFrom && pTo)
	{
		BYTE Arrow = ARROW_FT;

		if (pFrom>pTo)
		{
			LPCAIRPORT Temp = pFrom;
			pFrom = pTo;
			pTo = Temp;

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
		if (m_RouteMap.Lookup(ID, Route))
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

			if (!Route.CarrierMultiple && (Flight.Carrier[0]!=L'\0'))
				if (Route.Carrier[0]==L'\0')
				{
					wcscpy_s(Route.Carrier, 256, Flight.Carrier);
				}
				else
				{
					Route.CarrierMultiple = (_wcsicmp(Route.Carrier, Flight.Carrier)!=0);
				}

			if (!Route.EquipmentMultiple && (Flight.Equipment[0]!=L'\0'))
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
			Route.lpcFrom = pFrom;
			Route.lpcTo = pTo;
			Route.Waypoint = Flight.Waypoint;
			Route.Color = Flight.Color;
			Route.Count = 1;
			Route.Arrows = Arrow;
			Route.ptLabel.x = Route.ptLabel.y = -1.0f;
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

		m_RouteMap[ID] = Route;

		if (Route.Count>m_MaxRouteCount)
			m_MaxRouteCount = Route.Count;

		if ((Route.Count<m_MinRouteCount) || (m_MinRouteCount==0))
			m_MinRouteCount = Route.Count;
	}
}

INT CKitchen::CompareAirports(FlightAirport* pData1, FlightAirport* pData2, const SortParameters& /*Parameters*/)
{
	const INT Result = (INT)pData2->FlightCount-(INT)pData1->FlightCount;

	return Result ? Result : strcmp(pData1->lpcAirport->Code, pData2->lpcAirport->Code);
}

INT CKitchen::CompareRoutes(FlightRoute* pData1, FlightRoute* pData2, const SortParameters& /*Parameters*/)
{
	return (INT)pData2->Count-(INT)pData1->Count;
}

AirportList* CKitchen::GetAirports()
{
	if (!m_pAirportList)
		(m_pAirportList=ConvertToList<FlightAirport>(m_AirportMap))->SortItems((PFNCOMPARE)CompareAirports);

	return m_pAirportList;
}

RouteList* CKitchen::GetRoutes(BOOL Tesselate)
{
	if (!m_pRouteList)
	{
		(m_pRouteList=ConvertToList<FlightRoute>(m_RouteMap))->SortItems((PFNCOMPARE)CompareRoutes);

		if (Tesselate)
			for (UINT a=0; a<m_pRouteList->m_ItemCount; a++)
				TesselateRoute((*m_pRouteList)[a]);
	}

	return m_pRouteList;
}

FlightSegments* CKitchen::ParseGPX(CGPXFile* pGPXFile)
{
	if (!pGPXFile)
		return NULL;

	xml_node<>* pRootNode = pGPXFile->first_node("gpx");
	if (!pRootNode)
		return NULL;

	// Number of waypoints
	UINT PointCount = 0;

	for (xml_node<>* pTrkNode = pRootNode->first_node("trk"); pTrkNode; pTrkNode = pTrkNode->next_sibling())
		for (xml_node<>* pTrkSegNode = pTrkNode->first_node("trkseg"); pTrkSegNode; pTrkSegNode = pTrkSegNode->next_sibling())
			for (xml_node<>* pTrkPtNode = pTrkSegNode->first_node("trkpt"); pTrkPtNode; pTrkPtNode = pTrkPtNode->next_sibling())
				PointCount++;

	if (!PointCount)
		return NULL;

	// Allocate FlightSegments
	FlightSegments* pSegments = (FlightSegments*)malloc(sizeof(FlightSegments)+3*sizeof(DOUBLE)*(PointCount-1));
	pSegments->PointCount = PointCount;

	// Extract waypoints
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

void CKitchen::TesselateRoute(FlightRoute& Route)
{
	ASSERT(Route.lpcFrom);
	ASSERT(Route.lpcTo);

	const BOOL UseWaypoint = (Route.lpcFrom==Route.lpcTo) && ((Route.Waypoint.Latitude!=0) || (Route.Waypoint.Longitude!=0));

	if (!Route.GPSPathMultiple && Route.pGPSPath)
	{
		CGPXFile* pGPXFile = CItinerary::DecodeGPXAttachment(*Route.pGPSPath);
		if (pGPXFile)
		{
			Route.pSegments = ParseGPX(pGPXFile);
			delete pGPXFile;

			return;
		}
	}

	const DOUBLE Lat1 = PI*Route.lpcFrom->Location.Latitude/180.0;
	const DOUBLE Lon1 = PI*Route.lpcFrom->Location.Longitude/180.0;
	const DOUBLE Lat2 = PI*(UseWaypoint ? Route.Waypoint.Latitude : Route.lpcTo->Location.Latitude)/180.0;
	const DOUBLE Lon2 = PI*(UseWaypoint ? Route.Waypoint.Longitude : Route.lpcTo->Location.Longitude)/180.0;

	const DOUBLE D = 2*asin(sqrt(pow(sin((Lat1-Lat2)/2),2)+cos(Lat1)*cos(Lat2)*pow(sin((Lon1-Lon2)/2),2)));
	const UINT PointCount = (D<=0.1) ? 10 : (D<=0.5) ? 40 : 100;

	// Allocate FlightSegments
	Route.pSegments = (FlightSegments*)malloc(sizeof(FlightSegments)+3*sizeof(DOUBLE)*(PointCount-1));
	Route.pSegments->PointCount = PointCount;

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

		Route.pSegments->Points[a][0] = atan2(Z, sqrt(X*X+Y*Y));
		Route.pSegments->Points[a][1] = atan2(Y, X);
		Route.pSegments->Points[a][2] = MinH+Elevation*sin(ElevationEase*a/(PointCount-1));
	}
}
