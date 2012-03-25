
// CMapFactory.cpp: Implementierung der Klasse CMapFactory
//

#pragma once
#include "stdafx.h"
#include "CMapFactory.h"
#include "Flightmap.h"


// CMapFactory
//

CMapFactory::CMapFactory(MapSettings* pSettings)
{
	m_Settings = *pSettings;
}

CBitmap* CMapFactory::RenderMap(CKitchen* pKitchen, BOOL DeleteKitchen)
{
	INT Width = 900;
	INT Height = 900;
	CBitmap* pBitmap = LoadBackground(1000, 1000, Width, Height);

	// Deface
#ifndef _DEBUG
	if (TRUE)
		Deface(pBitmap);
#endif

	// Finish
	if (DeleteKitchen)
		delete pKitchen;

	return pBitmap;
}

CBitmap* CMapFactory::CreateBitmap(INT Width, INT Height)
{
	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = Width;
	dib.bmiHeader.biHeight = -Height;
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 24;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(GetWindowDC(GetDesktopWindow()), &dib, DIB_RGB_COLORS, NULL, NULL, 0);

	CBitmap* pBitmap = new CBitmap();
	pBitmap->Attach(hBmp);
	pBitmap->SetBitmapDimension(Width, Height);

	return pBitmap;
}

CBitmap* CMapFactory::LoadBackground(INT Left, INT Top, INT Width, INT Height)
{
	ASSERT(Left>=0);
	ASSERT(Top>=0);
	ASSERT(Width>=0);
	ASSERT(Height>=0);
	ASSERT(Left+Width<=8192);
	ASSERT(Top+Height<=4096);

	CBitmap* pBitmap = CreateBitmap(Width, Height);

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	if (m_Settings.Background==3)
	{
		dc.FillSolidRect(0, 0, Width, Height, m_Settings.BackgroundColor);
	}
	else
	{
	}

	dc.SelectObject(pOldBitmap);

	return pBitmap;
}

__forceinline void CMapFactory::Deface(CBitmap* pBitmap)
{
	ASSERT(pBitmap);

	CSize sz = pBitmap->GetBitmapDimension();

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	for (INT z=0; z<sz.cy; z+=48)
		dc.FillSolidRect(0, z, sz.cx, 16, 0xFF00FF);

	dc.SelectObject(pOldBitmap);
}
