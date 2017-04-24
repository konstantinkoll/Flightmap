
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "CGoogleEarthFile.h"
#include "GlobeOptionsDlg.h"
#include "Flightmap.h"
#include <math.h>


// CGlobeView
//

#define DISTANCENEAR           3.0f
#define DISTANCEFAR            17.0f
#define DOLLY                  0.09f
#define BLENDOUT               0.075f
#define BLENDIN                0.275f
#define ARROWSIZE              8
#define ANIMLENGTH             200
#define MOVEDELAY              10
#define MOVEDIVIDER            8.0f
#define WHITE                  100

CString CGlobeView::m_strFlightCountSingular;
CString CGlobeView::m_FlightCount_Plural;

const GLfloat CGlobeView::m_lAmbient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
const GLfloat CGlobeView::m_lDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat CGlobeView::m_lSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat CGlobeView::m_FogColor[] = { 0.65f, 0.75f, 0.95f, 1.0f };

CGlobeView::CGlobeView()
	: CFrontstageWnd()
{
	m_RenderContext.pDC = NULL;
	m_RenderContext.hRC = NULL;

	lpszCursorName = IDC_WAIT;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;

	m_FocusItem = m_HotItem = -1;

	m_nTextureBlueMarble = m_nTextureClouds = m_nTextureLocationIndicator = m_nRouteModel = m_nGlobeModel = m_nHaloModel = 0;
	m_GlobeRadius = m_Momentum = 0.0f;
	m_IsSelected = m_Hover = m_Grabbed = FALSE;
	m_MinRouteCount = m_MaxRouteCount = m_AnimCounter = m_MoveCounter = 0;

	if (m_strFlightCountSingular.IsEmpty())
		ENSURE(m_strFlightCountSingular.LoadString(IDS_FLIGHTCOUNT_SINGULAR));

	if (m_FlightCount_Plural.IsEmpty())
		ENSURE(m_FlightCount_Plural.LoadString(IDS_FLIGHTCOUNT_PLURAL));
}

BOOL CGlobeView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS | CS_OWNDC, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	if (!CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID))
		return FALSE;

	UpdateViewOptions(TRUE);

	return TRUE;
}

BOOL CGlobeView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		FMGetApp()->HideTooltip();
		break;
	}

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

void CGlobeView::SetFlights(CKitchen* pKitchen, BOOL DeleteKitchen)
{
	// Reset
	m_Airports.m_ItemCount = 0;
	m_DisplayName = pKitchen->m_DisplayName;

	// Airports
	CFlightAirports::CPair* pPair1 = pKitchen->m_FlightAirports.PGetFirstAssoc();
	while (pPair1)
	{
		GlobeItemData pData;
		ZeroMemory(&pData, sizeof(pData));
		pData.pAirport = pPair1->value.pAirport;

		strcpy_s(pData.NameString, 130, pData.pAirport->Name);
		const FMCountry* pCountry = FMIATAGetCountry(pData.pAirport->CountryID);
		if (pCountry)
		{
			strcat_s(pData.NameString, 130, ", ");
			strcat_s(pData.NameString, 130, pCountry->Name);
		}

		FMGeoCoordinatesToString(pData.pAirport->Location, pData.CoordString, 32, FALSE);

		// Calculate world coordinates
		const DOUBLE LatitudeRad = -theRenderer.DegToRad(pData.pAirport->Location.Latitude);
		const DOUBLE LongitudeRad = theRenderer.DegToRad(pData.pAirport->Location.Longitude);

		const DOUBLE D = cos(LatitudeRad);

		pData.World[0] = (GLfloat)(sin(LongitudeRad)*D);
		pData.World[1] = (GLfloat)(sin(LatitudeRad));
		pData.World[2] = (GLfloat)(cos(LongitudeRad)*D);

		// Other properties
		UINT Count = 0;
		pKitchen->m_FlightAirportCounts.Lookup(pData.pAirport->Code, Count);

		CString tmpStr;
		tmpStr.Format(Count==1 ? m_strFlightCountSingular : m_FlightCount_Plural, Count);
		wcscpy_s(pData.CountStringLarge, 64, tmpStr.GetBuffer());

		tmpStr.Format(_T("%u"), Count);
		wcscpy_s(pData.CountStringSmall, 16, tmpStr.GetBuffer());

		m_Airports.AddItem(pData);
		pPair1 = pKitchen->m_FlightAirports.PGetNextAssoc(pPair1);
	}

	// Routes
	m_MinRouteCount = pKitchen->m_MinRouteCount;
	m_MaxRouteCount = pKitchen->m_MaxRouteCount;

	CFlightRoutes::CPair* pPair2 = pKitchen->m_FlightRoutes.PGetFirstAssoc();
	while (pPair2)
	{
		m_Routes.AddItem(pKitchen->Tesselate(pPair2->value));

		pPair2 = pKitchen->m_FlightRoutes.PGetNextAssoc(pPair2);
	}

	// Finish
	if (DeleteKitchen)
		delete pKitchen;

	// Create display list for routes
	if (!m_RenderContext.hRC)
		return;

	theRenderer.MakeCurrent(m_RenderContext);

	if (!m_nRouteModel)
		m_nRouteModel = glGenLists(1);

	glNewList(m_nRouteModel, GL_COMPILE);
	theRenderer.SetColor(0xFFFFFF);

	for (UINT a=0; a<m_Routes.m_ItemCount; a++)
	{
		glBegin(GL_LINE_STRIP);
		DOUBLE* pPoints = &m_Routes[a]->Points[0][0];

		for (UINT b=0; b<m_Routes[a]->PointCount; b++)
		{
			const DOUBLE H = pPoints[2];
			const DOUBLE D = cos(pPoints[0]);
			const DOUBLE X = H*sin(pPoints[1])*D;
			const DOUBLE Y = -H*sin(pPoints[0]);
			const DOUBLE Z = H*cos(pPoints[1])*D;

			glVertex3d(X, Y, Z);
			pPoints += 3;
		}

		glEnd();
	}

	glEndList();
}

