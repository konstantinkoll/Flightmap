
// CMapFactory.cpp: Implementierung der Klasse CMapFactory
//

#include "stdafx.h"
#include "CMapFactory.h"
#include "Flightmap.h"
#include <math.h>


// CMapFactory
//

#define WRAPMARGIN     2500

struct FactoryAirportData
{
	RECT Spot;
	RECT IATA;
	REAL Z;
	REAL S;
	BYTE TBLR;
};

CMapFactory::CMapFactory(MapSettings* pSettings)
{
	m_Settings = *pSettings;
}

CBitmap* CMapFactory::RenderMap(CKitchen* pKitchen)
{
	ASSERT(pKitchen);

	// Compute map boundaries and scale
	//

	// Initialize
	INT MinZ = BGHEIGHT;
	INT MaxZ = 0;
	INT MinS = BGWIDTH;
	INT MaxS = 0;
	const INT MapOffset = m_Settings.CenterPacific ? 4700 : 0;
	REAL Scale = 1.0f;

	const UINT AirportCount = (UINT)pKitchen->m_FlightAirports.GetCount();
	FactoryAirportData* pAirportData = new FactoryAirportData[AirportCount];
	ZeroMemory(pAirportData, AirportCount*sizeof(FactoryAirportData));

	const UINT RouteCount = (UINT)pKitchen->m_FlightRoutes.GetCount();
	FlightSegments** ppRouteData = NULL;

	// Convert airports' geocoordinates to map coordinates, and compute background boundaries
	UINT CurAirport = 0;
	for (CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair))
	{
		// Map coordinates
		pAirportData[CurAirport].Z = ((REAL)pPair->value.pAirport->Location.Latitude*2048.0f)/90.0f+2048.0f;
		pAirportData[CurAirport].S = ((REAL)pPair->value.pAirport->Location.Longitude*4096.0f)/180.0f+(MapOffset+4096);
		if (pAirportData[CurAirport].S>=BGWIDTH)
			pAirportData[CurAirport].S -= BGWIDTH;

		// Background boundaries
		const INT Z = (INT)pAirportData[CurAirport].Z;
		if (Z<MinZ)
			MinZ =Z;
		if (Z>MaxZ)
			MaxZ = Z;

		const INT S = (INT)pAirportData[CurAirport].S;
		if (S<MinS)
			MinS =S;
		if (S>MaxS)
			MaxS = S;

		pPair->value.lpAirport = &pAirportData[CurAirport++];
	}

	// When there are no airports, assume max dimension
	if (AirportCount==0)
	{
		MinS = 0;
		MaxS = BGWIDTH-1;
	}

	// Tesselate routes when required, and compute wrap-around the background edges
	if (m_Settings.ShowRoutes && (RouteCount>0))
	{
		BOOL WrapAround = FALSE;
		const DOUBLE Offs = (2.0*PI*MapOffset)/BGWIDTH;

		UINT CurRoute = 0;
		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			// Match routes with airport data
			FlightAirport Airport;

			VERIFY(pKitchen->m_FlightAirports.Lookup(pPair->value.pFrom->Code, Airport));
			const FactoryAirportData* pFrom = (FactoryAirportData*)(pPair->value.lpFrom = (FactoryAirportData*)Airport.lpAirport);

			VERIFY(pKitchen->m_FlightAirports.Lookup(pPair->value.pTo->Code, Airport));
			const FactoryAirportData* pTo = (FactoryAirportData*)(pPair->value.lpTo = (FactoryAirportData*)Airport.lpAirport);

			if (m_Settings.StraightLines)
			{
				// Straight lines
				WrapAround |= ((pFrom->S<WRAPMARGIN) && (pTo->S>BGWIDTH-WRAPMARGIN)) || ((pFrom->S>BGWIDTH-WRAPMARGIN) && (pTo->S<WRAPMARGIN));
			}
			else
			{
				// Great circle
				if (!ppRouteData)
				{
					// Array with pointers to tesselated routes
					ppRouteData = new FlightSegments*[RouteCount];
					ZeroMemory(ppRouteData, RouteCount*sizeof(FlightSegments*));
				}

				// Tesselate
				FlightSegments* pSegments = ppRouteData[CurRoute++] = pKitchen->Tesselate(pPair->value);

				// Check individual segments for wrap-around
				for (UINT a=0; a<pSegments->PointCount; a++)
				{
					if ((pSegments->Points[a][1]+=Offs)>=PI)
						pSegments->Points[a][1] -= 2*PI;

					if (a>0)
						WrapAround |= ((pSegments->Points[a-1][1]<-2.0) && (pSegments->Points[a][1]>2.0)) ||
							((pSegments->Points[a-1][1]>2.0) && (pSegments->Points[a][1]<-2.0));
				}
			}
		}

		// When routes wrap-around the background edges, force maximum width
		if (WrapAround)
		{
			MinS = 0;
			MaxS = BGWIDTH-1;
		}
	}

	// Add borders to boundaries
	const INT BorderZ = m_Settings.WideBorder ? 254:54;
	MinZ = max(0, MinZ-BorderZ);
	MaxZ = min(BGHEIGHT-1, MaxZ+BorderZ);

	const INT BorderS = m_Settings.WideBorder ? 294:94;
	MinS = max(0, MinS-BorderS);
	MaxS = min(BGWIDTH-1, MaxS+BorderS);

	// Compute true boundaries and scale
	if (((m_Settings.Width<BGWIDTH) || (m_Settings.Height<BGHEIGHT)) && (MaxZ>0))
	{
		const INT Z = (MinZ+MaxZ)/2;
		const INT S = (MinS+MaxS)/2;
		const REAL ScX = (REAL)(MaxS-MinS+1)/(REAL)m_Settings.Width;
		const REAL ScY = (REAL)(MaxZ-MinZ+1)/(REAL)m_Settings.Height;

		Scale = max(ScX, ScY);
		if (Scale<0.5f)
			Scale = 0.5f;

		MinS = S-(INT)(m_Settings.Width*Scale/2.0f);
		MaxS = S+(INT)(m_Settings.Width*Scale/2.0f)-1;
		MinZ = Z-(INT)(m_Settings.Height*Scale/2.0f);
		MaxZ = Z+(INT)(m_Settings.Height*Scale/2.0f)-1;

		if (MinZ<0)
		{
			MaxZ -= MinZ;
			MinZ = 0;
		}

		if (MaxZ>BGHEIGHT-1)
		{
			if (MinZ>MaxZ-BGHEIGHT+1)
				MinZ -= MaxZ-BGHEIGHT+1;

			MaxZ = BGHEIGHT-1;
		}

		if (MinS<0)
		{
			MaxS -= MinS;
			MinS = 0;
		}

		if (MaxS>BGWIDTH-1)
		{
			if (MinS>MaxS-BGWIDTH+1)
				MinS -= MaxS-BGWIDTH+1;

			MaxZ = BGHEIGHT-1;
		}
	}
	else
	{
		MinZ = MinS = 0;
		MaxZ = BGHEIGHT-1;
		MaxS = BGWIDTH-1;
	}

	// Background
	const INT Width = Scale<=1.0f ? m_Settings.Width : MaxS-MinS+1;
	const INT Height = Scale<=1.0f ? m_Settings.Height : MaxZ-MinZ+1;
	CBitmap* pBitmap = LoadBackground(MinS, MinZ, MaxS, MaxZ, Width, Height, MapOffset);

	// Use 1:1 scale for drawing when downscaling entire image later
	if (Scale>1.0f)
		Scale = 1.0f;

	// Compute foreground upscale
	CSize Size = pBitmap->GetBitmapDimension();
	const REAL FinalScale = max((REAL)m_Settings.Width/(REAL)Size.cx, (REAL)m_Settings.Height/(REAL)Size.cy);

	REAL Upscale = 1.0f;
	switch (m_Settings.ForegroundScale)
	{
	case 0:
		Upscale = max(1.0f, 1.0f+((1.0f/FinalScale)-1.0f)*0.8f);
		break;

	case 1:
		Upscale = max(1.0f, 1.0f/FinalScale);
		break;

	case 2:
		Upscale = max(2.0f, 2.0f/FinalScale);
		break;
	}


	// Draw map
	//

	// Obtain device context and graphics surface
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	Graphics g(dc);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	g.SetSmoothingMode(FMGetApp()->m_SmoothingModeAntiAlias8x8);

	// Draw routes
	if (m_Settings.ShowRoutes && (RouteCount>0))
	{
#define PreparePen(Route) \
	const BYTE Alpha = (m_Settings.UseCountOpacity && (pKitchen->m_MaxRouteCount!=0)) ? 0x60+(BYTE)(159.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1))) : 0xFF; \
	Color col(COLORREF2ARGB((((Route.Color==(COLORREF)-1) || !m_Settings.UseColors) ? m_Settings.RouteColor : Route.Color), Alpha)); \
	const DOUBLE Width = (m_Settings.UseCountWidth && (pKitchen->m_MaxRouteCount!=0)) ? (0.2+(3.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1)))) : 3.2; \
	Pen pen(col, (REAL)(Width*Upscale));

		if (!m_Settings.StraightLines)
		{
			// Draw tesselated routes
			for (UINT a=0; a<RouteCount; a++)
			{
				const FlightSegments* pSegments = ppRouteData[a];
				PreparePen(pSegments->Route);

				for (UINT b=1; b<pSegments->PointCount; b++)
					DrawLine(g, pen,
						MapX(pSegments->Points[b-1][1]), MapY(pSegments->Points[b-1][0]),
						MapX(pSegments->Points[b][1]), MapY(pSegments->Points[b][0]),
						MinS, MinZ, Scale);

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity)
				{
					SolidBrush brush(col);

					if (pSegments->Route.Arrows & ARROW_FT)
						DrawArrow(g, brush,
							MapX(pSegments->Points[pSegments->PointCount-1][1]), MapY(pSegments->Points[pSegments->PointCount-1][0]),
							MapX(pSegments->Points[pSegments->PointCount-2][1]), MapY(pSegments->Points[pSegments->PointCount-2][0]),
							MinS, MinZ, Scale, Upscale);

					if (pSegments->Route.Arrows & ARROW_TF)
						DrawArrow(g, brush,
							MapX(pSegments->Points[0][1]), MapY(pSegments->Points[0][0]),
							MapX(pSegments->Points[1][1]), MapY(pSegments->Points[1][0]),
							MinS, MinZ, Scale, Upscale);
				}
			}
		}

		UINT CurRoute = 0;
		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			FactoryAirportData* pFrom = (FactoryAirportData*)pPair->value.lpFrom;
			FactoryAirportData* pTo = (FactoryAirportData*)pPair->value.lpTo;

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

			if (m_Settings.StraightLines)
			{
				// Draw straight routes
				PreparePen(pPair->value);

				const BOOL UseWaypoint = (pFrom==pTo) && ((pPair->value.Waypoint.Latitude!=0.0) || (pPair->value.Waypoint.Longitude!=0.0));
				DrawLine(g, pen, pFrom->S, pFrom->Z, UseWaypoint ? ((REAL)pPair->value.Waypoint.Longitude*4096.0f)/180.0f+4096.0f+MapOffset : pTo->S, UseWaypoint ? ((REAL)pPair->value.Waypoint.Latitude*2048.0f)/90.0f+2048.0f : pTo->Z, MinS, MinZ, Scale, &pPair->value.LabelZ, &pPair->value.LabelS);

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity && !UseWaypoint)
				{
					SolidBrush brush(col);

					if (pPair->value.Arrows & ARROW_FT)
						DrawArrow(g, brush, pTo->S, pTo->Z, pFrom->S, pFrom->Z, MinS, MinZ, Scale, Upscale);

					if (pPair->value.Arrows & ARROW_TF)
						DrawArrow(g, brush, pFrom->S, pFrom->Z, pTo->S, pTo->Z, MinS, MinZ, Scale, Upscale);
				}
			}
			else
			{
				const FlightSegments* pSegments = ppRouteData[CurRoute++];

				pPair->value.LabelZ = (MapY(pSegments->Points[pSegments->PointCount/2][0])-MinZ)/Scale;
				pPair->value.LabelS = (MapX(pSegments->Points[pSegments->PointCount/2][1])-MinS)/Scale;
			}
		}
	}

	// Draw locations
	const REAL Radius = 5.5f*Upscale;

	if (m_Settings.ShowLocations)
	{
		Pen pen(Color(COLORREF2RGB(m_Settings.LocationsOuterColor)), 2.0f*Upscale);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.LocationsInnerColor)));

		for (UINT a=0; a<AirportCount; a++)
		{
			const REAL Z = (pAirportData[a].Z-MinZ)/Scale;
			const REAL S = (pAirportData[a].S-MinS)/Scale;

			pAirportData[a].Spot.left = (INT)(S-Radius);
			pAirportData[a].Spot.right = (INT)(S+Radius);
			pAirportData[a].Spot.top = (INT)(Z-Radius);
			pAirportData[a].Spot.bottom = (INT)(Z+Radius);

			g.FillEllipse(&brush, S-Radius, Z-Radius, Radius*2.0f-1.0f, Radius*2.0f-1.0f);

			if (m_Settings.LocationsOuterColor!=(COLORREF)-1)
				g.DrawEllipse(&pen, S-Radius, Z-Radius, Radius*2.0f, Radius*2.0f);
		}
	}

	// Draw IATA codes
	if (m_Settings.ShowIATACodes)
	{
		FontFamily font(_T("Arial"));

		Pen pen(Color(COLORREF2RGB(m_Settings.IATACodesOuterColor)), (REAL)(2.0*Upscale+0.5));
		pen.SetLineJoin(LineJoinRound);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.IATACodesInnerColor)));

		for (CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair))
		{
			FactoryAirportData* pData = (FactoryAirportData*)pPair->value.lpAirport;
			const INT Z = (INT)((pData->Z-MinZ)/Scale);
			const INT S = (INT)((pData->S-MinS)/Scale);

			// Prepare path
			WCHAR pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, pPair->value.pAirport->Code, -1, pszBuf, 4);

			StringFormat strformat;
			GraphicsPath TextPath;
			TextPath.AddString(pszBuf, (INT)wcslen(pszBuf), &font, FontStyleRegular, 16.0f*Upscale, Gdiplus::Point(0, 0), &strformat);

			Rect rectPath;
			TextPath.GetBounds(&rectPath);
			const INT L = rectPath.Width+(INT)(5.5*Upscale);
			const INT H = rectPath.Height+(INT)(4.5*Upscale);

			// Find suitable location
			Matrix m;

			for (UINT a=0; a<16; a++)
			{
				CRect rectLabel(S, Z, S+L, Z+H);

				switch (a%7)
				{
				case 0:
					if ((a<8) && (pData->TBLR & 8))
						continue;

					rectLabel.OffsetRect((INT)(Radius*1.5f), -H/2);

					break;

				case 1:
					if ((a<8) && (pData->TBLR & 4))
						continue;

					rectLabel.OffsetRect(-L-(INT)(Radius*1.5f), -H/2);

					break;

				case 2:
					if ((a<8) && (pData->TBLR & 2))
						continue;

					rectLabel.OffsetRect(-L/2, (INT)(Radius*1.5f));

					break;

				case 3:
					if ((a<8) && (pData->TBLR & 1))
						continue;

					rectLabel.OffsetRect(-L/2, -H-(INT)(Radius*1.5f));

					break;

				case 4:
					if ((a<8) && (pData->TBLR & 2))
						continue;

					rectLabel.OffsetRect(-L-(INT)(Radius*1.2f), (INT)(Radius*1.2f));

					break;

				case 5:
					if ((a<8) && (pData->TBLR & 1))
						continue;

					rectLabel.OffsetRect(-L-(INT)(Radius*1.2f), -H-(INT)(Radius*1.2f));

					break;

				case 6:
					if ((a<8) && (pData->TBLR & 2))
						continue;

					rectLabel.OffsetRect((INT)(Radius*1.2f), (INT)(Radius*1.2f));

					break;

				case 7:
					if ((a<8) && (pData->TBLR & 1))
						continue;

					rectLabel.OffsetRect((INT)(Radius*1.2f), -H-(INT)(Radius*1.2f));

					break;
				}

				// Move label rectangle back into map boundaries
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

				// Find intersections
				RECT rect;

				for (UINT b=0; b<AirportCount; b++)
				{
					if (IntersectRect(&rect, rectLabel, &pAirportData[b].Spot))
						goto Skip;

					if (IntersectRect(&rect, rectLabel, &pAirportData[b].IATA))
						goto Skip;
				}

				// Draw label
				m.Reset();
				m.Translate((REAL)rectLabel.left, (REAL)rectLabel.top);
				TextPath.Transform(&m);

				if (m_Settings.IATACodesOuterColor!=(COLORREF)-1)
					g.DrawPath(&pen, &TextPath);

				g.FillPath(&brush, &TextPath);

				pData->IATA = rectLabel;

				break;

Skip:
				continue;
			}
		}
	}

	// Draw annotations
	if ((m_Settings.ShowRoutes) && ((m_Settings.NoteDistance || m_Settings.NoteFlightCount || m_Settings.NoteFlightTime || m_Settings.NoteCarrier || m_Settings.NoteEquipment)))
	{
		FontFamily font(m_Settings.NoteSmallFont ? _T("Tahoma") : _T("Arial"));

		Pen pen(Color(COLORREF2RGB(m_Settings.NoteOuterColor)), (m_Settings.NoteSmallFont ? 1.75f : 2.0f)*Upscale+0.5f);
		pen.SetLineJoin(LineJoinRound);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.NoteInnerColor)));

		const UINT MaxLines = (m_Settings.NoteDistance ? 1 : 0)+(m_Settings.NoteFlightCount ? 1 : 0)+(m_Settings.NoteFlightTime ? 1 : 0)+(m_Settings.NoteCarrier ? 1 : 0)+(m_Settings.NoteEquipment ? 1 : 0);

		RECT* RouteLabel = new RECT[RouteCount];
		ZeroMemory(RouteLabel, sizeof(RECT)*RouteCount);

		UINT CurRoute = 0;
		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			// Skip if there is no anchor point
			const REAL Z = pPair->value.LabelZ;
			const REAL S = pPair->value.LabelS;

			if ((S==-1.0f) || (Z==-1.0f))
				continue;

			// Create annotation text
			CString Buf;
			WCHAR tmpStr[256];

			if ((m_Settings.NoteDistance) && (pPair->value.DistanceNM!=0.0))
			{
				AppendLabel(Buf, IDS_COLUMN6, MaxLines);

				DistanceToString(tmpStr, 256, pPair->value.DistanceNM);
				Buf.Append(tmpStr);
			}

			if ((m_Settings.NoteFlightTime) && (pPair->value.FlightTimeCount))
			{
				AppendLabel(Buf, IDS_COLUMN23, MaxLines);

				UINT FlightTime = pPair->value.FlightTime/pPair->value.FlightTimeCount;
				swprintf_s(tmpStr, 256, L"%02d:%02d", FlightTime/60, FlightTime%60);
				Buf.Append(tmpStr);
			}

			if (m_Settings.NoteFlightCount)
			{
				CString tmpMask(_T("%u"));
				if (MaxLines>1)
				{
					if (!Buf.IsEmpty())
						Buf.Append(_T("\n"));

					ENSURE(tmpMask.LoadString(pPair->value.Count==1 ? IDS_FLIGHTS_SINGULAR : IDS_FLIGHTS_PLURAL));
				}

				swprintf_s(tmpStr, 256, tmpMask, pPair->value.Count);
				Buf.Append(tmpStr);
			}

			if ((m_Settings.NoteCarrier) && (pPair->value.Carrier[0]!=L'\0') && (!pPair->value.CarrierMultiple))
			{
				AppendLabel(Buf, IDS_COLUMN7, MaxLines);
				Buf.Append(pPair->value.Carrier);
			}

			if ((m_Settings.NoteEquipment) && (pPair->value.Equipment[0]!=L'\0') && (!pPair->value.EquipmentMultiple))
			{
				AppendLabel(Buf, IDS_COLUMN10, MaxLines);
				Buf.Append(pPair->value.Equipment);
			}

			// Create path from string
			StringFormat strFormat;
			GraphicsPath TextPath;
			TextPath.AddString(Buf, Buf.GetLength(), &font, FontStyleRegular, (m_Settings.NoteSmallFont ? 14.0f : 16.0f)*Upscale, Gdiplus::Point(0, 0), &strFormat);

			Rect rectPath;
			TextPath.GetBounds(&rectPath);
			const REAL L = rectPath.Width+5.5f*Upscale;
			const REAL H = rectPath.Height+4.5f*Upscale;

			// Find suitable location
			Matrix m;

			CRect rectNote((INT)(S-L/2.0f), (INT)(Z-H/2.0f), (INT)(S+L/2.0f), (INT)(Z+H/2.0f));

			// Move label rectangle back into map boundaries
			if (rectNote.left<0)
				rectNote.OffsetRect(-rectNote.left, 0);

			if (rectNote.top<0)
				rectNote.OffsetRect(0, -rectNote.top);

			if (rectNote.right>=Width)
				rectNote.OffsetRect(-(rectNote.right-Width), 0);

			if (rectNote.bottom>=Height)
				rectNote.OffsetRect(0, -(rectNote.bottom-Height));

			if ((rectNote.left<0) || (rectNote.top<0) || (rectNote.right>=Width) || (rectNote.bottom>=Height))
				goto SkipNote;

			// Find intersections
			RECT rect;

			for (UINT a=0; a<AirportCount; a++)
			{
				if (IntersectRect(&rect, rectNote, &pAirportData[a].Spot))
					goto SkipNote;

				if (IntersectRect(&rect, rectNote, &pAirportData[a].IATA))
					goto SkipNote;
			}

			for (UINT a=0; a<CurRoute; a++)
				if (IntersectRect(&rect, rectNote, &RouteLabel[a]))
					goto SkipNote;

			// Draw annotation
			m.Reset();
			m.Translate((REAL)rectNote.left, (REAL)rectNote.top);
			TextPath.Transform(&m);

			if (m_Settings.NoteOuterColor!=(COLORREF)-1)
				g.DrawPath(&pen, &TextPath);

			g.FillPath(&brush, &TextPath);

			RouteLabel[CurRoute++] = rectNote;

