
// CMapFactory.cpp: Implementierung der Klasse CMapFactory
//

#pragma once
#include "stdafx.h"
#include "CMapFactory.h"
#include "Flightmap.h"


// CColor
//

CColor::CColor(COLORREF clr)
	: Color(clr & 0xFF, (clr>>8) & 0xFF, (clr>>16) & 0xFF)
{
}


// CMapFactory
//

struct FactoryAirportData
{
	RECT Spot;
	RECT IATA;
	DOUBLE Z;
	DOUBLE S;
	BYTE TBLR;
};

CMapFactory::CMapFactory(MapSettings* pSettings)
{
	m_Settings = *pSettings;
}

CBitmap* CMapFactory::RenderMap(CKitchen* pKitchen, BOOL DeleteKitchen)
{
	// Initialize
	INT MinS = 8192;
	INT MinZ = 4096;
	INT MaxS = 0;
	INT MaxZ = 0;
	DOUBLE Upscale = 2.0;

	UINT AirportCount = pKitchen->m_FlightAirports.GetCount();
	FactoryAirportData* AirportData = new FactoryAirportData[AirportCount];
	ZeroMemory(AirportData, AirportCount*sizeof(FactoryAirportData));

	UINT RouteCount = pKitchen->m_FlightRoutes.GetCount();
	FlightSegments** RouteData = NULL;

	// Convert coordinates to row/column, and compute background boundaries
	CFlightAirports::CPair* pPair1 = pKitchen->m_FlightAirports.PGetFirstAssoc();
	UINT Cnt = 0;
	while (pPair1)
	{
		AirportData[Cnt].Z = (pPair1->value.pAirport->Location.Latitude*2048.0)/90.0+2048.0;
		AirportData[Cnt].S = (pPair1->value.pAirport->Location.Longitude*4096.0)/180.0+4096.0;

		MinS = min(MinS, (INT)AirportData[Cnt].S);
		MinZ = min(MinZ, (INT)AirportData[Cnt].Z);
		MaxS = max(MaxS, (INT)AirportData[Cnt].S);
		MaxZ = max(MaxZ, (INT)AirportData[Cnt].Z);

		pPair1->value.lpAirport = &AirportData[Cnt++];

		pPair1 = pKitchen->m_FlightAirports.PGetNextAssoc(pPair1);
	}

	// Match routes with airport data
	CFlightRoutes::CPair* pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
	while (pPair2)
	{
		FlightAirport Airport;

		VERIFY(pKitchen->m_FlightAirports.Lookup(pPair2->value.pFrom->Code, Airport));
		pPair2->value.lpFrom = Airport.lpAirport;
		VERIFY(pKitchen->m_FlightAirports.Lookup(pPair2->value.pTo->Code, Airport));
		pPair2->value.lpTo = Airport.lpAirport;

		pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
	}

	// Tesselate routes if neccessary, and check for wrap-arounds of routes
	if ((m_Settings.ShowFlightRoutes) && (RouteCount>0))
	{
		BOOL WrapAround = FALSE;

		if (!m_Settings.StraightLines)
		{
			// Tesselated routes
			RouteData = new FlightSegments*[RouteCount];
			ZeroMemory(RouteData, RouteCount*sizeof(FlightSegments*));

			pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
			UINT Cnt = 0;
			while (pPair2)
			{
				FlightSegments* pSegments = RouteData[Cnt++] = pKitchen->Tesselate(pPair2->value);
				for (UINT b=0; b<pSegments->PointCount; b++)
				{
					if (b>0)
						WrapAround |= ((pSegments->Points[b-1][1]<-2.0) && (pSegments->Points[b][1]>2.0)) ||
							((pSegments->Points[b-1][1]>2.0) && (pSegments->Points[b][1]<-2.0));
				}

				pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
			}
		}
		else
		{
			// Straight routes
			pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
			while (pPair2)
			{
				const FactoryAirportData* pFrom = (FactoryAirportData*)pPair2->value.lpFrom;
				const FactoryAirportData* pTo = (FactoryAirportData*)pPair2->value.lpTo;
				if (((pFrom->S<2500.0) && (pTo->S>5500.0)) || ((pFrom->S>5500.0) && (pTo->S<2500.0)))
				{
					WrapAround = TRUE;
					break;
				}

				pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
			}
		}

		if (WrapAround)
		{
			MinS = 0;
			MaxS = 8191;
		}
	}

	// Border
	MinS = max(0, MinS-94);
	MinZ = max(0, MinZ-54);
	MaxS = min(8191, MaxS+94);
	MaxZ = min(4095, MaxZ+54);

	// Background
	INT Width = MaxS-MinS+1;
	INT Height = MaxZ-MinZ+1;
	CBitmap* pBitmap = LoadBackground(MinS, MinZ, Width, Height);

	// Obtain device context and graphics surface
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);

	// Draw routes
	if (m_Settings.ShowFlightRoutes)
	{
	}

	// Draw locations
	if (m_Settings.ShowLocations)
	{
		SolidBrush brush(CColor(m_Settings.LocationInnerColor));
		Pen pen(CColor(m_Settings.LocationOuterColor), (REAL)(2.0*Upscale));
		DOUBLE Radius = 11.0*Upscale;

		for (UINT a=0; a<AirportCount; a++)
		{
			g.FillEllipse(&brush, (REAL)(AirportData[a].S-MinS-Radius/2.0), (REAL)(AirportData[a].Z-MinZ-Radius/2.0), (REAL)(Radius-1.0), (REAL)(Radius-1.0));
			g.DrawEllipse(&pen, (REAL)(AirportData[a].S-MinS-Radius/2.0), (REAL)(AirportData[a].Z-MinZ-Radius/2.0), (REAL)Radius, (REAL)Radius);
		}
	}

	// Clean up GDI
	dc.SelectObject(pOldBitmap);

	// Deface
#ifndef _DEBUG
	if (TRUE)
		Deface(pBitmap);
#endif

	// Finish
	if (DeleteKitchen)
		delete pKitchen;

	delete[] AirportData;
	if (RouteData)
	{
		for (UINT a=0; a<RouteCount; a++)
			free(RouteData[a]);

		delete[] RouteData;
	}

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

	if (m_Settings.Background>=3)
	{
		dc.FillSolidRect(0, 0, Width, Height, m_Settings.BackgroundColor);
	}
	else
	{
		const UINT ResID[3] = { IDB_BLUEMARBLE_8192, IDB_NIGHT_8192, IDB_GRAY_8192 };
		CGdiPlusBitmapResource img(ResID[m_Settings.Background], m_Settings.Background==2 ? _T("PNG") : _T("JPG"));

		Graphics g(dc);
		g.DrawImage(img.m_pBitmap, -Left, -Top, 8192, 4096);
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
