
// CMapFactory.cpp: Implementierung der Klasse CMapFactory
//

#pragma once
#include "stdafx.h"
#include "CMapFactory.h"
#include "Flightmap.h"


void AppendLabel(CString& Buf, UINT nID, UINT MaxLines)
{
	if (!Buf.IsEmpty())
		Buf.Append(_T("\n"));

	if (MaxLines>1)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(nID));
		Buf.Append(tmpStr);
		Buf.Append(_T(": "));
	}
}


// CColor
//

CColor::CColor(COLORREF clr)
	: Color(clr & 0xFF, (clr>>8) & 0xFF, (clr>>16) & 0xFF)
{
}


// CMapFactory
//

#define BGWIDTH        8192
#define BGHEIGHT       4096
#define WRAPMARGIN     2500

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
	INT MinS = BGWIDTH;
	INT MinZ = BGHEIGHT;
	INT MaxS = 0;
	INT MaxZ = 0;
	INT MapOffset = m_Settings.CenterPacific ? 4700 : 0;
	DOUBLE Scale = 1.0;

	const UINT AirportCount = (UINT)pKitchen->m_FlightAirports.GetCount();
	FactoryAirportData* AirportData = new FactoryAirportData[AirportCount];
	ZeroMemory(AirportData, AirportCount*sizeof(FactoryAirportData));

	const UINT RouteCount = (UINT)pKitchen->m_FlightRoutes.GetCount();
	FlightSegments** RouteData = NULL;

	// Convert coordinates to row/column, and compute minimum background boundaries
	CFlightAirports::CPair* pPair1 = pKitchen->m_FlightAirports.PGetFirstAssoc();
	UINT Cnt = 0;
	while (pPair1)
	{
		AirportData[Cnt].Z = (pPair1->value.pAirport->Location.Latitude*2048.0)/90.0+2048.0;
		AirportData[Cnt].S = (pPair1->value.pAirport->Location.Longitude*4096.0)/180.0+4096.0+MapOffset;
		if (AirportData[Cnt].S>=BGWIDTH)
			AirportData[Cnt].S -= BGWIDTH;

		MinS = min(MinS, (INT)AirportData[Cnt].S);
		MinZ = min(MinZ, (INT)AirportData[Cnt].Z);
		MaxS = max(MaxS, (INT)AirportData[Cnt].S);
		MaxZ = max(MaxZ, (INT)AirportData[Cnt].Z);

		pPair1->value.lpAirport = &AirportData[Cnt++];

		pPair1 = pKitchen->m_FlightAirports.PGetNextAssoc(pPair1);
	}

	if (AirportCount==0)
	{
		MinS = 0;
		MaxS = 8191;
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

			const DOUBLE Offs = (2.0*PI*MapOffset)/BGWIDTH;

			pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
			UINT Cnt = 0;
			while (pPair2)
			{
				FlightSegments* pSegments = RouteData[Cnt++] = pKitchen->Tesselate(pPair2->value);
				for (UINT b=0; b<pSegments->PointCount; b++)
				{
					pSegments->Points[b][1] += Offs;
					if (pSegments->Points[b][1]>=PI)
						pSegments->Points[b][1] -= 2*PI;

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
				if (((pFrom->S<WRAPMARGIN) && (pTo->S>BGWIDTH-WRAPMARGIN)) || ((pFrom->S>BGWIDTH-WRAPMARGIN) && (pTo->S<WRAPMARGIN)))
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

	// Compute true boundaries and scale
	if (((m_Settings.Width<BGWIDTH) || (m_Settings.Height<BGHEIGHT)) && (MaxZ>0))
	{
		INT S = (MinS+MaxS)/2;
		INT Z = (MinZ+MaxZ)/2;
		DOUBLE L = MaxS-MinS+1;
		DOUBLE H = MaxZ-MinZ+1;
		const DOUBLE ScX = L/m_Settings.Width;
		const DOUBLE ScY = H/m_Settings.Height;

		Scale = max(ScX, ScY);
		if (Scale<0.5)
			Scale = 0.5;

		MinS = S-(INT)(m_Settings.Width*Scale/2.0);
		MaxS = S+(INT)(m_Settings.Width*Scale/2.0)-1;
		MinZ = Z-(INT)(m_Settings.Height*Scale/2.0);
		MaxZ = Z+(INT)(m_Settings.Height*Scale/2.0)-1;

		if (MinS<0)
		{
			MaxS += -MinS;
			MinS = 0;
		}
		if (MinZ<0)
		{
			MaxZ += -MinZ;
			MinZ = 0;
		}

		if (MaxS>BGWIDTH-1)
		{
			if (MinS>MaxS-BGWIDTH+1)
				MinS -= MaxS-BGWIDTH+1;
			MaxZ = BGHEIGHT-1;
		}
		if (MaxZ>BGHEIGHT-1)
		{
			if (MinZ>MaxZ-BGHEIGHT+1)
				MinZ -= MaxZ-BGHEIGHT+1;
			MaxZ = BGHEIGHT-1;
		}
	}
	else
	{
		MinS = 0; MaxS = BGWIDTH-1;
		MinZ = 0; MaxZ = BGHEIGHT-1;
	}

	// Background
	const INT Width = Scale<1.0 ? m_Settings.Width : MaxS-MinS+1;
	const INT Height = Scale<1.0 ? m_Settings.Height : MaxZ-MinZ+1;
	CBitmap* pBitmap = LoadBackground(MinS, MinZ, MaxS, MaxZ, Width, Height, MapOffset);

	Scale = min(1.0, Scale);

	// Compute upscale
	CSize sz = pBitmap->GetBitmapDimension();
	const DOUBLE FinalScale = max((DOUBLE)m_Settings.Width/(DOUBLE)sz.cx, (DOUBLE)m_Settings.Height/(DOUBLE)sz.cy);
	const DOUBLE Upscale = max(1.0, 1.0+((1.0/FinalScale)-1.0)*0.75);

	// Obtain device context and graphics surface
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	// Draw routes
	if (m_Settings.ShowFlightRoutes)
	{
#define PreparePen(Route) \
	CColor col(((Route.Color==(COLORREF)-1) || !m_Settings.UseColors) ? m_Settings.RouteColor : Route.Color); \
	const DOUBLE Width = (m_Settings.UseCount && (pKitchen->m_MaxRouteCount!=0)) ? (0.5+(5.9*((DOUBLE)Route.Count)/((DOUBLE)pKitchen->m_MaxRouteCount))) : 3.2; \
	Pen pen(col, (REAL)(Width*Upscale));

		if (!m_Settings.StraightLines)
		{
			// Tesselated routes
			for (UINT a=0; a<RouteCount; a++)
			{
				const FlightSegments* pSegments = RouteData[a];
				PreparePen(pSegments->Route);

#define CompS(s) s*BGWIDTH/(2*PI)+BGWIDTH/2
#define CompZ(z) z*BGHEIGHT/PI+BGHEIGHT/2

				for (UINT b=1; b<pSegments->PointCount; b++)
					DrawLine(g, pen,
						CompS(pSegments->Points[b-1][1]), CompZ(pSegments->Points[b-1][0]),
						CompS(pSegments->Points[b][1]), CompZ(pSegments->Points[b][0]),
						MinS, MinZ, Scale);

				if (m_Settings.Arrows)
				{
					SolidBrush brush(col);

					if (pSegments->Route.Arrows & ARROW_FT)
						DrawArrow(g, brush,
							CompS(pSegments->Points[pSegments->PointCount-1][1]), CompZ(pSegments->Points[pSegments->PointCount-1][0]),
							CompS(pSegments->Points[pSegments->PointCount-2][1]), CompZ(pSegments->Points[pSegments->PointCount-2][0]),
							MinS, MinZ, Scale, Upscale);
					if (pSegments->Route.Arrows & ARROW_TF)
						DrawArrow(g, brush,
							CompS(pSegments->Points[0][1]), CompZ(pSegments->Points[0][0]),
							CompS(pSegments->Points[1][1]), CompZ(pSegments->Points[1][0]),
							MinS, MinZ, Scale, Upscale);
				}
			}
		}

		UINT RouteNo = 0;
		pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
		while (pPair2)
		{
			FactoryAirportData* pFrom = (FactoryAirportData*)pPair2->value.lpFrom;
			FactoryAirportData* pTo = (FactoryAirportData*)pPair2->value.lpTo;

			// Recommend label placement
			if ((pFrom->S<WRAPMARGIN) && (pTo->S>BGWIDTH-WRAPMARGIN))
			{
				pFrom->TBLR |= 4;
				pTo->TBLR |= 8;
			}
			else
				if ((pFrom->S>BGWIDTH-WRAPMARGIN) && (pTo->S<WRAPMARGIN))
				{
					pFrom->TBLR |= 8;
					pTo->TBLR |= 4;
				}
				else
					if (pFrom->S>pTo->S)
					{
						pFrom->TBLR |= 4;
						pTo->TBLR |= 8;
					}
					else
					{
						pFrom->TBLR |= 8;
						pTo->TBLR |= 4;
					}

			if (abs(pFrom->Z-pTo->Z)>abs(pFrom->S-pTo->S))
				if (pFrom->S>pTo->S)
				{
					pFrom->TBLR |= 1;
					pTo->TBLR |= 2;
				}
				else
				{
					pFrom->TBLR |= 2;
					pTo->TBLR |= 1;
				}

			// Straight routes
			if (m_Settings.StraightLines)
			{
				PreparePen(pPair2->value);

				const BOOL UseWaypoint = (pFrom==pTo) && ((pPair2->value.Waypoint.Latitude!=0.0) || (pPair2->value.Waypoint.Longitude!=0.0));
				DrawLine(g, pen, pFrom->S, pFrom->Z, UseWaypoint ? (pPair2->value.Waypoint.Longitude*4096.0)/180.0+4096.0+MapOffset : pTo->S, UseWaypoint ? (pPair2->value.Waypoint.Latitude*2048.0)/90.0+2048.0 : pTo->Z, MinS, MinZ, Scale, &pPair2->value.LabelS, &pPair2->value.LabelZ);

				if (m_Settings.Arrows && !UseWaypoint)
				{
					SolidBrush brush(col);

					if (pPair2->value.Arrows & ARROW_FT)
						DrawArrow(g, brush, pTo->S, pTo->Z, pFrom->S, pFrom->Z, MinS, MinZ, Scale, Upscale);
					if (pPair2->value.Arrows & ARROW_TF)
						DrawArrow(g, brush, pFrom->S, pFrom->Z, pTo->S, pTo->Z, MinS, MinZ, Scale, Upscale);
				}
			}
			else
			{
				const FlightSegments* pSegments = RouteData[RouteNo++];

				pPair2->value.LabelS = (CompS(pSegments->Points[pSegments->PointCount/2][1])-MinS)/Scale;
				pPair2->value.LabelZ = (CompZ(pSegments->Points[pSegments->PointCount/2][0])-MinZ)/Scale;
			}

			pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
		}
	}

	// Draw locations
	const DOUBLE Radius = 5.5*Upscale;
	if (m_Settings.ShowLocations)
	{
		SolidBrush brush(CColor(m_Settings.LocationInnerColor));
		Pen pen(CColor(m_Settings.LocationOuterColor), (REAL)(2.0*Upscale));

		for (UINT a=0; a<AirportCount; a++)
		{
			const DOUBLE S = (AirportData[a].S-MinS)/Scale;
			const DOUBLE Z = (AirportData[a].Z-MinZ)/Scale;

			AirportData[a].Spot.left = (INT)(S-Radius);
			AirportData[a].Spot.right = (INT)(S+Radius);
			AirportData[a].Spot.top = (INT)(Z-Radius);
			AirportData[a].Spot.bottom = (INT)(Z+Radius);

			g.FillEllipse(&brush, (REAL)(S-Radius), (REAL)(Z-Radius), (REAL)(Radius*2.0-1.0), (REAL)(Radius*2.0-1.0));
			g.DrawEllipse(&pen, (REAL)(S-Radius), (REAL)(Z-Radius), (REAL)(Radius*2.0), (REAL)(Radius*2.0));
		}
	}

	// Draw IATA codes
	if (m_Settings.ShowIATACodes)
	{
		FontFamily font(_T("Arial"));
		Pen pen(CColor(m_Settings.IATAOuterColor), (REAL)(2.0*Upscale+0.5));
		SolidBrush brush(CColor(m_Settings.IATAInnerColor));

		pen.SetLineJoin(LineJoinRound);

		pPair1 = pKitchen->m_FlightAirports.PGetFirstAssoc();
		while (pPair1)
		{
			FactoryAirportData* pData = (FactoryAirportData*)pPair1->value.lpAirport;
			const INT S = (INT)((pData->S-MinS)/Scale);
			const INT Z = (INT)((pData->Z-MinZ)/Scale);

			// Prepare path
			WCHAR pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, pPair1->value.pAirport->Code, -1, pszBuf, 4);

			StringFormat strformat;
			GraphicsPath TextPath;
			TextPath.Reset();
			TextPath.AddString(pszBuf, (INT)wcslen(pszBuf), &font, FontStyleRegular, (REAL)(16.0*Upscale), Gdiplus::Point(0, 0), &strformat);

			Rect rectPath;
			TextPath.GetBounds(&rectPath);
			const INT L = rectPath.Width+(INT)(5.5*Upscale);
			const INT H = rectPath.Height+(INT)(4.5*Upscale);

			// Find location
			for (UINT a=0; a<16; a++)
			{
				CRect rectLabel(S, Z, S+L, Z+H);

				switch (a%7)
				{
				case 0:
					if (((a&8)==0) && (pData->TBLR & 8))
						continue;
					rectLabel.OffsetRect((INT)(Radius*1.5), -H/2);
					break;
				case 1:
					if (((a&8)==0) && (pData->TBLR & 4))
						continue;
					rectLabel.OffsetRect(-L-(INT)(Radius*1.5), -H/2);
					break;
				case 2:
					if (((a&8)==0) && (pData->TBLR & 2))
						continue;
					rectLabel.OffsetRect(-L/2, (INT)(Radius*1.5));
					break;
				case 3:
					if (((a&8)==0) && (pData->TBLR & 1))
						continue;
					rectLabel.OffsetRect(-L/2, -H-(INT)(Radius*1.5));
					break;
				case 4:
					if (((a&8)==0) && (pData->TBLR & 2))
						continue;
					rectLabel.OffsetRect(-L-(INT)(Radius*1.5), (INT)(Radius*1.5));
					break;
				case 5:
					if (((a&8)==0) && (pData->TBLR & 1))
						continue;
					rectLabel.OffsetRect(-L-(INT)(Radius*1.5), -H-(INT)(Radius*1.5));
					break;				
				case 6:
					if (((a&8)==0) && (pData->TBLR & 2))
						continue;
					rectLabel.OffsetRect((INT)(Radius*1.5), (INT)(Radius*1.5));
					break;
				case 7:
					if (((a&8)==0) && (pData->TBLR & 1))
						continue;
					rectLabel.OffsetRect((INT)(Radius*1.5), -H-(INT)(Radius*1.5));
					break;
				}

				if (rectLabel.left<0)
					rectLabel.OffsetRect(-rectLabel.left, 0);
				if (rectLabel.top<0)
					rectLabel.OffsetRect(0, -rectLabel.top);
				if (rectLabel.right>=Width)
					rectLabel.OffsetRect(-(rectLabel.right-Width), 0);
				if (rectLabel.bottom>=Height)
					rectLabel.OffsetRect(0, -(rectLabel.bottom-Height));

				if ((rectLabel.left<0) || (rectLabel.top<0) || (rectLabel.right>=Width) || (rectLabel.bottom>=Height))
					continue;

				for (UINT b=0; b<AirportCount; b++)
				{
					RECT rect;
					if (IntersectRect(&rect, rectLabel, &AirportData[b].Spot))
						goto Skip;
					if (IntersectRect(&rect, rectLabel, &AirportData[b].IATA))
						goto Skip;
				}

				{
					Matrix m;
					m.Translate((REAL)rectLabel.left, (REAL)rectLabel.top);
					TextPath.Transform(&m);
				}

				g.DrawPath(&pen, &TextPath);
				g.FillPath(&brush, &TextPath);

				pData->IATA = rectLabel;
				break;
Skip:
				continue;
			}

			pPair1 = pKitchen->m_FlightAirports.PGetNextAssoc(pPair1);
		}
	}

	// Draw annotations
	if ((m_Settings.ShowFlightRoutes) && ((m_Settings.NoteDistance || m_Settings.NoteFlightCount || m_Settings.NoteFlightTime || m_Settings.NoteCarrier || m_Settings.NoteEquipment)))
	{
		const UINT MaxLines = (m_Settings.NoteDistance ? 1 : 0)+(m_Settings.NoteFlightCount ? 1 : 0)+(m_Settings.NoteFlightTime ? 1 : 0)+(m_Settings.NoteCarrier ? 1 : 0)+(m_Settings.NoteEquipment ? 1 : 0);

		RECT* RouteLabel = new RECT[RouteCount];
		ZeroMemory(RouteLabel, sizeof(RECT)*RouteCount);

		FontFamily font(_T("Tahoma"));
		Pen pen(CColor(m_Settings.NoteOuterColor), (REAL)((m_Settings.NoteSmallFont ? 1.75 : 2.0)*Upscale+0.5));
		SolidBrush brush(CColor(m_Settings.NoteInnerColor));

		pen.SetLineJoin(LineJoinRound);

		UINT CurRoute = 0;
		pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
		while (pPair2)
		{
			CString Buf;
			WCHAR tmpStr[256];
			if ((m_Settings.NoteDistance) && (pPair2->value.DistanceNM!=0.0))
			{
				AppendLabel(Buf, IDS_COLUMN6, MaxLines);
				DistanceToString(tmpStr, 256, pPair2->value.DistanceNM);
				Buf.Append(tmpStr);
			}
			if ((m_Settings.NoteFlightTime) && (pPair2->value.FlightTimeCount))
			{
				AppendLabel(Buf, IDS_COLUMN23, MaxLines);
				UINT FlightTime = pPair2->value.FlightTime/pPair2->value.FlightTimeCount;
				swprintf(tmpStr, 256, L"%02d:%02d", FlightTime/60, FlightTime%60);
				Buf.Append(tmpStr);
			}
			if (m_Settings.NoteFlightCount)
			{
				CString tmpMask(_T("%d"));
				if (MaxLines>1)
				{
					if (!Buf.IsEmpty())
						Buf.Append(_T("\n"));
					ENSURE(tmpMask.LoadString(pPair2->value.Count==1 ? IDS_FLIGHTS_SINGULAR : IDS_FLIGHTS_PLURAL));
				}
				CString tmpStr;
				tmpStr.Format(tmpMask, pPair2->value.Count);
				Buf.Append(tmpStr);
			}
			if ((m_Settings.NoteCarrier) && (pPair2->value.Carrier[0]!=L'\0') && (!pPair2->value.CarrierMultiple))
			{
				AppendLabel(Buf, IDS_COLUMN7, MaxLines);
				Buf.Append(pPair2->value.Carrier);
			}
			if ((m_Settings.NoteCarrier) && (pPair2->value.Equipment[0]!=L'\0') && (!pPair2->value.EquipmentMultiple))
			{
				AppendLabel(Buf, IDS_COLUMN10, MaxLines);
				Buf.Append(pPair2->value.Equipment);
			}

			StringFormat strformat;
			GraphicsPath TextPath;
			TextPath.Reset();
			TextPath.AddString(Buf, Buf.GetLength(), &font, FontStyleRegular, (REAL)((m_Settings.NoteSmallFont ? 14.0 : 16.0)*Upscale), Gdiplus::Point(0, 0), &strformat);

			Rect rectPath;
			TextPath.GetBounds(&rectPath);
			const DOUBLE S = pPair2->value.LabelS;
			const DOUBLE Z = pPair2->value.LabelZ;
			const INT L = rectPath.Width+(INT)(5.5*Upscale);
			const INT H = rectPath.Height+(INT)(4.5*Upscale);

			CRect rectLabel((INT)(S-L/2.0), (INT)(Z-H/2.0), (INT)(S+L/2.0), (INT)(Z+H/2.0));
			if ((S==-1.0) || (Z==-1.0))
				goto SkipNote;
			if (rectLabel.left<0)
				rectLabel.OffsetRect(-rectLabel.left, 0);
			if (rectLabel.top<0)
				rectLabel.OffsetRect(0, -rectLabel.top);
			if (rectLabel.right>=Width)
				rectLabel.OffsetRect(-(rectLabel.right-Width), 0);
			if (rectLabel.bottom>=Height)
				rectLabel.OffsetRect(0, -(rectLabel.bottom-Height));

			if ((rectLabel.left<0) || (rectLabel.top<0) || (rectLabel.right>=Width) || (rectLabel.bottom>=Height))
				goto SkipNote;

			for (UINT a=0; a<AirportCount; a++)
			{
				RECT rect;
				if (IntersectRect(&rect, rectLabel, &AirportData[a].Spot))
					goto SkipNote;
				if (IntersectRect(&rect, rectLabel, &AirportData[a].IATA))
					goto SkipNote;
			}
			for (UINT a=0; a<CurRoute; a++)
			{
				RECT rect;
				if (IntersectRect(&rect, rectLabel, &RouteLabel[a]))
					goto SkipNote;
			}

			{
				Matrix m;
				m.Translate((REAL)rectLabel.left, (REAL)rectLabel.top);
				TextPath.Transform(&m);
			}

			g.DrawPath(&pen, &TextPath);
			g.FillPath(&brush, &TextPath);

			RouteLabel[CurRoute++] = rectLabel;
SkipNote:
			pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
		}

		delete[] RouteLabel;
	}

	// Clean up GDI
	dc.SelectObject(pOldBitmap);

	// Final scale
	if (FinalScale<1.0)
	{
		INT L = (INT)(Width*FinalScale);
		INT H = (INT)(Height*FinalScale);

		CBitmap* pBitmap2 = CreateBitmap(L, H);
		pOldBitmap = dc.SelectObject(pBitmap2);

		Graphics g(dc);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		Bitmap bmp((HBITMAP)pBitmap->m_hObject, NULL);
		g.DrawImage(&bmp, Rect(-1, -1, L+2, H+2), 0, 0, Width, Height, UnitPixel);

		dc.SelectObject(pOldBitmap);

		delete pBitmap;
		pBitmap = pBitmap2;
	}

	// Deface
#ifndef _DEBUG
	if (!FMIsLicensed())
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

CBitmap* CMapFactory::LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset)
{
	ASSERT(Left>=0);
	ASSERT(Top>=0);
	ASSERT(Width>=0);
	ASSERT(Height>=0);
	ASSERT(Left+Width<=BGWIDTH);
	ASSERT(Top+Height<=BGHEIGHT);

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

		ImageAttributes ImgAttr;
		ImgAttr.SetWrapMode(WrapModeTile);

		Graphics g(dc);
		g.DrawImage(img.m_pBitmap, Rect(0, 0, Width, Height), Left-MapOffset, Top, Right-Left, Bottom-Top, UnitPixel, &ImgAttr);
	}

	dc.SelectObject(pOldBitmap);

	return pBitmap;
}