SkipNote:
			continue;
		}

		delete[] RouteLabel;
	}

	// Final scale
	if (FinalScale<1.0f)
	{
		const LONG L = (LONG)(Width*FinalScale);
		const LONG H = (LONG)(Height*FinalScale);

		CBitmap* pBitmapScaled = CreateTruecolorBitmapObject(L, H);
		dc.SelectObject(pBitmapScaled);

		Graphics g(dc);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		Bitmap bmp((HBITMAP)pBitmap->m_hObject, NULL);
		g.DrawImage(&bmp, RectF(-0.5f, -0.5f, L+1.0f, H+1.0f), 0.0f, 0.0f, (REAL)Width, (REAL)Height, UnitPixel);

		delete pBitmap;
		pBitmap = pBitmapScaled;
	}

	// Deface
#ifndef _DEBUG
	if (!FMIsLicensed())
#endif
	{
		CSize sz = pBitmap->GetBitmapDimension();

		for (LONG Row=0; Row<sz.cy; Row+=48)
			dc.FillSolidRect(0, Row, sz.cx, 16, 0xFF00FF);
	}

	// Clean up GDI
	dc.SelectObject(pOldBitmap);

	// Finish
	delete pKitchen;
	delete pAirportData;

	if (ppRouteData)
	{
		for (UINT a=0; a<RouteCount; a++)
			free(ppRouteData[a]);

		delete ppRouteData;
	}

	return pBitmap;
}


