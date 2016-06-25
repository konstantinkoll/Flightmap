
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
	static void AppendLabel(CString& Buf, UINT nID, UINT MaxLines);
	CBitmap* LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const;
	void DrawLine(Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, INT MinS=0, INT MinZ=0, REAL Scale=1.0f, REAL* pLabelZ=NULL, REAL* pLabelS=NULL) const;
	static void DrawArrow(Graphics& g, Brush& brush, REAL x1, REAL y1, REAL x2, REAL y2, INT MinS, INT MinZ, REAL Scale, REAL Upscale);
	static REAL MapX(REAL S);
	static REAL MapX(DOUBLE S);
	static REAL MapY(REAL Z);
	static REAL MapY(DOUBLE Z);

	MapSettings m_Settings;
};

inline REAL CMapFactory::MapX(REAL S)
{
	return S*BGWIDTH/(2*PI)+BGWIDTH/2;
}

inline REAL CMapFactory::MapX(DOUBLE S)
{
	return MapX((REAL)S);
}

inline REAL CMapFactory::MapY(REAL Z)
{
	return Z*BGHEIGHT/PI+BGHEIGHT/2;
}

inline REAL CMapFactory::MapY(DOUBLE Z)
{
	return MapY((REAL)Z);
}