void CGlobeView::UpdateViewOptions(BOOL Force)
{
	// Settings
	if (Force)
	{
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude = theApp.m_GlobeLatitude/1000.0f;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude = theApp.m_GlobeLongitude/1000.0f;
		m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom = theApp.m_GlobeZoom;
	}

	// Textures and halo
	if (m_RenderContext.hRC)
	{
		CWaitCursor csr;

		theRenderer.MakeCurrent(m_RenderContext);

		theRenderer.CreateTextureBlueMarble(m_nTextureBlueMarble);
		theRenderer.CreateTextureClouds(m_nTextureClouds);
		theRenderer.CreateTextureLocationIndicator(m_nTextureLocationIndicator);

		if (m_nHaloModel)
		{
			glDeleteLists(m_nHaloModel, 1);
			m_nHaloModel = 0;
		}

		Invalidate();
	}
}

INT CGlobeView::ItemAtPosition(CPoint point) const
{
	INT Result = -1;
	GLfloat Alpha = 0.0f;

	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		const GlobeItemData* pData = &m_Airports[a];

		if ((pData->Alpha>0.75f) || ((pData->Alpha>0.1f) && (pData->Alpha>Alpha-0.05f)))
			if (PtInRect(&pData->Rect, point))
			{
				Alpha = pData->Alpha;
				Result = (INT)a;
			}
	}

	return Result;
}

void CGlobeView::InvalidateItem(INT Index)
{
	if ((Index>=0) && (Index<(INT)m_Airports.m_ItemCount))
	{
		RECT rect = m_Airports[Index].Rect;
		InflateRect(&rect, 0, ARROWSIZE);
		InvalidateRect(&rect);
	}
}

void CGlobeView::SelectItem(INT Index, BOOL Select)
{
	if ((Index!=m_FocusItem) || (Select!=m_IsSelected))
	{
		InvalidateItem(m_FocusItem);

		m_FocusItem = Index;
		m_IsSelected = Select;

		InvalidateItem(m_FocusItem);
	}
}

BOOL CGlobeView::CursorOnGlobe(const CPoint& point) const
{
	const GLfloat DistX = (GLfloat)point.x-(GLfloat)m_RenderContext.Width/2.0f;
	const GLfloat DistY = (GLfloat)point.y-(GLfloat)m_RenderContext.Height/2.0f;

	return DistX*DistX+DistY*DistY < m_GlobeRadius*m_GlobeRadius;
}

void CGlobeView::UpdateCursor()
{
	LPCWSTR Cursor;

	if (m_Grabbed)
	{
		Cursor = IDC_HAND;
	}
	else
	{
		Cursor = IDC_ARROW;

		if (CursorOnGlobe(m_CursorPos))
			if (ItemAtPosition(m_CursorPos)==-1)
				Cursor = IDC_HAND;
	}

	if (Cursor!=lpszCursorName)
	{
		hCursor = theApp.LoadStandardCursor(Cursor);

		SetCursor(hCursor);
		lpszCursorName = Cursor;
	}
}


// OpenGL
//

__forceinline void CGlobeView::CalcAndDrawSpots(const GLfloat ModelView[4][4], const GLfloat Projection[4][4])
{
	GLfloat SizeX = m_RenderContext.Width/2.0f;
	GLfloat SizeY = m_RenderContext.Height/2.0f;

	GLfloat MVP[4][4];
	theRenderer.MatrixMultiplication4f(MVP, ModelView, Projection);

	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		GlobeItemData* pData = &m_Airports[a];

		pData->Alpha = 0.0f;

		const GLfloat Z = ModelView[0][2]*pData->World[0] + ModelView[1][2]*pData->World[1] + ModelView[2][2]*pData->World[2];
		if (Z>BLENDOUT)
		{
			const GLfloat W = MVP[0][3]*pData->World[0] + MVP[1][3]*pData->World[1] + MVP[2][3]*pData->World[2] + MVP[3][3];
			const GLfloat X = (MVP[0][0]*pData->World[0] + MVP[1][0]*pData->World[1] + MVP[2][0]*pData->World[2] + MVP[3][0])*SizeX/W + SizeX + 0.5f;
			const GLfloat Y = -(MVP[0][1]*pData->World[0] + MVP[1][1]*pData->World[1] + MVP[2][1]*pData->World[2] + MVP[3][1])*SizeY/W + SizeY + 0.5f;

			pData->ScreenPoint[0] = (INT)X;
			pData->ScreenPoint[1] = (INT)Y;
			pData->Alpha = (Z<BLENDIN) ? (GLfloat)((Z-BLENDOUT)/(BLENDIN-BLENDOUT)) : 1.0f;

			if (theApp.m_GlobeShowSpots)
				theRenderer.DrawIcon(X, Y, 6.0f+8.0f*pData->Alpha, pData->Alpha);
		}
	}
}

