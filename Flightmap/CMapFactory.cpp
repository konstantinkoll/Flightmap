
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

struct FactoryAirportData
{
	REAL X;
	REAL Y;
};

CMapFactory::CMapFactory(MapSettings* pSettings)
{
	m_Settings = *pSettings;
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

__forceinline CBitmap* CMapFactory::LoadBackground(INT Left, INT Top, INT Right, INT Bottom, INT Width, INT Height, INT MapOffset) const
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
	COLORREF clr = ((Route.Color==(COLORREF)-1) || !m_Settings.UseColors) ? m_Settings.RouteColor : Route.Color;

	const BYTE Alpha = (m_Settings.UseCountOpacity && (pKitchen->m_MaxRouteCount!=0)) ? 0x60+(BYTE)(159.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1))) : 0xFF;
	pen.SetColor(Color(COLORREF2ARGB(clr, Alpha)));

	const DOUBLE Width = (m_Settings.UseCountWidth && (pKitchen->m_MaxRouteCount!=0)) ? (0.2+(3.0*((DOUBLE)(Route.Count-pKitchen->m_MinRouteCount))/((DOUBLE)(pKitchen->m_MaxRouteCount-pKitchen->m_MinRouteCount+1)))) : 3.2; \
	pen.SetWidth((REAL)(Width*GfxScale));

	return clr;
}

void CMapFactory::DrawLine(CDC& dcMask, Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale) const
{
	x1 *= Scale;
	y1 *= Scale;
	x2 *= Scale;
	y2 *= Scale;

	// Draw line multiple times on map when possible to avoid jitter due to anti-aliasing
	g.DrawLine(&pen, x1, y1, x2, y2);

	if (!m_Settings.UseCountOpacity)
	{
		g.DrawLine(&pen, x1+0.5f, y1, x2+0.5f, y2);
		g.DrawLine(&pen, x1, y1+0.5f, x2, y2+0.5f);
	}

	// Draw on mask
	dcMask.MoveTo((INT)x1, (INT)y1);
	dcMask.LineTo((INT)x2, (INT)y2);
}

void CMapFactory::DrawLine(CDC& dcMask, Graphics& g, Pen& pen, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale, INT MinS, INT MinZ, REAL* pLabelX, REAL* pLabelY) const
{
	x1 -= MinS;
	x2 -= MinS;
	y1 -= MinZ;
	y2 -= MinZ;

		// All coordinates need to be scaled
	if ((x1<WRAPMARGIN-MinS) && (x2>BGWIDTH-WRAPMARGIN-MinS))
	{
		DrawLine(dcMask, g, pen, x1, y1, x2-BGWIDTH, y2, Scale);
		DrawLine(dcMask, g, pen, x1+BGWIDTH, y1, x2, y2, Scale);

		if (pLabelX)
			*pLabelX = -1.0f;

		if (pLabelY)
			*pLabelY = -1.0f;
	}
	else
		if ((x1>BGWIDTH-WRAPMARGIN-MinS) && (x2<WRAPMARGIN-MinS))
		{
			DrawLine(dcMask, g, pen, x1-BGWIDTH, y1, x2, y2, Scale);
			DrawLine(dcMask, g, pen, x1, y1, x2+BGWIDTH, y2, Scale);

			if (pLabelX)
				*pLabelX = -1.0f;

			if (pLabelY)
				*pLabelY = -1.0f;
		}
		else
		{
			DrawLine(dcMask, g, pen, x1, y1, x2, y2, Scale);

			if (pLabelX)
				*pLabelX = (x1+x2)/2.0f;

			if (pLabelY)
				*pLabelY = (y1+y2)/2.0f;
		}
}

