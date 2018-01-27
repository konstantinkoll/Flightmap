
// CMapFactory.cpp: Implementierung der Klasse CMapFactory
//

#include "stdafx.h"
#include "CMapFactory.h"
#include "Flightmap.h"
#include <math.h>


// CMapFactory
//

#define MASKCLR_EMPTY       0xFFFFFF
#define MASKCLR_AIRPORT     0x0000DD
#define MASKCLR_LABEL       0x0066FF
#define MASKCLR_ROUTE       0x000000

#define MASK_FORBIDDEN      (INT)0x80000000

#define AIRPORTMARGIN       4
#define LABELMARGIN         2
#define WRAPMARGIN          2500


CMapFactory::CMapFactory(const MapSettings& Settings)
{
	m_Settings = Settings;
}

void CMapFactory::AppendLabel(CString& strNote, UINT nID, UINT MaxLines)
{
	if (MaxLines>1)
	{
		if (!strNote.IsEmpty())
			strNote.Append(_T("\n"));

		strNote.Append(CString((LPCSTR)nID));
		strNote.Append(_T(": "));
	}
}

inline CBitmap* CMapFactory::LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const
{
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

COLORREF CMapFactory::PreparePen(Pen& pen, const FlightRoute& Route, const CKitchen* pKitchen, DOUBLE GfxScale) const
{
	const COLORREF clr = ((Route.Color==(COLORREF)-1) || !m_Settings.UseColors) ? m_Settings.RouteColor : Route.Color;

	const BYTE Alpha = (m_Settings.UseCountOpacity && (pKitchen->m_MaxRouteCount!=0)) ? 0x60+(BYTE)(159.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1))) : 0xFF;
	pen.SetColor(Color(COLORREF2ARGB(clr, Alpha)));

	const DOUBLE Width = (m_Settings.UseCountWidth && (pKitchen->m_MaxRouteCount!=0)) ? (0.2+(3.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1)))) : 3.2; \
	pen.SetWidth((REAL)(Width*GfxScale));

	return clr;
}

void CMapFactory::DrawLine(CDC& dcMask, Graphics& g, const Pen& pen, const DRAWPOINT& ptA, const DRAWPOINT& ptB) const
{
	// Draw line multiple times on map when possible to avoid jitter due to anti-aliasing
	g.DrawLine(&pen, ptA.x, ptA.y, ptB.x, ptB.y);

	if (!m_Settings.UseCountOpacity)
	{
		g.DrawLine(&pen, ptA.x+0.5f, ptA.y, ptB.x+0.5f, ptB.y);
		g.DrawLine(&pen, ptA.x, ptA.y+0.5f, ptB.x, ptB.y+0.5f);
	}

	// Draw on mask
	dcMask.MoveTo(PIXELPOINT(ptA));
	dcMask.LineTo(PIXELPOINT(ptB));
}

void CMapFactory::DrawLine(CDC& dcMask, Graphics& g, const Pen& pen, const MAPPOINT& ptA, const MAPPOINT& ptB, REAL Scale, RENDERPOINT* pptLabel) const
{
	if ((ptA.x<WRAPMARGIN) && (ptB.x>BGWIDTH-WRAPMARGIN))
	{
		DrawLine(dcMask, g, pen, DRAWPOINT(ptA, Scale), DRAWPOINT(ptB, -BGWIDTH, Scale));
		DrawLine(dcMask, g, pen, DRAWPOINT(ptA, BGWIDTH, Scale), DRAWPOINT(ptB, Scale));

		if (pptLabel)
			pptLabel->x = pptLabel->y = -1.0f;
	}
	else
		if ((ptA.x>BGWIDTH-WRAPMARGIN) && (ptB.x<WRAPMARGIN))
		{
			DrawLine(dcMask, g, pen, DRAWPOINT(ptA, -BGWIDTH, Scale), DRAWPOINT(ptB, Scale));
			DrawLine(dcMask, g, pen, DRAWPOINT(ptA, Scale), DRAWPOINT(ptB, BGWIDTH, Scale));

			if (pptLabel)
				pptLabel->x = pptLabel->y = -1.0f;
		}
		else
		{
			const DRAWPOINT ptDrawA = DRAWPOINT(ptA, Scale);
			const DRAWPOINT ptDrawB = DRAWPOINT(ptB, Scale);

			DrawLine(dcMask, g, pen, ptDrawA, ptDrawB);

			if (pptLabel)
			{
				pptLabel->x = (ptDrawA.x+ptDrawB.x)/2.0f;
				pptLabel->y = (ptDrawA.y+ptDrawB.y)/2.0f;
			}
		}
}