__forceinline void CGlobeView::CalcAndDrawLabel(BOOL Themed)
{
	if (!(theApp.m_GlobeShowAirportIATA | theApp.m_GlobeShowAirportNames | theApp.m_GlobeShowGPS | theApp.m_GlobeShowMovements))
		return;

	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		GlobeItemData* pData = &m_Airports[a];

		if (pData->Alpha>0.0f)
		{
			// Beschriftung
			LPCSTR pCaption = (theApp.m_GlobeShowAirportIATA ? pData->pAirport->Code : theApp.m_GlobeShowAirportNames ? pData->NameString : NULL);
			LPCSTR pSubcaption = ((theApp.m_GlobeShowAirportIATA && theApp.m_GlobeShowAirportNames) ? pData->NameString : NULL);
			LPCSTR pCoordinates = (theApp.m_GlobeShowGPS ? pData->CoordString : NULL);
			LPCWSTR pCount = (theApp.m_GlobeShowMovements ? (theApp.m_GlobeShowAirportNames || theApp.m_GlobeShowGPS) ? pData->CountStringLarge : pData->CountStringSmall : NULL);

			DrawLabel(pData, pCaption, pSubcaption, pCoordinates, pCount, m_FocusItem==(INT)a, m_HotItem==(INT)a, Themed);
		}
	}
}