void CMapFactory::DrawArrow(Graphics& g, Brush& brush, REAL x1, REAL y1, REAL x2, REAL y2, REAL Scale, REAL GfxScale, INT MinS, INT MinZ)
{
	if ((x1==x2) && (y1==y2))
		return;

	const REAL Angle = atan2(y2-y1, x2-x1);
	x1 = (x1-MinS)*Scale;
	y1 = (y1-MinZ)*Scale;

	const PointF points[3] = {
		PointF(4.0f*GfxScale*cos(Angle)+x1, 4.0f*GfxScale*sin(Angle)+y1),
		PointF(16.0f*GfxScale*cos(Angle+PI/7.0f)+x1, 16.0f*GfxScale*sin(Angle+PI/7)+y1),
		PointF(16.0f*GfxScale*cos(Angle-PI/7.0f)+x1, 16.0f*GfxScale*sin(Angle-PI/7)+y1)
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

	// Compute map boundaries and scale
	//

	// Initialize
	INT MinZ = BGHEIGHT;
	INT MaxZ = 0;
	INT MinS = BGWIDTH;
	INT MaxS = 0;
	const INT MapOffset = m_Settings.CenterPacific ? 4700 : 0;
	REAL BackgroundScale = 1.0f;

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
		FactoryAirportData Data = {
			((REAL)pPair->value.pAirport->Location.Longitude*BGWIDTH)/360.0f+(MapOffset+BGWIDTH/2), 
			((REAL)pPair->value.pAirport->Location.Latitude*BGHEIGHT)/180.0f+BGHEIGHT/2
		};

		if (Data.X>=BGWIDTH)
			Data.X -= BGWIDTH;

		// Background boundaries
		const INT S = (INT)(Data.X+0.5f);
		if (S<MinS)
			MinS =S;
		if (S>MaxS)
			MaxS = S;

		const INT Z = (INT)(Data.Y+0.5f);
		if (Z<MinZ)
			MinZ =Z;
		if (Z>MaxZ)
			MaxZ = Z;

		pPair->value.lpAirportData = &(pAirportData[CurAirport++]=Data);
	}

	// Tesselate routes when required, and compute wrap-around at the map edges
	BOOL WrapAround = FALSE;

	if (m_Settings.ShowRoutes && (RouteCount>0))
	{
		const DOUBLE Offs = (2.0*PI*MapOffset)/BGWIDTH;

		UINT CurRoute = 0;
		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			// Match routes with airport data
			FlightAirport Airport;

			VERIFY(pKitchen->m_FlightAirports.Lookup(pPair->value.pFrom->Code, Airport));
			const FactoryAirportData* pFrom = (FactoryAirportData*)(pPair->value.lpFrom = (FactoryAirportData*)Airport.lpAirportData);

			VERIFY(pKitchen->m_FlightAirports.Lookup(pPair->value.pTo->Code, Airport));
			const FactoryAirportData* pTo = (FactoryAirportData*)(pPair->value.lpTo = (FactoryAirportData*)Airport.lpAirportData);

			if (m_Settings.StraightLines)
			{
				// Straight lines
				WrapAround |= ((pFrom->X<WRAPMARGIN) && (pTo->X>BGWIDTH-WRAPMARGIN)) || ((pFrom->Y>BGWIDTH-WRAPMARGIN) && (pTo->Y<WRAPMARGIN));
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
	}

	// When there are no airports or routes wrap-around the map edges, force maximum width
	if (!AirportCount || WrapAround)
	{
		MinS = 0;
		MaxS = BGWIDTH-1;
	}

	// Add borders to boundaries
	UINT GfxBorder = min(100, max(m_Settings.Width, m_Settings.Height)/25);

	if (m_Settings.WideBorder)
		GfxBorder *= 2;

	const UINT LabelBorder = GfxBorder/2;

	// Compute true boundaries and BackgroundScale
	if (((m_Settings.Width<BGWIDTH) || (m_Settings.Height<BGHEIGHT)) && (MaxZ>0))
	{
		const UINT Border = (MinS>0) && (MaxS<BGWIDTH-1) ? GfxBorder : 0;
		const INT Z = (MinZ+MaxZ)/2;
		const INT S = (MinS+MaxS)/2;
		const REAL ScX = (REAL)(m_Settings.Width-2*Border)/(REAL)(MaxS-MinS+1);
		const REAL ScY = (REAL)(m_Settings.Height-2*Border)/(REAL)(MaxZ-MinZ+1);

		BackgroundScale = min(ScX, ScY);
		if (BackgroundScale>2.0f)
			BackgroundScale = 2.0f;

		MinS = S-(INT)(m_Settings.Width/(BackgroundScale*2.0f));
		MaxS = S+(INT)(m_Settings.Width/(BackgroundScale*2.0f))-1;
		MinZ = Z-(INT)(m_Settings.Height/(BackgroundScale*2.0f));
		MaxZ = Z+(INT)(m_Settings.Height/(BackgroundScale*2.0f))-1;

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
	}
	else
	{
		MinZ = MinS = 0;
		MaxZ = BGHEIGHT-1;
		MaxS = BGWIDTH-1;
	}

	// Background
	const INT Width = (INT)((MaxS-MinS+1)*BackgroundScale+0.5f);
	const INT Height = (INT)((MaxZ-MinZ+1)*BackgroundScale+0.5f);
	CBitmap* pMapBitmap = LoadBackground(MinS, MinZ, MaxS, MaxZ, Width, Height, MapOffset);

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
	if (m_Settings.ShowRoutes && RouteCount)
	{
		// Map pen
		Pen pen(Color(0x00000000));

		// Mask pen
		HPEN hPen = CreatePen(PS_SOLID, 7, MASKCLR_ROUTE);
		HPEN hOldPen = (HPEN)dcMask.SelectObject(hPen);

		// Draw tesselated routes
		if (!m_Settings.StraightLines)
			for (UINT a=0; a<RouteCount; a++)
			{
				const FlightSegments* pSegments = ppRouteData[a];
				COLORREF clr = PreparePen(pen, pSegments->Route, pKitchen, GfxScale);

				for (UINT b=1; b<pSegments->PointCount; b++)
					DrawLine(dcMask, g, pen,
						MapX(pSegments->Points[b-1][1]), MapY(pSegments->Points[b-1][0]),
						MapX(pSegments->Points[b][1]), MapY(pSegments->Points[b][0]),
						BackgroundScale, MinS, MinZ);

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity)
				{
					SolidBrush brush(Color(COLORREF2RGB(clr)));

					if (pSegments->Route.Arrows & ARROW_FT)
						DrawArrow(g, brush,
							MapX(pSegments->Points[pSegments->PointCount-1][1]), MapY(pSegments->Points[pSegments->PointCount-1][0]),
							MapX(pSegments->Points[pSegments->PointCount-2][1]), MapY(pSegments->Points[pSegments->PointCount-2][0]),
							BackgroundScale, GfxScale, MinS, MinZ);

					if (pSegments->Route.Arrows & ARROW_TF)
						DrawArrow(g, brush,
							MapX(pSegments->Points[0][1]), MapY(pSegments->Points[0][0]),
							MapX(pSegments->Points[1][1]), MapY(pSegments->Points[1][0]),
							BackgroundScale, GfxScale, MinS, MinZ);
				}
			}

		UINT CurRoute = 0;
		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			const FactoryAirportData* pFrom = (FactoryAirportData*)pPair->value.lpFrom;
			const FactoryAirportData* pTo = (FactoryAirportData*)pPair->value.lpTo;

			if (m_Settings.StraightLines)
			{
				// Draw straight routes and compute annotation position
				COLORREF clr = PreparePen(pen, pPair->value, pKitchen, GfxScale);

				const BOOL UseWaypoint = (pFrom==pTo) && ((pPair->value.Waypoint.Latitude!=0.0) || (pPair->value.Waypoint.Longitude!=0.0));
				DrawLine(dcMask, g, pen, pFrom->X, pFrom->Y,
					UseWaypoint ? ((REAL)pPair->value.Waypoint.Longitude*4096.0f)/180.0f+4096.0f+MapOffset : pTo->X,
					UseWaypoint ? ((REAL)pPair->value.Waypoint.Latitude*2048.0f)/90.0f+2048.0f : pTo->Y,
					BackgroundScale, MinS, MinZ, &pPair->value.LabelX, &pPair->value.LabelY);

				if (m_Settings.Arrows && !m_Settings.UseCountOpacity && !UseWaypoint)
				{
					SolidBrush brush(Color(COLORREF2RGB(clr)));

					if (pPair->value.Arrows & ARROW_FT)
						DrawArrow(g, brush, pTo->X, pTo->Y, pFrom->X, pFrom->Y, BackgroundScale, GfxScale, MinS, MinZ);

					if (pPair->value.Arrows & ARROW_TF)
						DrawArrow(g, brush, pFrom->X, pFrom->Y, pTo->X, pTo->Y, BackgroundScale, GfxScale, MinS, MinZ);
				}
			}
			else
			{
				// Compute annotation position for tesselated routes
				const FlightSegments* pSegments = ppRouteData[CurRoute++];

				pPair->value.LabelX = MapX(pSegments->Points[pSegments->PointCount/2][1])-MinS;
				pPair->value.LabelY = MapY(pSegments->Points[pSegments->PointCount/2][0])-MinZ;
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

		for (UINT a=0; a<AirportCount; a++)
		{
			const INT X = (INT)((pAirportData[a].X-MinS)*BackgroundScale-LocationRadius+0.5f);
			const INT Y = (INT)((pAirportData[a].Y-MinZ)*BackgroundScale-LocationRadius+0.5f);

			// Draw on map
			g.FillEllipse(&brush, X, Y, LocationDiameter, LocationDiameter);

			if (m_Settings.LocationsOuterColor!=(COLORREF)-1)
				g.DrawEllipse(&pen, X, Y, LocationDiameter, LocationDiameter);

			// Draw on mask
			dcMask.Ellipse(CRect(X-AIRPORTMARGIN, Y-AIRPORTMARGIN, X+LocationDiameter+AIRPORTMARGIN, Y+LocationDiameter+AIRPORTMARGIN));
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

		for (CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair))
		{
			const FactoryAirportData* pData = (FactoryAirportData*)pPair->value.lpAirportData;
			const INT X = (INT)((pData->X-MinS)*BackgroundScale-LocationRadius+0.5f)+(INT)LocationRadius;
			const INT Y = (INT)((pData->Y-MinZ)*BackgroundScale-LocationRadius+0.5f)+(INT)LocationRadius;

			// Prepare path
			WCHAR pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, pPair->value.pAirport->Code, -1, pszBuf, 4);

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
				{ -LabelWidth-(INT)(LocationRadius*1.5f), (INT)(LocationRadius*1.5f) },
				{ -LabelWidth-(INT)(LocationRadius*1.5f), -LabelHeight-(INT)(LocationRadius*1.5f) },
				{ (INT)(LocationRadius*1.5f), (INT)(LocationRadius*1.5f) },
				{ (INT)(LocationRadius*1.5f), -LabelHeight-(INT)(LocationRadius*1.5f) },
			};

			for (UINT a=0; a<8; a++)
			{
				CRect rectLabel(X+LabelOffsets[a].x-LabelMargin, Y+LabelOffsets[a].y-LabelMargin, X+LabelOffsets[a].x+LabelWidth+LabelMargin, Y+LabelOffsets[a].y+LabelHeight+LabelMargin);

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
		FontFamily font(m_Settings.NoteSmallFont ? _T("Tahoma") : _T("Arial"));
		const REAL FontSize = (m_Settings.NoteSmallFont ? 14.0f : 15.5f)*GfxScale;

		// Map pen and brush
		Pen pen(Color(COLORREF2ARGB(m_Settings.IATACodesOuterColor, (m_Settings.NoteOuterColor || (m_Settings.ForegroundScale==2)) ? 0xFF : 0xCC)), (m_Settings.NoteSmallFont ? 1.75f : 2.0f)*GfxScale+1.0f);
		pen.SetLineJoin(LineJoinRound);

		SolidBrush brush(Color(COLORREF2RGB(m_Settings.NoteInnerColor)));

		const UINT MaxLines = (m_Settings.NoteDistance ? 1 : 0)+(m_Settings.NoteFlightCount ? 1 : 0)+(m_Settings.NoteFlightTime ? 1 : 0)+(m_Settings.NoteCarrier ? 1 : 0)+(m_Settings.NoteEquipment ? 1 : 0);

		for (CFlightRoutes::CPair* pPair = pKitchen->m_FlightRoutes.PGetFirstAssoc(); pPair; pPair = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair))
		{
			// Skip if there is no anchor point
			const INT X = (INT)(pPair->value.LabelX*BackgroundScale+0.5f);
			const INT Y = (INT)(pPair->value.LabelY*BackgroundScale+0.5f);

			if ((X==-1) || (Y==-1))
				continue;

			// Create annotation text
			CString strNote;
			WCHAR tmpStr[256];

			// Distance
			if (m_Settings.NoteDistance && (pPair->value.DistanceNM!=0.0))
			{
				AppendLabel(strNote, IDS_COLUMN6, MaxLines);

				DistanceToString(tmpStr, 256, pPair->value.DistanceNM);
				strNote.Append(tmpStr);
			}

			// Flight time
			if (m_Settings.NoteFlightTime && (pPair->value.FlightTimeCount))
			{
				AppendLabel(strNote, IDS_COLUMN23, MaxLines);

				const UINT FlightTime = pPair->value.FlightTime/pPair->value.FlightTimeCount;

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

					ENSURE(tmpMask.LoadString(pPair->value.Count==1 ? IDS_FLIGHTS_SINGULAR : IDS_FLIGHTS_PLURAL));
				}

				swprintf_s(tmpStr, 256, tmpMask, pPair->value.Count);
				strNote.Append(tmpStr);
			}

			// Carrier
			if (m_Settings.NoteCarrier && (pPair->value.Carrier[0]!=L'\0') && !pPair->value.CarrierMultiple)
			{
				AppendLabel(strNote, IDS_COLUMN7, MaxLines);
				strNote.Append(pPair->value.Carrier);
			}

			// Equipment
			if (m_Settings.NoteEquipment && (pPair->value.Equipment[0]!=L'\0') && !pPair->value.EquipmentMultiple)
			{
				AppendLabel(strNote, IDS_COLUMN10, MaxLines);
				strNote.Append(pPair->value.Equipment);
			}

			// Create path from string
			StringFormat strFormat;
			GraphicsPath TextPath;
			TextPath.AddString(strNote, strNote.GetLength(), &font, FontStyleRegular, FontSize, Gdiplus::Point(0, 0), &strFormat);

			RectF rectPath;
			TextPath.GetBounds(&rectPath);

			const INT NoteW = (INT)(rectPath.Width+0.5f);
			const INT NoteH = (INT)(rectPath.Height+0.5f);

			// Find suitable location
			CRect rectNote((INT)(X-(REAL)NoteW/2.0f), (INT)(Y-(REAL)NoteH/2.0f), (INT)(X+(REAL)NoteW/2.0f), (INT)(Y+(REAL)NoteH/2.0f));

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

	// Deface
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
	if (ppRouteData)
	{
		for (UINT a=0; a<RouteCount; a++)
			free(ppRouteData[a]);

		delete ppRouteData;
	}

	delete pAirportData;
	delete pKitchen;

	return pMapBitmap;
}