void CMapFactory::DrawArrow(Graphics& g, const Brush& brush, const DRAWPOINT& ptA, const DRAWPOINT& ptB, REAL GfxScale)
{
	if (ptA==ptB)
		return;

	const REAL Angle = atan2(ptB.y-ptA.y, ptB.x-ptA.x);

	const PointF points[3] = {
		PointF(4.0f*GfxScale*cos(Angle)+ptA.x, 4.0f*GfxScale*sin(Angle)+ptA.y),
		PointF(16.0f*GfxScale*cos(Angle+PI/7.0f)+ptA.x, 16.0f*GfxScale*sin(Angle+PI/7)+ptA.y),
		PointF(16.0f*GfxScale*cos(Angle-PI/7.0f)+ptA.x, 16.0f*GfxScale*sin(Angle-PI/7)+ptA.y)
	};

	g.FillPolygon(&brush, points, 3);
}

INT CMapFactory::ScanMask(const CDC& dc, CRect& rect, UINT Border, INT Width, INT Height)
{
	// Move rectangle back into map boundaries
	if (rect.left<(INT)Border)
	{
		rect.OffsetRect(-rect.left+Border, 0);

		if (rect.right>=Width-(INT)Border)
			return MASK_FORBIDDEN;
	}

	if (rect.top<(INT)Border)
	{
		rect.OffsetRect(0, -rect.top+Border);

		if (rect.bottom>=Height-(INT)Border)
			return MASK_FORBIDDEN;
	}

	if (rect.right>=Width-(INT)Border)
	{
		rect.OffsetRect(-(rect.right-Width+(INT)Border), 0);

		if (rect.left<(INT)Border)
			return MASK_FORBIDDEN;
	}

	if (rect.bottom>=Height-(INT)Border)
	{
		rect.OffsetRect(0, -(rect.bottom-Height+(INT)Border));

		if (rect.top<(INT)Border)
			return MASK_FORBIDDEN;
	}

	// Actually pixel-scan mask bitmap
	INT Score = 0;

	for (INT y=rect.top; y<rect.bottom; y++)
		for (INT x=rect.left; x<rect.right; x++)
			switch (dc.GetPixel(x, y))
			{
			case MASKCLR_ROUTE:
				Score--;

			case MASKCLR_EMPTY:
				break;

			default:
				return MASK_FORBIDDEN;
			}

	return Score;
}