__forceinline void CGlobeView::DrawLabel(GlobeItemData* pData, LPCSTR pCaption, LPCSTR pSubcaption, LPCSTR pCoordinates, LPCWSTR pDescription, BOOL Focused, BOOL Hot, BOOL Themed)
{
	ASSERT(ARROWSIZE>3);

	// Width
	UINT W1 = m_Fonts[1].GetTextWidth(pCaption);
	UINT W2 = m_Fonts[0].GetTextWidth(pSubcaption);
	UINT W3 = m_Fonts[0].GetTextWidth(pCoordinates);
	UINT W4 = m_Fonts[0].GetTextWidth(pDescription);
	UINT Width = max(2*ARROWSIZE, max(W1, max(W2, max(W3, W4)))+11);

	// Height
	UINT Height = 8;
	Height += m_Fonts[1].GetTextHeight(pCaption);
	Height += m_Fonts[0].GetTextHeight(pSubcaption);
	Height += m_Fonts[0].GetTextHeight(pCoordinates);
	Height += m_Fonts[0].GetTextHeight(pDescription);

	// Position and bounding rectangle
	INT Top = (pData->ScreenPoint[1]<m_RenderContext.Height/2) ? -1 : 1;

	INT x = pData->Rect.left = pData->ScreenPoint[0]-ARROWSIZE-(((INT)Width-2*ARROWSIZE)*(m_RenderContext.Width-pData->ScreenPoint[0])/m_RenderContext.Width);
	INT y = pData->Rect.top = pData->ScreenPoint[1]+(ARROWSIZE-2)*Top-(Top<0 ? (INT)Height : 0);
	pData->Rect.right = x+Width;
	pData->Rect.bottom = y+Height;

	// Visible?
	if ((x+Width+6<0) || (x-1>m_RenderContext.Width) || (y+Height+ARROWSIZE+6<0) || (y-ARROWSIZE-6>m_RenderContext.Height))
	{
		pData->Alpha = 0.0f;
		return;
	}

	// Colors
	GLcolor BorderColor;
	theRenderer.ColorRef2GLColor(BorderColor, Themed ? Focused ? 0xE08010 : Hot ? 0xF0C08A : 0xD5D1D0 : GetSysColor(Focused ? COLOR_HIGHLIGHT : COLOR_3DSHADOW));

	GLcolor BackgroundColor;
	theRenderer.ColorRef2GLColor(BackgroundColor, Themed ? 0xFFFFFF : GetSysColor(Focused ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	// Shadow
	if (Themed)
	{
		glColor4f(0.0f, 0.0f, 0.0f, pData->Alpha*(12.0f/256.0f));
		glBegin(GL_LINES);
		glVertex2i(x+2, y+Height);
		glVertex2i(x+Width-1, y+Height);
		glVertex2i(x+Width, y+2);
		glVertex2i(x+Width, y+Height-1);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x+Width, y+Height-1);
		glEnd();
	}

	// Inner
	if (Themed && (Hot | Focused))
	{
		glBegin(GL_QUADS);
		theRenderer.SetColor(*(Focused ? &m_TopColorSelected : &m_TopColorHot), pData->Alpha);
		glVertex2i(x+1, y+1);
		glVertex2i(x+Width-1, y+1);

		theRenderer.SetColor(*(Focused ? &m_BottomColorSelected : &m_BottomColorHot), pData->Alpha);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x+1, y+Height-1);
		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, (((Hot && !Focused) ? 0x60 : 0x48)*pData->Alpha)/255.0f);

		glBegin(GL_LINE_LOOP);
		glVertex2i(x+1, y+1);
		glVertex2i(x+Width-2, y+1);
		glVertex2i(x+Width-2, y+Height-2);
		glVertex2i(x+1, y+Height-2);
		glEnd();

		theRenderer.SetColor(*(Top>0 ? Focused ? &m_TopColorSelected : &m_TopColorHot : Focused ? &m_BottomColorSelected : &m_BottomColorHot), pData->Alpha);
	}
	else
	{
		theRenderer.SetColor(BackgroundColor, pData->Alpha);
		glRecti(x+1, y+1, x+Width-1, y+Height-1);
	}

	// Arrow
	glBegin(GL_TRIANGLES);
	glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]);

	if (Top>0)
	{
		glVertex2i(pData->ScreenPoint[0]+ARROWSIZE+1, pData->ScreenPoint[1]+ARROWSIZE);
		glVertex2i(pData->ScreenPoint[0]-ARROWSIZE, pData->ScreenPoint[1]+ARROWSIZE);
	}
	else
	{
		glVertex2i(pData->ScreenPoint[0]+ARROWSIZE, pData->ScreenPoint[1]-ARROWSIZE);
		glVertex2i(pData->ScreenPoint[0]-ARROWSIZE, pData->ScreenPoint[1]-ARROWSIZE);
	}
	glEnd();

	// Border
	glBegin(GL_LINES);
	theRenderer.SetColor(BorderColor, pData->Alpha);

	glVertex2i(x, y+2);					// Left
	glVertex2i(x, y+Height-2);
	glVertex2i(x+Width-1, y+2);			// Right
	glVertex2i(x+Width-1, y+Height-2);

	if (Top>0)
	{
		glVertex2i(x+2, y+Height-1);	// Bottom
		glVertex2i(x+Width-2, y+Height-1);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2i(x+2, y);
		glVertex2i(pData->ScreenPoint[0]-(ARROWSIZE-1), y);
		glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]-1);
		glVertex2i(pData->ScreenPoint[0]+(ARROWSIZE-1), y);
		glVertex2i(x+Width-2, y);
	}
	else
	{
		glVertex2i(x+2, y);				// Top
		glVertex2i(x+Width-2, y);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2i(x+2, y+Height-1);
		glVertex2i(pData->ScreenPoint[0]-(ARROWSIZE-1), y+Height-1);
		glVertex2i(pData->ScreenPoint[0], pData->ScreenPoint[1]);
		glVertex2i(pData->ScreenPoint[0]+(ARROWSIZE-1), y+Height-1);
		glVertex2i(x+Width-2, y+Height-1);
	}
	glEnd();

	theRenderer.SetColor(BorderColor, pData->Alpha*0.5f);

	glBegin(GL_POINTS);
	glVertex2i(x, y+1);					// Upper left
	glVertex2i(x+1, y+1);
	glVertex2i(x+1, y);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x, y+Height-2);			// Lower left
	glVertex2i(x+1, y+Height-2);
	glVertex2i(x+1, y+Height-1);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x+Width-1, y+1);			// Upper right
	glVertex2i(x+Width-2, y+1);
	glVertex2i(x+Width-2, y);
	glEnd();

	glBegin(GL_POINTS);
	glVertex2i(x+Width-1, y+Height-2);	// Lower right
	glVertex2i(x+Width-2, y+Height-2);
	glVertex2i(x+Width-2, y+Height-1);
	glEnd();

	x += 5;
	y += 3;

	// Caption
	m_Fonts[1].Begin(*(Focused ? &m_SelectedColor : &m_CaptionColor), pData->Alpha);
	y += m_Fonts[1].Render(pCaption, x, y);
	m_Fonts[1].End();

	// Hints
	m_Fonts[0].Begin(*(Focused ? &m_SelectedColor : &m_TextColor), pData->Alpha);

	if (pSubcaption)
		y += m_Fonts[0].Render(pSubcaption, x, y);

	if (pCoordinates)
		y += m_Fonts[0].Render(pCoordinates, x, y);

	// Description
	if (pDescription)
	{
		if (!Focused)
			m_Fonts[0].SetColor(m_AttrColor, pData->Alpha);

		y += m_Fonts[0].Render(pDescription, x, y);
	}

	m_Fonts[0].End();
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	BOOL Result = Redraw;

	// Do not roll over ploes
	if (m_GlobeTarget.Latitude<-75.0f)
		m_GlobeTarget.Latitude = -75.0f;

	if (m_GlobeTarget.Latitude>75.0f)
		m_GlobeTarget.Latitude = 75.0f;

	// Normalize rotation
	if (m_GlobeTarget.Longitude<-180.0f)
		m_GlobeTarget.Longitude += 360.0f;

	if (m_GlobeTarget.Longitude>180.0f)
		m_GlobeTarget.Longitude -= 360.0f;

	// Zoom
	if (m_GlobeTarget.Zoom<0)
		m_GlobeTarget.Zoom = 0;

	if (m_GlobeTarget.Zoom>1000)
		m_GlobeTarget.Zoom = 1000;

	if (m_GlobeCurrent.Zoom<=m_GlobeTarget.Zoom-5)
	{
		m_GlobeCurrent.Zoom += 5;
		m_HotItem = -1;

		Result = TRUE;
	}
	else
		if (m_GlobeCurrent.Zoom>=m_GlobeTarget.Zoom+5)
		{
			m_GlobeCurrent.Zoom -= 5;
			m_HotItem = -1;

			Result = TRUE;
		}
		else
		{
			Result |= (m_GlobeCurrent.Zoom!=m_GlobeTarget.Zoom);

			m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom;
		}

	// Animation
	if (m_AnimCounter)
	{
		const GLfloat Factor = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);

		m_GlobeCurrent.Latitude = m_AnimStartLatitude*(1.0f-Factor) + m_GlobeTarget.Latitude*Factor;
		m_GlobeCurrent.Longitude = m_AnimStartLongitude*(1.0f-Factor) + m_GlobeTarget.Longitude*Factor;

		if (m_GlobeTarget.Zoom<600)
		{
			const INT Distance = 600-m_GlobeTarget.Zoom;
			const INT MaxDistance = (INT)((m_GlobeTarget.Zoom+100)*1.5f);

			const GLfloat Factor = (GLfloat)sin(PI*m_AnimCounter/ANIMLENGTH);

			m_GlobeCurrent.Zoom = (INT)(m_GlobeTarget.Zoom*(1.0f-Factor)+(m_GlobeTarget.Zoom+min(Distance, MaxDistance))*Factor);
		}

		m_AnimCounter--;

		Result = TRUE;
	}
	else
	{
		if (m_Momentum!=0.0f)
			m_GlobeTarget.Longitude += m_Momentum;

		Result |= (m_GlobeCurrent.Latitude!=m_GlobeTarget.Latitude) || (m_GlobeCurrent.Longitude!=m_GlobeTarget.Longitude);

		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude;
	}

	if (Result)
	{
		Invalidate();
		UpdateCursor();
	}

	return Result;
}

