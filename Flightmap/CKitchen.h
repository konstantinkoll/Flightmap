
// CKitchen.h: Schnittstelle der Klasse CKitchen
//

#pragma once
#include "Flightmap.h"
#include "FMCommDlg.h"
#include "CItinerary.h"
#include <hash_map>
#include <math.h>


// CKitchen
//

#define ARROW_FT     1
#define ARROW_TF     2

struct FlightAirport
{
	FMAirport* pAirport;
	LPVOID lpAirport;
};

struct FlightRoute
{
	FMAirport* pFrom;
	FMAirport* pTo;
	LPVOID lpFrom;
	LPVOID lpTo;
	COLORREF Color;
	UINT Count;
	BYTE Arrows;
};

struct FlightSegments
{
	FlightRoute Route;
	UINT PointCount;
	DOUBLE Points[1][3];
};

typedef CMap<CStringA, LPCSTR, FlightAirport, FlightAirport> CFlightAirports;
typedef CMap<CStringA, LPCSTR, UINT, UINT> CFlightCounts;
typedef CMap<CStringA, LPCSTR, FlightRoute, FlightRoute&> CFlightRoutes;

class CKitchen
{
public:
	CKitchen(CString DisplayName, BOOL MergeMetro=FALSE);

	virtual void AddFlight(CHAR* From, CHAR* To, COLORREF Color);

	void AddFlight(AIRX_Flight& Flight);
	static FlightSegments* Tesselate(FlightRoute& Route);

	CString m_DisplayName;
	CFlightAirports m_FlightAirports;
	CFlightCounts m_FlightAirportCounts;
	CFlightRoutes m_FlightRoutes;
	UINT m_MaxRouteCount;

protected:
	BOOL m_MergeMetro;

private:
	FMAirport* AddAirport(CHAR* Code);
};
