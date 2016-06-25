
// CKitchen.h: Schnittstelle der Klasse CKitchen
//

#pragma once
#include "CItinerary.h"
#include <hash_map>


// CKitchen
//

#define ARROW_FT     1
#define ARROW_TF     2

struct FlightAirport
{
	FMAirport* pAirport;	// Airport
	LPVOID lpAirport;		// Additonal data, e.g. from map factory
};

struct FlightRoute
{
	FMAirport* pFrom;		// Airport
	FMAirport* pTo;			// Airport
	LPVOID lpFrom;			// Additonal data, e.g. from map factory
	LPVOID lpTo;			// Additonal data, e.g. from map factory
	FMGeoCoordinates Waypoint;
	COLORREF Color;
	UINT Count;
	BYTE Arrows;

	// Annotations
	REAL LabelZ;
	REAL LabelS;
	DOUBLE DistanceNM;
	ULONG FlightTime;
	UINT FlightTimeCount;
	WCHAR Carrier[256];
	BOOL CarrierMultiple;
	WCHAR Equipment[256];
	BOOL EquipmentMultiple;
	AIRX_Attachment* pGPSPath;
	BOOL GPSPathMultiple;
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
	CKitchen(const CString& DisplayName, BOOL MergeMetro=FALSE);

	void AddFlight(const AIRX_Flight& Flight, AIRX_Attachment* pGPSPath);
	static FlightSegments* Tesselate(FlightRoute& Route);

	CString m_DisplayName;
	CFlightAirports m_FlightAirports;
	CFlightCounts m_FlightAirportCounts;
	CFlightRoutes m_FlightRoutes;
	UINT m_MaxRouteCount;
	UINT m_MinRouteCount;

protected:
	BOOL m_MergeMetro;
	UINT m_WaypointCount;

private:
	FMAirport* AddAirport(const CHAR* Code);
	static FlightSegments* ParseGPX(FlightRoute& Route, CGPXFile* pGPXFile);
};