void CMapFactory::AppendLabel(CString& Buf, UINT nID, UINT MaxLines)
{
	if (MaxLines>1)
	{
		if (!Buf.IsEmpty())
			Buf.Append(_T("\n"));

		Buf.Append(CString((LPCSTR)nID));
		Buf.Append(_T(": "));
	}
}

__forceinline CBitmap* CMapFactory::LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const
{
	ASSERT(Left>=0);
	ASSERT(Top>=0);
	ASSERT(Width>=0);
	ASSERT(Height>=0);
	ASSERT(Left+Width<=BGWIDTH);
	ASSERT(Top+Height<=BGHEIGHT);

	CBitmap* pBitmap = CreateTruecolorBitmapObject(Width, Height);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CBitmap* pOldBitmap = dc.SelectObject(pBitmap);

	if (m_Settings.Background>=3)
	{
		dc.FillSolidRect(0, 0, Width, Height, m_Settings.BackgroundColor);
	}
	else
	{
		Graphics g(dc);

		ImageAttributes ImgAttr;
		ImgAttr.SetWrapMode(WrapModeTile);

		static const UINT ResID[3] = { IDB_BLUEMARBLE_8192, IDB_NIGHT_8192, IDB_ABSTRACT_8192 };
		g.DrawImage(theApp.GetCachedResourceImage(ResID[m_Settings.Background]), Rect(0, 0, Width, Height), Left-MapOffset, Top, Right-Left+1, Bottom-Top+1, UnitPixel, &ImgAttr);
	}

	dc.SelectObject(pOldBitmap);

	return pBitmap;
}