__forceinline void CGlobeView::RenderScene(BOOL Themed)
{
	theRenderer.BeginRender(this, m_RenderContext);

	//Clear background
	//
	GLcolor BackColor;
	theRenderer.ColorRef2GLColor(BackColor, theApp.m_GlobeDarkBackground ? Themed ? 0x141312 : 0x181818 : Themed ? 0xF8F5F4 : GetSysColor(COLOR_WINDOW));

	glClearColor(BackColor[0], BackColor[1], BackColor[2], 1.0f);
	glClearDepth(DISTANCEFAR+25.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// White top gradient
	if (Themed && !theApp.m_GlobeDarkBackground)
	{
		theRenderer.Project2D();
		glBegin(GL_QUADS);

		theRenderer.SetColor(BackColor);
		glVertex2i(0, WHITE-1);
		glVertex2i(m_RenderContext.Width, WHITE-1);

		glColor3f(1.0, 1.0, 1.0);
		glVertex2i(m_RenderContext.Width, 0);
		glVertex2i(0, 0);

		glEnd();
	}

	// Setup colors
	//
	theRenderer.ColorRef2GLColor(m_AttrColor, Themed ? 0x333333 : GetSysColor(COLOR_WINDOWTEXT));
	theRenderer.ColorRef2GLColor(m_BottomColorHot, 0xFAEBE0);
	theRenderer.ColorRef2GLColor(m_BottomColorSelected, 0xE08010);
	theRenderer.ColorRef2GLColor(m_CaptionColor, Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	theRenderer.ColorRef2GLColor(m_SelectedColor, Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));
	theRenderer.ColorRef2GLColor(m_TextColor, Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));
	theRenderer.ColorRef2GLColor(m_TopColorHot, 0xFFFCF9);
	theRenderer.ColorRef2GLColor(m_TopColorSelected, 0xFFA020);

	// Draw globe
	//

	// Distance
	GLfloat Distance = DISTANCENEAR+((DISTANCEFAR-DISTANCENEAR)*m_GlobeCurrent.Zoom)/1000.0f;
	ASSERT(Distance>0.0f);

	// Size
	m_GlobeRadius = (GLfloat)min(m_RenderContext.Width, m_RenderContext.Height)/(DOLLY*Distance*2.0f);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Distance, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLfloat ScaleX = (m_RenderContext.Width>m_RenderContext.Height) ? (GLfloat)m_RenderContext.Width/(GLfloat)(m_RenderContext.Height+1) : 1.0f;
	GLfloat ScaleY = (m_RenderContext.Height>m_RenderContext.Width) ? (GLfloat)m_RenderContext.Height/(GLfloat)(m_RenderContext.Width+1) : 1.0f;
	glFrustum(-ScaleX*DOLLY, ScaleX*DOLLY, -ScaleY*DOLLY, ScaleY*DOLLY, 1.0f, 1000.0f);

	// Halo
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELHIGH)
	{
		if (!m_nHaloModel)
		{
			m_nHaloModel = glGenLists(1);
			glNewList(m_nHaloModel, GL_COMPILE);

			GLcolor HaloColor;
			theRenderer.ColorRef2GLColor(HaloColor, BackColor[1]>=0.5f ? 0xFFFFFF : 0xFFD8C0);

			const GLfloat Radius = (BackColor[1]>=0.5f) ? 1.125f : 1.0125f;

			glBegin(GL_QUAD_STRIP);

			for (UINT a=0; a<=256; a++)
			{
				const GLfloat Arc = PI*a/128.0f;
				const GLfloat X = sin(Arc);
				const GLfloat Y = cos(Arc);

				theRenderer.SetColor(HaloColor);
				glVertex3f(0.0f, X, Y);
	
				theRenderer.SetColor(HaloColor, 0.0f);
				glVertex3f(0.0f, X*Radius, Y*Radius);
			}

			glEnd();
			glEndList();
		}

		glCallList(m_nHaloModel);
	}

	// Lighting
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELMEDIUM)
	{
		glLightfv(GL_LIGHT0, GL_AMBIENT, m_lAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, m_lDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, m_lSpecular);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, m_lAmbient);
	}

	// Rotate globe (AFTER lighting)
	glMatrixMode(GL_MODELVIEW);
	glRotatef(m_GlobeCurrent.Latitude, 0.0f, 0.0f, 1.0f);
	glRotatef(m_GlobeCurrent.Longitude+90.0f, 0.0f, 1.0f, 0.0f);

	// Store matrices for later
	GLfloat MatrixModelView[4][4];
	GLfloat MatrixProjection[4][4];
	glGetFloatv(GL_MODELVIEW_MATRIX, &MatrixModelView[0][0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &MatrixProjection[0][0]);

	// Atmosphere
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELHIGH)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_START, Distance-0.5f);
		glFogf(GL_FOG_END, Distance+0.35f);

		glFogfv(GL_FOG_COLOR, m_FogColor);
	}

	// Texture units
	if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELULTRA)
	{
		// Setup texture units for clouds
		// CloudAlpha depends on distance
		GLfloat CloudAlpha = (GLfloat)(m_GlobeCurrent.Zoom-300)/300.0f;
		if (CloudAlpha<0.0f)
			CloudAlpha = 0.0f;
		if (CloudAlpha>1.0f)
			CloudAlpha = 1.0f;

		// Texture unit 0
		// Multiply cloud texture with (0.4, 0.4, 0.4, CloudAlpha)
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureClouds);

		GLfloat AlphaColor[] = { 0.4f, 0.4f, 0.4f, CloudAlpha };
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, AlphaColor);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

		// Texture unit 1
		// Interpolate cloud texture and Blue Marble texture depending on cloud's texture alpha
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureBlueMarble);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

		// Texture unit 2
		// Modulate resulting texture with lighting-dependent vertex color
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureClouds);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
	else
	{
		// Simple texture mapping (either GL_MODULATE for lighting, or GL_REPLACE for "low" quality model)
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureBlueMarble);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELMEDIUM ? GL_MODULATE : GL_REPLACE);
	}

	// Render globe model
	theRenderer.EnableMultisample();
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glCallList(m_nGlobeModel);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	theRenderer.DisableMultisample();

	// Disable texture units
	if (glActiveTexture)
	{
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
	}

	glDisable(GL_TEXTURE_2D);

	// Disable atmosphere
	glDisable(GL_FOG);

	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	// 2D overlay
	//
	theRenderer.Project2D();

	// Draw locations
	if (m_Airports.m_ItemCount)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTextureLocationIndicator);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBegin(GL_QUADS);
		CalcAndDrawSpots(MatrixModelView, MatrixProjection);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	// Draw routes
	if (m_Routes.m_ItemCount)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&MatrixProjection[0][0]);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&MatrixModelView[0][0]);

		if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELULTRA)
			theRenderer.EnableMultisample();

		glLineWidth(min(3.75f, 1.0f+m_GlobeRadius/400.0f));
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glCallList(m_nRouteModel);

		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		if (min(theApp.m_ModelQuality, theRenderer.m_MaxModelQuality)>=MODELULTRA)
			theRenderer.DisableMultisample();

		theRenderer.Project2D();
	}

	// Draw label
	if (m_Airports.m_ItemCount)
		CalcAndDrawLabel(Themed);

	// Taskbar shadow
	if (Themed)
	{
		theRenderer.SetColor(0x000000, 0.094f);
		glRecti(0, 0, m_RenderContext.Width, 1);

		theRenderer.SetColor(0x000000, 0.047f);
		glRecti(0, 1, m_RenderContext.Width, 2);
	}

	theRenderer.EndRender(this, m_RenderContext, Themed);
}