void CMapFactory::DrawLine(Graphics& g, Pen& pen, DOUBLE x1, DOUBLE y1, DOUBLE x2, DOUBLE y2, INT MinS, INT MinZ, DOUBLE Scale, DOUBLE* MidS, DOUBLE* MidZ)
{
#define Line(pen, x1, y1, x2, y2) \
	g.DrawLine(&pen, (REAL)(x1), (REAL)(y1), (REAL)(x2), (REAL)(y2)); \
	g.DrawLine(&pen, (REAL)(x1), (REAL)(y1+1.0), (REAL)(x2), (REAL)(y2+1.0)); \
	g.DrawLine(&pen, (REAL)(x1+1.0), (REAL)(y1), (REAL)(x2+1.0), (REAL)(y2));

	x1 -= MinS;
	x2 -= MinS;
	y1 -= MinZ;
	y2 -= MinZ;

	if ((x1<WRAPMARGIN-MinS) && (x2>BGWIDTH-WRAPMARGIN-MinS))
	{
		Line(pen, x1, y1, x2-BGWIDTH, y2);
		Line(pen, x1+BGWIDTH, y1, x2, y2);

		if (MidS)
			*MidS = -1.0;
		if (MidZ)
			*MidZ = -1.0;
	}
	else
		if ((x1>BGWIDTH-WRAPMARGIN-MinS) && (x2<WRAPMARGIN-MinS))
		{
			Line(pen, x1-BGWIDTH, y1, x2, y2);
			Line(pen, x1, y1, x2+BGWIDTH, y2);

			if (MidS)
				*MidS = -1.0;
			if (MidZ)
				*MidZ = -1.0;
		}
		else
		{
			Line(pen, x1/Scale, y1/Scale, x2/Scale, y2/Scale);

			if (MidS)
				*MidS = (x1+x2)/(2.0*Scale);
			if (MidZ)
				*MidZ = (y1+y2)/(2.0*Scale);
		}
}

void CMapFactory::DrawArrow(Graphics& g, Brush& brush, DOUBLE x1, DOUBLE y1, DOUBLE x2, DOUBLE y2, INT MinS, INT MinZ, DOUBLE Scale, DOUBLE Upscale)
{
	if ((x1==x2) && (y1==y2))
		return;

	const DOUBLE Angle = atan2(y2-y1, x2-x1);
	x1 = (x1-MinS)/Scale;
	y1 = (y1-MinZ)/Scale;

	PointF points[3];
	points[0] = PointF((REAL)(4.0*Upscale*cos(Angle)+x1), (REAL)(4.0*Upscale*sin(Angle)+y1));
	points[1] = PointF((REAL)(16.0*Upscale*cos(Angle+PI/7)+x1), (REAL)(16.0*Upscale*sin(Angle+PI/7)+y1));
	points[2] = PointF((REAL)(16.0*Upscale*cos(Angle-PI/7)+x1), (REAL)(16.0*Upscale*sin(Angle-PI/7)+y1));

	g.FillPolygon(&brush, points, 3);
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
