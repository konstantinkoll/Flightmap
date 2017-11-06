
// CMapFactory.h: Schnittstelle der Klasse CMapFactory
//

#pragma once
#include "CKitchen.h"
#include "Flightmap.h"


// CMapFactory
//

#define BGWIDTH        8192
#define BGHEIGHT       4096

class CMapFactory
{
public:
	CMapFactory(MapSettings* pSettings);

	CBitmap* RenderMap(CKitchen* pKitchen);

protected:
	static void AppendLabel(CString& strNote, UINT nID, UINT MaxLines);
	CBitmap* LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const;
	COLORREF PreparePen(Pen& pen, const FlightRoute& Route, const CKitchen* pKitchen, DOUBLE GfxScale) const;
	void DrawLine(CDC& dcMask, Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale, INT MinS, INT MinZ, REAL* pLabelX=NULL, REAL* pLabelY=NULL) const;
	static void DrawArrow(Graphics& g, Brush& brush, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale, REAL GfxScale, INT MinS, INT MinZ);
	static REAL MapX(REAL X);
	static REAL MapX(DOUBLE X);
	static REAL MapY(REAL Y);
	static REAL MapY(DOUBLE Y);
	static INT ScanMask(const CDC& dc, CRect& rect, UINT Border, INT Width, INT Height);

	MapSettings m_Settings;

private:
	void DrawLine(CDC& dcMask, Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale) const;
};

inline REAL CMapFactory::MapX(REAL X)
{
	return X*(BGWIDTH/2)/PI+BGWIDTH/2;
}

inline REAL CMapFactory::MapX(DOUBLE X)
{
	return MapX((REAL)X);
}

inline REAL CMapFactory::MapY(REAL Y)
{
	return Y*BGHEIGHT/PI+BGHEIGHT/2;
}

inline REAL CMapFactory::MapY(DOUBLE Y)
{
	return MapY((REAL)Y);
}