BEGIN_MESSAGE_MAP(CGlobeView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()

	ON_COMMAND(IDM_GLOBEWND_JUMPTOLOCATION, OnJumpToLocation)
	ON_COMMAND(IDM_GLOBEWND_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_GLOBEWND_ZOOMOUT, OnZoomOut)
	ON_COMMAND(IDM_GLOBEWND_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_GLOBEWND_SAVEAS, OnSaveAs)
	ON_COMMAND(IDM_GLOBEWND_GOOGLEEARTH, OnGoogleEarth)
	ON_COMMAND(IDM_GLOBEWND_LIQUIDFOLDERS, OnLiquidFolders)
	ON_COMMAND(IDM_GLOBEWND_SETTINGS, OnSettings)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBEWND_JUMPTOLOCATION, IDM_GLOBEWND_LIQUIDFOLDERS, OnUpdateCommands)
END_MESSAGE_MAP()

INT CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!theRenderer.Initialize())
		return -1;

	// OpenGL
	if (theRenderer.CreateRenderContext(this, m_RenderContext))
	{
		// Fonts
		m_Fonts[0].Create(&theApp.m_DefaultFont);
		m_Fonts[1].Create(&theApp.m_LargeFont);

		// Model
		m_nGlobeModel = theRenderer.CreateGlobe();

		// Timer
		SetTimer(1, 10, NULL);
	}

	return 0;
}

