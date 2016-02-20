
// CMapFactory.h: Schnittstelle der Klasse CMapFactory
//

#pragma once
#include "CKitchen.h"
#include "Flightmap.h"


// CMapFactory
//

class CMapFactory
{
public:
	CMapFactory(MapSettings* pSettings);

	CBitmap* RenderMap(CKitchen* pKitchen, BOOL DeleteKitchen=TRUE);

protected:
	static void AppendLabel(CString& Buf, UINT nID, UINT MaxLines);
	CBitmap* LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset);
	void DrawLine(Graphics& g, Pen& pen, DOUBLE x1, DOUBLE y1, DOUBLE x2, DOUBLE y2, INT MinS=0, INT MinZ=0, DOUBLE Scale=1.0, DOUBLE* MidS=NULL, DOUBLE* MidZ=NULL);
	static void DrawArrow(Graphics& g, Brush& brush, DOUBLE x1, DOUBLE y1, DOUBLE x2, DOUBLE y2, INT MinS=0, INT MinZ=0, DOUBLE Scale=1.0, DOUBLE Upscale=1.0);
	static void Deface(CBitmap* pBitmap);

	MapSettings m_Settings;
};
