
// CMapFactory.h: Schnittstelle der Klasse CMapFactory
//

#pragma once
#include "CKitchen.h"
#include "Flightmap.h"


// CMapFactory
//

#define BGWIDTH      8192
#define BGHEIGHT     4096

typedef struct _BACKGROUNDPOINT : RENDERPOINT
{
	_BACKGROUNDPOINT() {}

	_BACKGROUNDPOINT(LPCAIRPORT lpcAirport, INT MapOffset)
	{
		if ((x=(REAL)(lpcAirport->Location.Longitude*BGWIDTH)/360.0f+(BGWIDTH/2+MapOffset))>=BGWIDTH)
			x -= BGWIDTH;

		y = (REAL)(lpcAirport->Location.Latitude*BGHEIGHT)/180.0f+BGHEIGHT/2;
	}
} BACKGROUNDPOINT;

typedef struct _MAPPOINT : BACKGROUNDPOINT
{
	_MAPPOINT() {}
	_MAPPOINT(const BACKGROUNDPOINT& ptBackground, INT MinX, INT MinY) { x = ptBackground.x-MinX; y = ptBackground.y-MinY; }
	_MAPPOINT(const DOUBLE* Point, INT MinX, INT MinY) { x = (REAL)(Point[1]*(BGWIDTH/2)/PI+BGWIDTH/2)-MinX; y = (REAL)(Point[0]*BGHEIGHT/PI+BGHEIGHT/2)-MinY; }
} MAPPOINT;

typedef struct _DRAWPOINT : MAPPOINT
{
	_DRAWPOINT() {}
	_DRAWPOINT(const MAPPOINT& ptMap, REAL Scale) { x = ptMap.x*Scale; y = ptMap.y*Scale; }
	_DRAWPOINT(const MAPPOINT& ptMap, REAL OffsetX, REAL Scale) { x = (ptMap.x+OffsetX)*Scale; y = ptMap.y*Scale; }
} DRAWPOINT;

typedef struct _PIXELPOINT : POINT
{
	_PIXELPOINT() {}
	_PIXELPOINT(const DRAWPOINT& ptDraw) { x = (INT)(ptDraw.x+0.5f); y = (INT)(ptDraw.y+0.5f); }
	_PIXELPOINT(const DRAWPOINT& ptDraw, REAL Offset) { x = (INT)(ptDraw.x+Offset+0.5f); y = (INT)(ptDraw.y+Offset+0.5f); }
} PIXELPOINT;

class CMapFactory sealed
{
public:
	CMapFactory(const MapSettings& Settings);

	CBitmap* RenderMap(CKitchen* pKitchen);

private:
	static void AppendLabel(CString& strNote, UINT nID, UINT MaxLines);
	CBitmap* LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const;
	COLORREF PreparePen(Pen& pen, const FlightRoute& Route, const CKitchen* pKitchen, DOUBLE GfxScale) const;
	void DrawLine(CDC& dcMask, Graphics& g, const Pen& pen, const DRAWPOINT& ptA, const DRAWPOINT& ptB) const;
	void DrawLine(CDC& dcMask, Graphics& g, const Pen& pen, const MAPPOINT& ptA, const MAPPOINT& ptB, REAL Scale, RENDERPOINT* pptLabel=NULL) const;
	static void DrawArrow(Graphics& g, const Brush& brush, const DRAWPOINT& ptA, const DRAWPOINT& ptB, REAL GfxScale);
	static INT ScanMask(const CDC& dc, CRect& rect, UINT Border, INT Width, INT Height);

	MapSettings m_Settings;
};
