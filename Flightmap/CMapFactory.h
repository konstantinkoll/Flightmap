
// CMapFactory.h: Schnittstelle der Klasse CMapFactory
//

#pragma once
#include "Flightmap.h"
#include "CKitchen.h"


// CMapFactory
//

class CMapFactory
{
public:
	CMapFactory(MapSettings* pSettings);

	CBitmap* RenderMap(CKitchen* pKitchen, BOOL DeleteKitchen=TRUE);

protected:
	static CBitmap* CreateBitmap(INT Width, INT Height);
	CBitmap* LoadBackground(INT Left, INT Top, INT Width, INT Height);
	static void Deface(CBitmap* pBitmap);

	MapSettings m_Settings;
};