void CMapFactory::DrawLine(Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, INT MinS, INT MinZ, REAL Scale, REAL* pLabelZ, REAL* pLabelS) const
{
// Draw line multiple times when possible to avoid jitter due to anti-aliasing
#define Line(pen, x1, y1, x2, y2) \
	g.DrawLine(&pen, x1, y1, x2, y2); \
	if (!m_Settings.UseCountOpacity) \
	{ \
		g.DrawLine(&pen, x1+1.0f, y1, x2+1.0f, y2); \
		g.DrawLine(&pen, x1, y1+1.0f, x2, y2+1.0f); \
	}

	x1 -= MinS;
	x2 -= MinS;
	y1 -= MinZ;
	y2 -= MinZ;

	if ((x1<WRAPMARGIN-MinS) && (x2>BGWIDTH-WRAPMARGIN-MinS))
	{
		// When there is a wrap-around, we do NOT need to divide by Scale, as it's 1.0f anyway
		Line(pen, x1, y1, x2-BGWIDTH, y2);
		Line(pen, x1+BGWIDTH, y1, x2, y2);

		if (pLabelZ)
			*pLabelZ = -1.0f;

		if (pLabelS)
			*pLabelS = -1.0f;
	}
	else
		if ((x1>BGWIDTH-WRAPMARGIN-MinS) && (x2<WRAPMARGIN-MinS))
		{
			// When there is a wrap-around, we do NOT need to divide by Scale, as it's 1.0f anyway
			Line(pen, x1-BGWIDTH, y1, x2, y2);
			Line(pen, x1, y1, x2+BGWIDTH, y2);

			if (pLabelZ)
				*pLabelZ = -1.0f;

			if (pLabelS)
				*pLabelS = -1.0f;
		}
		else
		{
			// All coordinates need to be scaled
			Line(pen, x1/Scale, y1/Scale, x2/Scale, y2/Scale);

			if (pLabelZ)
				*pLabelZ = (y1+y2)/(2.0f*Scale);

			if (pLabelS)
				*pLabelS = (x1+x2)/(2.0f*Scale);
		}
}

void CMapFactory::DrawArrow(Graphics& g, Brush& brush, REAL x1, REAL y1, REAL x2, REAL y2, INT MinS, INT MinZ, REAL Scale, REAL Upscale)
{
	if ((x1==x2) && (y1==y2))
		return;

	const REAL Angle = atan2(y2-y1, x2-x1);
	x1 = (x1-MinS)/Scale;
	y1 = (y1-MinZ)/Scale;

	PointF points[3];
	points[0] = PointF(4.0f*Upscale*cos(Angle)+x1, 4.0f*Upscale*sin(Angle)+y1);
	points[1] = PointF(16.0f*Upscale*cos(Angle+PI/7.0f)+x1, 16.0f*Upscale*sin(Angle+PI/7)+y1);
	points[2] = PointF(16.0f*Upscale*cos(Angle-PI/7.0f)+x1, 16.0f*Upscale*sin(Angle-PI/7)+y1);

	g.FillPolygon(&brush, points, 3);
}