void CGlobeView::OnDestroy()
{
	// OpenGL
	if (m_RenderContext.hRC)
	{
		// Timer
		KillTimer(1);

		theRenderer.MakeCurrent(m_RenderContext);

		// Textures
		glDeleteTextures(1, &m_nTextureBlueMarble);
		glDeleteTextures(1, &m_nTextureClouds);
		glDeleteTextures(1, &m_nTextureLocationIndicator);

		// Models
		glDeleteLists(m_nHaloModel, 1);
		glDeleteLists(m_nGlobeModel, 1);
	}

	theRenderer.DeleteRenderContext(m_RenderContext);

	// Settings
	theApp.m_GlobeLatitude = (INT)(m_GlobeTarget.Latitude*1000.0f);
	theApp.m_GlobeLongitude = (INT)(m_GlobeTarget.Longitude*1000.0f);
	theApp.m_GlobeZoom = m_GlobeTarget.Zoom;

	CFrontstageWnd::OnDestroy();
}

void CGlobeView::OnPaint()
{
	CPaintDC pDC(this);

	BOOL Themed = IsCtrlThemed();

	if (m_RenderContext.hRC)
	{
		RenderScene(Themed);
	}
	else
	{
		CRect rect;
		GetClientRect(rect);

		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		CRect rectText(rect);
		rectText.top += 6;

		dc.SetTextColor(0x0000FF);
		dc.DrawText(CString((LPCSTR)IDS_NORENDERINGCONTEXT), rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		DrawWindowEdge(dc, Themed);

		if (Themed)
		{
			Graphics g(dc);

			CTaskbar::DrawTaskbarShadow(g, rect);
		}

		pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldFont);
		dc.SelectObject(pOldBitmap);
	}
}

BOOL CGlobeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(hCursor);

	return TRUE;
}

void CGlobeView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	m_CursorPos = point;

	if (m_Grabbed)
	{
		m_MoveCounter = 0;

		const GLfloat Scale = ((GLfloat)min(m_RenderContext.Width, m_RenderContext.Height))/m_GlobeRadius;

		CSize szRotate = m_GrabPoint-point;
		m_GlobeTarget.Longitude = m_GlobeCurrent.Longitude += (m_LastMove=-szRotate.cx*Scale/4.0f)/4.0f;
		m_GlobeTarget.Latitude = m_GlobeCurrent.Latitude += szRotate.cy*Scale/16.0f;

		m_GrabPoint = point;

		UpdateScene(TRUE);
	}
	else
	{
		UpdateCursor();
	}

	INT Item = ItemAtPosition(point);
	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((FMGetApp()->IsTooltipVisible()) && (Item!=m_HotItem))
			FMGetApp()->HideTooltip();

	if (m_HotItem!=Item)
	{
		InvalidateItem(m_HotItem);
		m_HotItem = Item;
		InvalidateItem(m_HotItem);
	}
}

BOOL CGlobeView::OnMouseWheel(UINT /*nFlags*/, SHORT zDelta, CPoint /*pt*/)
{
	FMGetApp()->HideTooltip();

	if (zDelta<0)
	{
		OnZoomOut();
	}
	else
	{
		OnZoomIn();
	}

	return TRUE;
}

void CGlobeView::OnMouseLeave()
{
	FMGetApp()->HideTooltip();

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CGlobeView::OnMouseHover(UINT nFlags, CPoint point)
{
	if (m_Momentum==0.0f)
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
			if (m_HotItem!=-1)
			{
				if (!FMGetApp()->IsTooltipVisible())
					FMGetApp()->ShowTooltip(this, point, m_Airports[m_HotItem].pAirport, m_Airports[m_HotItem].CountStringLarge);
			}
			else
			{
				FMGetApp()->HideTooltip();
			}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index==-1)
	{
		if (CursorOnGlobe(point))
		{
			m_Momentum = m_LastMove = 0.0f;

			m_GrabPoint = point;
			m_Grabbed = TRUE;

			if (m_AnimCounter)
			{
				m_AnimCounter = 0;
				m_GlobeTarget = m_GlobeCurrent;
			}

			SetCapture();

			UpdateCursor();
		}

		if (GetFocus()!=this)
			SetFocus();
	}
	else
	{
		SelectItem(Index, (nFlags & MK_CONTROL) && (m_FocusItem==Index) && m_IsSelected ? !m_IsSelected : TRUE);
	}
}