CBitmap* CMapFactory::RenderMap(CKitchen* pKitchen)
{
	ASSERT(pKitchen);

	AirportList* pAirportList = pKitchen->GetAirports();
	RouteList* pRouteList = pKitchen->GetRoutes(!m_Settings.StraightLines);

	// Compute map boundaries and scale
	//

	// Initialize
	INT MinY = BGHEIGHT;
	INT MaxY = 0;
	INT MinX = BGWIDTH;
	INT MaxX = 0;

	const INT MapOffset = m_Settings.CenterPacific ? 4700 : 0;
	REAL BackgroundScale = 1.0f;

	// Convert airports' geocoordinates to map coordinates, and compute background boundaries
	for (UINT a=0; a<pAirportList->m_ItemCount; a++)
	{
		FlightAirport* pAirport = &((*pAirportList)[a]);

		// Map coordinates
		pAirport->Point = BACKGROUNDPOINT(pAirport->lpcAirport, MapOffset);

		// Background boundaries
		const INT X = (INT)(pAirport->Point.x+0.5f);
		if (X<MinX)
			MinX =X;
		if (X>MaxX)
			MaxX = X;

		const INT Y = (INT)(pAirport->Point.y+0.5f);
		if (Y<MinY)
			MinY =Y;
		if (Y>MaxY)
			MaxY = Y;
	}

	// Compute wrap-around at the map edges
	BOOL WrapAround = FALSE;

	if (m_Settings.ShowRoutes)
		for (UINT a=0; a<pRouteList->m_ItemCount; a++)
		{
			FlightRoute* pRoute = &((*pRouteList)[a]);

			if (m_Settings.StraightLines)
			{
				// Check single line for wrap-around
				const BACKGROUNDPOINT ptFrom = BACKGROUNDPOINT(pRoute->lpcFrom, MapOffset);
				const BACKGROUNDPOINT ptTo = BACKGROUNDPOINT(pRoute->lpcTo, MapOffset);

				WrapAround |= ((ptFrom.x<WRAPMARGIN) && (ptTo.x>BGWIDTH-WRAPMARGIN)) || ((ptFrom.x>BGWIDTH-WRAPMARGIN) && (ptTo.x<WRAPMARGIN));
			}
			else
			{
				const DOUBLE Offs = (2.0*PI*MapOffset)/BGWIDTH;

				// Check individual segments for wrap-around, and adjust tesselated route for map offset
				FlightSegments* pSegments = pRoute->pSegments;
				ASSERT(pSegments);

				if ((pSegments->Points[0][1]+=Offs)>=PI)
					pSegments->Points[0][1] -= 2*PI;

				for (UINT Pt=1; Pt<pSegments->PointCount; Pt++)
				{
					if ((pSegments->Points[Pt][1]+=Offs)>=PI)
						pSegments->Points[Pt][1] -= 2*PI;

					WrapAround |= ((pSegments->Points[Pt-1][1]<-2.0) && (pSegments->Points[Pt][1]>2.0)) ||
						((pSegments->Points[Pt-1][1]>2.0) && (pSegments->Points[Pt][1]<-2.0));
				}
			}
		}

	// When there are no airports or routes wrap-around the map edges, force maximum width
	if (!pAirportList->m_ItemCount || WrapAround)
	{
		MinX = 0;
		MaxX = BGWIDTH-1;
	}

	// Add borders to boundaries
	UINT GfxBorder = min(100, max(m_Settings.Width, m_Settings.Height)/16);

	if (m_Settings.WideBorder)
		GfxBorder *= 2;

	const UINT LabelBorder = GfxBorder/2;

	// Compute true boundaries and background scale
	if (((m_Settings.Width<BGWIDTH) || (m_Settings.Height<BGHEIGHT)) && (MaxY>0))
	{
		const UINT Border = (MinX>0) && (MaxX<BGWIDTH-1) ? GfxBorder : 0;
		const INT X = (MinX+MaxX)/2;
		const INT Y = (MinY+MaxY)/2;
		const REAL ScX = (REAL)(m_Settings.Width-2*Border)/(REAL)(MaxX-MinX+1);
		const REAL ScY = (REAL)(m_Settings.Height-2*Border)/(REAL)(MaxY-MinY+1);

		BackgroundScale = min(ScX, ScY);
		if (BackgroundScale>2.0f)
			BackgroundScale = 2.0f;

		MinX = X-(INT)(m_Settings.Width/(BackgroundScale*2.0f));
		MaxX = X+(INT)(m_Settings.Width/(BackgroundScale*2.0f))-1;
		MinY = Y-(INT)(m_Settings.Height/(BackgroundScale*2.0f));
		MaxY = Y+(INT)(m_Settings.Height/(BackgroundScale*2.0f))-1;

		if (MinY<0)
		{
			MaxY -= MinY;
			MinY = 0;
		}

		if (MaxY>BGHEIGHT-1)
		{
			if (MinY>MaxY-BGHEIGHT+1)
				MinY -= MaxY-BGHEIGHT+1;

			MaxY = BGHEIGHT-1;
		}
	}
	else
	{
		MinY = MinX = 0;
		MaxY = BGHEIGHT-1;
		MaxX = BGWIDTH-1;
	}

	// Background
	const INT Width = (INT)((MaxX-MinX+1)*BackgroundScale+0.5f);
	const INT Height = (INT)((MaxY-MinY+1)*BackgroundScale+0.5f);
	CBitmap* pMapBitmap = LoadBackground(MinX, MinY, MaxX, MaxY, Width, Height, MapOffset);

	// Graphics scale
	const REAL GfxScale = (m_Settings.ForegroundScale==0) ? 0.65f+0.35f*BackgroundScale : (m_Settings.ForegroundScale==2) ? 2.0f : 1.0f;


	// Draw map
	//

	// Obtain device context and graphics surface for map
	CDC dcMap;
	dcMap.CreateCompatibleDC(NULL);

	CBitmap* pOldMapBitmap = dcMap.SelectObject(pMapBitmap);

	Graphics g(dcMap);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(theApp.m_SmoothingModeAntiAlias8x8);

	// Obtain device context and bitmap for mask
	CDC dcMask;
	dcMask.CreateCompatibleDC(NULL);
	dcMask.SelectStockObject(DC_BRUSH);

	HBITMAP hMaskBitmap = CreateMaskBitmap(Width, Height);
	HBITMAP hOldMaskBitmap = (HBITMAP)dcMask.SelectObject(hMaskBitmap);

	// Draw routes
	if (m_Settings.ShowRoutes && pRouteList->m_ItemCount)
	{
		// Map pen
		Pen pen(Color(0x00000000));

		// Mask pen
		HPEN hPen = CreatePen(PS_SOLID, 7, MASKCLR_ROUTE);
		HPEN hOldPen = (HPEN)dcMask.SelectObject(hPen);

		for (UINT a=0; a<pRouteList->m_ItemCount; a++)
		{
			FlightRoute* pRoute = &((*pRouteList)[a]);

			const COLORREF clr = PreparePen(pen, *pRoute, pKitchen, GfxScale);

			if (m_Settings.StraightLines)
			{
				// Draw straight route and compute annotation position
				const BOOL UseWaypoint = (pRoute->lpcFrom==pRoute->lpcTo) && (pRoute->Waypoint.Latitude || pRoute->Waypoint.Longitude);

				const MAPPOINT ptFrom = MAPPOINT(BACKGROUNDPOINT(pRoute->lpcFrom, MapOffset), MinX, MinY);
				const MAPPOINT ptTo = MAPPOINT(BACKGROUNDPOINT(pRoute->lpcTo, MapOffset), MinX, MinY);

				DrawLine(dcMask, g, pen, ptFrom, ptTo, BackgroundScale, &pRoute->ptLabel);

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity && !UseWaypoint)
				{
					SolidBrush brush(Color(COLORREF2RGB(clr)));

					if (pRoute->Arrows & ARROW_FT)
						DrawArrow(g, brush, DRAWPOINT(ptTo, BackgroundScale), DRAWPOINT(ptFrom, BackgroundScale), GfxScale);

					if (pRoute->Arrows & ARROW_TF)
						DrawArrow(g, brush, DRAWPOINT(ptFrom, BackgroundScale), DRAWPOINT(ptTo, BackgroundScale), GfxScale);
				}
			}
			else
			{
				const FlightSegments* pSegments = pRoute->pSegments;

				// Draw tesselated route
				MAPPOINT ptLast = MAPPOINT(&pSegments->Points[0][0], MinX, MinY);

				for (UINT Pt=1; Pt<pSegments->PointCount; Pt++)
				{
					const MAPPOINT ptCurrent = MAPPOINT(&pSegments->Points[Pt][0], MinX, MinY);

					DrawLine(dcMask, g, pen, ptLast, ptCurrent, BackgroundScale);
					ptLast = ptCurrent;
				}

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity)
				{
					SolidBrush brush(Color(COLORREF2RGB(clr)));

					if (pRoute->Arrows & ARROW_FT)
						DrawArrow(g, brush,
							DRAWPOINT(MAPPOINT(&pSegments->Points[pSegments->PointCount-1][0], MinX, MinY), BackgroundScale),
							DRAWPOINT(MAPPOINT(&pSegments->Points[pSegments->PointCount-2][0], MinX, MinY), BackgroundScale),
							GfxScale);

					if (pRoute->Arrows & ARROW_TF)
						DrawArrow(g, brush,
							DRAWPOINT(MAPPOINT(&pSegments->Points[0][0], MinX, MinY), BackgroundScale),
							DRAWPOINT(MAPPOINT(&pSegments->Points[1][0], MinX, MinY), BackgroundScale),
							GfxScale);
				}

				// Compute annotation position
				pRoute->ptLabel = DRAWPOINT(MAPPOINT(&pSegments->Points[pSegments->PointCount/2][0], MinX, MinY), BackgroundScale);
			}
		}

		dcMask.SelectObject(hOldPen);
		DeleteObject(hPen);
	}

	// Draw locations
	const REAL LocationRadius = 5.5f*GfxScale;

	if (m_Settings.ShowLocations)
	{
		// Map pen and brush
		Pen pen(Color(COLORREF2RGB(m_Settings.LocationsOuterColor)), 2.0f*GfxScale);
		SolidBrush brush(Color(COLORREF2RGB(m_Settings.LocationsInnerColor)));

		// Mask pen and brush
		dcMask.SetDCBrushColor(MASKCLR_AIRPORT);
		dcMask.SetDCPenColor(MASKCLR_AIRPORT);

		const INT LocationDiameter = (INT)(LocationRadius*2.0f);

		for (UINT a=0; a<pAirportList->m_ItemCount; a++)
		{
			const FlightAirport* pAirport = &((*pAirportList)[a]);
			const PIXELPOINT ptAirport = PIXELPOINT(DRAWPOINT(MAPPOINT(*((BACKGROUNDPOINT*)&pAirport->Point), MinX, MinY), BackgroundScale), -LocationRadius);

			// Draw on map
			g.FillEllipse(&brush, ptAirport.x, ptAirport.y, LocationDiameter, LocationDiameter);

			if (m_Settings.LocationsOuterColor!=(COLORREF)-1)
				g.DrawEllipse(&pen, ptAirport.x, ptAirport.y, LocationDiameter, LocationDiameter);

			// Draw on mask
			dcMask.Ellipse(CRect(ptAirport.x-AIRPORTMARGIN, ptAirport.y-AIRPORTMARGIN, ptAirport.x+LocationDiameter+AIRPORTMARGIN, ptAirport.y+LocationDiameter+AIRPORTMARGIN));
		}
	}

	// Draw IATA codes
	const INT LabelMargin = (INT)(LABELMARGIN*GfxScale);

	if (m_Settings.ShowIATACodes)
	{
		// Font
		FontFamily font(_T("Arial"));
		const REAL FontSize = 15.4f*GfxScale;
		
		StringFormat strFormat;
		GraphicsPath TextPath;
		TextPath.AddString(L"W", 1, &font, FontStyleRegular, FontSize, Point(0, 0), &strFormat);

		RectF rectPath;
		TextPath.GetBounds(&rectPath);

		const INT LabelHeight = (INT)(rectPath.Height+0.5f);
		const INT PathX = (INT)(rectPath.X+0.5);
		const INT PathY = (INT)(rectPath.Y+0.5);

		// Map pen and brush
		Pen pen(Color(COLORREF2ARGB(m_Settings.IATACodesOuterColor, (m_Settings.IATACodesOuterColor || (m_Settings.ForegroundScale==2)) ? 0xFF : 0xCC)), 2.0f*GfxScale+1.0f);
		pen.SetLineJoin(LineJoinRound);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.IATACodesInnerColor)));

		for (UINT a=0; a<pAirportList->m_ItemCount; a++)
		{
			const FlightAirport* pAirport = &((*pAirportList)[a]);
			PIXELPOINT ptLabel = PIXELPOINT(DRAWPOINT(MAPPOINT(*((BACKGROUNDPOINT*)&pAirport->Point), MinX, MinY), BackgroundScale), -LocationRadius+(INT)LocationRadius);

			// Prepare path
			WCHAR pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, pAirport->lpcAirport->Code, -1, pszBuf, 4);

			TextPath.Reset();
			TextPath.AddString(pszBuf, (INT)wcslen(pszBuf), &font, FontStyleRegular, 15.4f*GfxScale, Point(0, 0), &strFormat);

			TextPath.GetBounds(&rectPath);
			const INT LabelWidth = (INT)(rectPath.Width+0.5f);

			// Find suitable location
			CRect rectBestLabel;

			INT BestScore = MASK_FORBIDDEN;
			ASSERT(BestScore<0);

			const POINT LabelOffsets[8] = {
				{ (INT)(LocationRadius*2.0f+1.0f), -LabelHeight/2 },
				{ -LabelWidth-(INT)(LocationRadius*2.0f), -LabelHeight/2 },
				{ -LabelWidth/2, (INT)(LocationRadius*2.0f+1.0f) },
				{ -LabelWidth/2, -LabelHeight-(INT)(LocationRadius*2.0f) },
				{ -LabelWidth-(INT)(LocationRadius*1.5f+1.0f), (INT)(LocationRadius*1.5f+0.5f) },
				{ -LabelWidth-(INT)(LocationRadius*1.5f+1.0f), -LabelHeight-(INT)(LocationRadius*1.5f+0.5f) },
				{ (INT)(LocationRadius*1.5f+1.0f), (INT)(LocationRadius*1.5f+0.5f) },
				{ (INT)(LocationRadius*1.5f+1.0f), -LabelHeight-(INT)(LocationRadius*1.5f+0.5f) },
			};

			for (UINT a=0; a<8; a++)
			{
				CRect rectLabel(ptLabel.x+LabelOffsets[a].x-LabelMargin, ptLabel.y+LabelOffsets[a].y-LabelMargin, ptLabel.x+LabelOffsets[a].x+LabelWidth+LabelMargin, ptLabel.y+LabelOffsets[a].y+LabelHeight+LabelMargin);

				// Find intersections
				const INT Score = ScanMask(dcMask, rectLabel, LabelBorder, Width, Height);
				if (Score>BestScore)
				{
					rectBestLabel = rectLabel;
					BestScore = Score;
				}
			}

			if (BestScore>MASK_FORBIDDEN)
			{
				// Draw label on map
				Matrix m;
				m.Translate((REAL)(rectBestLabel.left-PathX+LabelMargin), (REAL)(rectBestLabel.top-PathY+LabelMargin));
				TextPath.Transform(&m);

				if (m_Settings.IATACodesOuterColor!=(COLORREF)-1)
					g.DrawPath(&pen, &TextPath);

				g.FillPath(&brush, &TextPath);

				// Draw label on mask
				dcMask.FillSolidRect(rectBestLabel, MASKCLR_LABEL);
			}
		}
	}

	// Draw annotations
	if (m_Settings.ShowRoutes && ((m_Settings.NoteDistance || m_Settings.NoteFlightCount || m_Settings.NoteFlightTime || m_Settings.NoteCarrier || m_Settings.NoteEquipment)))
	{
		// Font
		FontFamily Font(m_Settings.NoteSmallFont ? _T("Tahoma") : _T("Arial"));
		const REAL FontSize = (m_Settings.NoteSmallFont ? 14.0f : 15.5f)*GfxScale;

		// Map pen and brush
		Pen pen(Color(COLORREF2ARGB(m_Settings.IATACodesOuterColor, (m_Settings.NoteOuterColor || (m_Settings.ForegroundScale==2)) ? 0xFF : 0xCC)), (m_Settings.NoteSmallFont ? 1.75f : 2.0f)*GfxScale+1.0f);
		pen.SetLineJoin(LineJoinRound);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.NoteInnerColor)));

		const UINT MaxLines = (m_Settings.NoteDistance ? 1 : 0)+(m_Settings.NoteFlightCount ? 1 : 0)+(m_Settings.NoteFlightTime ? 1 : 0)+(m_Settings.NoteCarrier ? 1 : 0)+(m_Settings.NoteEquipment ? 1 : 0);

		for (UINT a=0; a<pRouteList->m_ItemCount; a++)
		{
			FlightRoute* pRoute = &((*pRouteList)[a]);

			// Skip if there is no anchor point
			if ((pRoute->ptLabel.x==-1.0f) || (pRoute->ptLabel.y==-1.0f))
				continue;

			PIXELPOINT ptLabel = PIXELPOINT(*((DRAWPOINT*)&pRoute->ptLabel));

			// Create annotation text
			CString strNote;
			WCHAR tmpStr[256];

			// Distance
			if (m_Settings.NoteDistance && (pRoute->DistanceNM!=0.0))
			{
				AppendLabel(strNote, IDS_COLUMN6, MaxLines);

				DistanceToString(tmpStr, 256, pRoute->DistanceNM);
				strNote.Append(tmpStr);
			}

			// Flight time
			if (m_Settings.NoteFlightTime && pRoute->FlightTimeCount)
			{
				AppendLabel(strNote, IDS_COLUMN23, MaxLines);

				const UINT FlightTime =pRoute->FlightTime/pRoute->FlightTimeCount;

				swprintf_s(tmpStr, 256, L"%02u:%02u", FlightTime/60, FlightTime%60);
				strNote.Append(tmpStr);
			}

			// Flight count
			if (m_Settings.NoteFlightCount)
			{
				CString tmpMask(_T("%u"));
				if (MaxLines>1)
				{
					if (!strNote.IsEmpty())
						strNote.Append(_T("\n"));

					ENSURE(tmpMask.LoadString(pRoute->Count==1 ? IDS_FLIGHTS_SINGULAR : IDS_FLIGHTS_PLURAL));
				}

				swprintf_s(tmpStr, 256, tmpMask, pRoute->Count);
				strNote.Append(tmpStr);
			}

			// Carrier
			if (m_Settings.NoteCarrier && (pRoute->Carrier[0]!=L'\0') && !pRoute->CarrierMultiple)
			{
				AppendLabel(strNote, IDS_COLUMN7, MaxLines);
				strNote.Append(pRoute->Carrier);
			}

			// Equipment
			if (m_Settings.NoteEquipment && (pRoute->Equipment[0]!=L'\0') && !pRoute->EquipmentMultiple)
			{
				AppendLabel(strNote, IDS_COLUMN10, MaxLines);
				strNote.Append(pRoute->Equipment);
			}

			// Create path from string
			StringFormat strFormat;
			GraphicsPath TextPath;
			TextPath.AddString(strNote, strNote.GetLength(), &Font, FontStyleRegular, FontSize, Gdiplus::Point(0, 0), &strFormat);

			RectF rectPath;
			TextPath.GetBounds(&rectPath);

			const INT NoteW = (INT)(rectPath.Width+0.5f);
			const INT NoteH = (INT)(rectPath.Height+0.5f);

			// Find suitable location
			CRect rectNote(ptLabel.x-NoteW/2, ptLabel.y-NoteH/2, ptLabel.x+NoteW/2, ptLabel.y+NoteH/2);

			// Find intersections
			if (ScanMask(dcMask, rectNote, LabelBorder, Width, Height)!=MASK_FORBIDDEN)
			{
				// Draw annotation on map
				Matrix m;
				m.Translate((REAL)(rectNote.left-(INT)rectPath.X), (REAL)(rectNote.top-(INT)rectPath.Y));
				TextPath.Transform(&m);

				if (m_Settings.NoteOuterColor!=(COLORREF)-1)
					g.DrawPath(&pen, &TextPath);

				g.FillPath(&brush, &TextPath);

				// Draw annotation on mask
				dcMask.FillSolidRect(rectNote, MASKCLR_LABEL);
			}
		}
	}

	// Deface map if not licensed
	if (!FMIsLicensed())
	{
		const CSize Size = pMapBitmap->GetBitmapDimension();

		for (LONG Row=0; Row<Size.cy; Row+=48)
			dcMap.FillSolidRect(0, Row, Size.cx, 16, 0xFF00FF);
	}

	// Clean up GDI
	dcMap.SelectObject(pOldMapBitmap);
	
	dcMask.SelectObject(hOldMaskBitmap);
	DeleteObject(hMaskBitmap);

	// Finish
	delete pKitchen;

	return pMapBitmap;
}
