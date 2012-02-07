
// CKitchen.h: Schnittstelle der Klasse CKitchen
//

#pragma once
#include "Flightmap.h"
#include "FMCommDlg.h"
#include <hash_map>
#include <math.h>


// CKitchen
//

struct FlightRoute
{
	FMAirport* pFrom;
	FMAirport* pTo;
	COLORREF Color;
	INT Count;
};

struct FlightSegments
{
	FlightRoute Route;
	UINT PointCount;
	DOUBLE Points[1][3];
};

typedef CMap<CStringA, LPCSTR, FMAirport*, FMAirport*&> CFlightAirports;
typedef CMap<CStringA, LPCSTR, UINT, UINT> CFlightCounts;
typedef CMap<CStringA, LPCSTR, FlightRoute, FlightRoute&> CFlightRoutes;

class CKitchen
{
public:
	CKitchen();
	~CKitchen();

	virtual void AddFlight(CHAR* From, CHAR* To, COLORREF Color);

	static FlightSegments* Tesselate(FlightRoute& Route);

	CFlightAirports m_FlightAirports;
	CFlightCounts m_FlightAirportCounts;
	CFlightRoutes m_FlightRoutes;

private:
	FMAirport* AddAirport(CHAR* Code);
};