void CGlobeView::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (m_Grabbed)
	{
		if (m_MoveCounter<MOVEDELAY)
			m_Momentum = m_LastMove/MOVEDIVIDER;

		m_Grabbed = FALSE;
		ReleaseCapture();

		UpdateCursor();
	}
	else
	{
		INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			if (GetFocus()!=this)
				SetFocus();
		}
		else
		{
			SelectItem(-1, FALSE);
		}
	}
}

void CGlobeView::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		SelectItem(Index, TRUE);
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CGlobeView::OnRButtonUp(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if ((Index!=-1) && (GetFocus()!=this))
		SetFocus();

	SelectItem(Index, Index!=-1);

	CFrontstageWnd::OnRButtonUp(nFlags, point);
}

void CGlobeView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch (nChar)
	{
	case VK_ADD:
	case VK_OEM_PLUS:
	case VK_PRIOR:
		OnZoomIn();
		break;

	case VK_SUBTRACT:
	case VK_OEM_MINUS:
	case VK_NEXT:
		OnZoomOut();
		break;

	case VK_HOME:
		OnAutosize();
		break;

	case 'L':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnJumpToLocation();

		break;
	}
}

void CGlobeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		UpdateScene();

	if (m_MoveCounter<1000)
		m_MoveCounter++;

	CFrontstageWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void CGlobeView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	m_Momentum = 0.0f;

	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu Menu;
	Menu.LoadMenu(((m_FocusItem!=-1) && m_IsSelected) ? IDM_GLOBEWND_ITEM : IDM_GLOBEWND);
	ASSERT_VALID(&Menu);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
}


void CGlobeView::OnJumpToLocation()
{
	FMSelectLocationIATADlg dlg(this);
	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.p_Airport);

		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_GlobeCurrent.Latitude;
		m_AnimStartLongitude = m_GlobeCurrent.Longitude;
		m_GlobeTarget.Latitude = (GLfloat)dlg.p_Airport->Location.Latitude;
		m_GlobeTarget.Longitude = -(GLfloat)dlg.p_Airport->Location.Longitude;
		m_Momentum = 0.0f;

		UpdateScene();
	}
}

void CGlobeView::OnZoomIn()
{
	if (m_GlobeTarget.Zoom>0)
	{
		m_GlobeTarget.Zoom -= 100;

		UpdateScene();
	}
}

void CGlobeView::OnZoomOut()
{
	if (m_GlobeTarget.Zoom<1000)
	{
		m_GlobeTarget.Zoom += 100;

		UpdateScene();
	}
}

void CGlobeView::OnAutosize()
{
	m_GlobeTarget.Zoom = 600;

	UpdateScene();
}

void CGlobeView::OnSaveAs()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_KML, _T("kml"), TRUE);

	CFileDialog dlg(FALSE, _T("kml"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		CGoogleEarthFile f;

		if (!f.Open(dlg.GetPathName(), m_DisplayName))
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}
		else
		{
			try
			{
				for (UINT a=0; a<m_Routes.m_ItemCount; a++)
					f.WriteRoute(m_Routes[a], m_MinRouteCount, m_MaxRouteCount, theApp.m_GoogleEarthUseCount, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClampHeight, FALSE);

				for (UINT a=0; a<m_Airports.m_ItemCount; a++)
					f.WriteAirport(m_Airports[a].pAirport);

				f.Close();
			}
			catch(CFileException ex)
			{
				f.Close();
				FMErrorBox(this, IDS_DRIVENOTREADY);
			}
		}
	}
}

void CGlobeView::OnGoogleEarth()
{
	if ((m_FocusItem!=-1) && (m_IsSelected))
		theApp.OpenAirportGoogleEarth(m_Airports[m_FocusItem].pAirport);
}

void CGlobeView::OnLiquidFolders()
{
	if ((m_FocusItem!=-1) && (m_IsSelected))
		theApp.OpenAirportLiquidFolders(m_Airports[m_FocusItem].pAirport->Code);
}

void CGlobeView::OnSettings()
{
	GlobeOptionsDlg dlg(this);
	dlg.DoModal();
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_RenderContext.hRC!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBEWND_ZOOMIN:
		bEnable &= m_GlobeTarget.Zoom>0;
		break;

	case IDM_GLOBEWND_ZOOMOUT:
		bEnable &= m_GlobeTarget.Zoom<1000;
		break;

	case IDM_GLOBEWND_AUTOSIZE:
		bEnable &= m_GlobeTarget.Zoom!=600;
		break;

	case IDM_GLOBEWND_SETTINGS:
		bEnable = TRUE;
		break;

	case IDM_GLOBEWND_SAVEAS:
		bEnable = m_Routes.m_ItemCount>0;
		break;

	case IDM_GLOBEWND_GOOGLEEARTH:
		bEnable = (m_FocusItem!=-1) && (m_IsSelected) && (theApp.m_PathGoogleEarth[0]!=L'\0');
		break;

	case IDM_GLOBEWND_LIQUIDFOLDERS:
		bEnable = (m_FocusItem!=-1) && (m_IsSelected) && (!theApp.m_PathLiquidFolders.IsEmpty());
		break;
	}

	pCmdUI->Enable(bEnable);
}
