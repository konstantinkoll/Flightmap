
// CKitchen.h: Schnittstelle der Klasse CKitchen
//

#pragma once
#include "CItinerary.h"
#include "FMDynArray.h"
#include <hash_map>


// CKitchen
//

#define ARROW_FT     1
#define ARROW_TF     2

typedef struct _RENDERPOINT
{
	REAL x;
	REAL y;

	friend BOOL operator==(const _RENDERPOINT& ptA, const _RENDERPOINT& ptB) { return (ptA.x==ptB.x) && (ptA.y==ptB.y); }
	friend BOOL operator!=(const _RENDERPOINT& ptA, const _RENDERPOINT& ptB) { return (ptA.x!=ptB.x) || (ptA.y!=ptB.y); }
} RENDERPOINT;

struct FlightAirport
{
	LPCAIRPORT lpcAirport;	// Airport
	UINT FlightCount;		// Number of movements
	union
	{
		struct
		{
			REAL X;
			REAL Y;
		};

		RENDERPOINT Point;
	};
};

struct FlightSegments
{
	UINT PointCount;
	DOUBLE Points[1][3];
};

struct FlightRoute
{
	LPCAIRPORT lpcFrom;		// Airport
	LPCAIRPORT lpcTo;		// Airport
	FMGeoCoordinates Waypoint;
	COLORREF Color;
	UINT Count;
	BYTE Arrows;

	// Annotations
	RENDERPOINT ptLabel;
	DOUBLE DistanceNM;
	ULONG FlightTime;
	UINT FlightTimeCount;
	WCHAR Carrier[256];
	BOOL CarrierMultiple;
	WCHAR Equipment[256];
	BOOL EquipmentMultiple;
	AIRX_Attachment* pGPSPath;
	BOOL GPSPathMultiple;

	// Segments
	FlightSegments* pSegments;
};

typedef FMDynArray<FlightAirport, 128, 128> AirportList;
typedef FMDynArray<FlightRoute, 128, 128> RouteList;
typedef CMap<CStringA, LPCSTR, FlightAirport, const FlightAirport&> AirportMap;
typedef CMap<CStringA, LPCSTR, FlightRoute, const FlightRoute&> RouteMap;

class CKitchen
{
public:
	CKitchen(CItinerary* pItinerary, BOOL MergeMetro=FALSE);
	~CKitchen();

	LPCWSTR GetDisplayName() const;
	void InsertDisplayName(CString& Caption) const;
	void AddFlight(const AIRX_Flight& Flight, AIRX_Attachment* pGPSPath);
	AirportList* GetAirports();
	RouteList* GetRoutes(BOOL Tesselate=TRUE);

	UINT m_MaxRouteCount;
	UINT m_MinRouteCount;

protected:
	BOOL m_MergeMetro;
	UINT m_WaypointCount;

private:
	LPCAIRPORT AddAirport(const LPCSTR Code);
	static INT __stdcall CompareAirports(FlightAirport* pData1, FlightAirport* pData2, const SortParameters& Parameters);
	static INT __stdcall CompareRoutes(FlightRoute* pData1, FlightRoute* pData2, const SortParameters& Parameters);
	static FlightSegments* ParseGPX(CGPXFile* pGPXFile);
	static void TesselateRoute(FlightRoute& Route);

	CString m_DisplayName;
	AirportMap m_AirportMap;
	RouteMap m_RouteMap;

	AirportList* m_pAirportList;
	RouteList* m_pRouteList;
};

inline LPCWSTR CKitchen::GetDisplayName() const
{
	return m_DisplayName;
}

inline void CKitchen::InsertDisplayName(CString& Caption) const
{
	if (!m_DisplayName.IsEmpty())
		Caption.Insert(0, m_DisplayName+_T(" - "));
}
