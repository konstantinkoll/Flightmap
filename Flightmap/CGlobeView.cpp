
// CGlobeView.cpp: Implementierung der Klasse CGlobeView
//

#include "stdafx.h"
#include "CGlobeView.h"
#include "Resource.h"
#include "GlobeOptionsDlg.h"
#include <math.h>

#define DISTANCE       39.0f
#define ARROWSIZE      9
#define PI             3.14159265358979323846
#define ANIMLENGTH     200
#define MOVEDELAY      10
#define MOVEDIVIDER    8.0f
#define SPOT           2
#define CROSSHAIRS     3

__forceinline void ColorRef2GLColor(GLfloat* dst, COLORREF src, GLfloat Alpha=1.0f)
{
	dst[0] = (src & 0xFF)/255.0f;
	dst[1] = ((src>>8) & 0xFF)/255.0f;
	dst[2] = ((src>>16) & 0xFF)/255.0f;
	dst[3] = Alpha;
}

__forceinline double decToRad(double dec)
{
	return dec*(PI/180.0);
}

__forceinline void MatrixMul(GLfloat Result[4][4], GLfloat Left[4][4], GLfloat Right[4][4])
{
	Result[0][0] = Left[0][0]*Right[0][0] + Left[0][1]*Right[1][0] + Left[0][2]*Right[2][0] + Left[0][3]*Right[3][0];
	Result[0][1] = Left[0][0]*Right[0][1] + Left[0][1]*Right[1][1] + Left[0][2]*Right[2][1] + Left[0][3]*Right[3][1];
	Result[0][2] = Left[0][0]*Right[0][2] + Left[0][1]*Right[1][2] + Left[0][2]*Right[2][2] + Left[0][3]*Right[3][2];
	Result[0][3] = Left[0][0]*Right[0][3] + Left[0][1]*Right[1][3] + Left[0][2]*Right[2][3] + Left[0][3]*Right[3][3];
	Result[1][0] = Left[1][0]*Right[0][0] + Left[1][1]*Right[1][0] + Left[1][2]*Right[2][0] + Left[1][3]*Right[3][0];
	Result[1][1] = Left[1][0]*Right[0][1] + Left[1][1]*Right[1][1] + Left[1][2]*Right[2][1] + Left[1][3]*Right[3][1];
	Result[1][2] = Left[1][0]*Right[0][2] + Left[1][1]*Right[1][2] + Left[1][2]*Right[2][2] + Left[1][3]*Right[3][2];
	Result[1][3] = Left[1][0]*Right[0][3] + Left[1][1]*Right[1][3] + Left[1][2]*Right[2][3] + Left[1][3]*Right[3][3];
	Result[2][0] = Left[2][0]*Right[0][0] + Left[2][1]*Right[1][0] + Left[2][2]*Right[2][0] + Left[2][3]*Right[3][0];
	Result[2][1] = Left[2][0]*Right[0][1] + Left[2][1]*Right[1][1] + Left[2][2]*Right[2][1] + Left[2][3]*Right[3][1];
	Result[2][2] = Left[2][0]*Right[0][2] + Left[2][1]*Right[1][2] + Left[2][2]*Right[2][2] + Left[2][3]*Right[3][2];
	Result[2][3] = Left[2][0]*Right[0][3] + Left[2][1]*Right[1][3] + Left[2][2]*Right[2][3] + Left[2][3]*Right[3][3];
	Result[3][0] = Left[3][0]*Right[0][0] + Left[3][1]*Right[1][0] + Left[3][2]*Right[2][0] + Left[3][3]*Right[3][0];
	Result[3][1] = Left[3][0]*Right[0][1] + Left[3][1]*Right[1][1] + Left[3][2]*Right[2][1] + Left[3][3]*Right[3][1];
	Result[3][2] = Left[3][0]*Right[0][2] + Left[3][1]*Right[1][2] + Left[3][2]*Right[2][2] + Left[3][3]*Right[3][2];
	Result[3][3] = Left[3][0]*Right[0][3] + Left[3][1]*Right[1][3] + Left[3][2]*Right[2][3] + Left[3][3]*Right[3][3];
}

__forceinline void CalculateWorldCoords(double lat, double lon, GLfloat result[])
{
	double lon_r = decToRad(lon);
	double lat_r = -decToRad(lat);

	double c = cos(lat_r);

	result[0] = (GLfloat)(cos(lon_r)*c);
	result[1] = (GLfloat)(sin(lon_r)*c);
	result[2] = (GLfloat)(sin(lat_r));
}

__forceinline BOOL SetupPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		32,								// 32-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0,						// accum bits ignored
		0,								// no z-buffer
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
	};

	INT PixelFormat = ChoosePixelFormat(hDC, &pfd);
	return PixelFormat ? SetPixelFormat(hDC, PixelFormat, &pfd) : FALSE;
}

void glEnable2D()
{
	GLint iViewport[4];
	glGetIntegerv(GL_VIEWPORT, iViewport);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(iViewport[0], iViewport[0]+iViewport[2], iViewport[1]+iViewport[3], iViewport[1], -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0);

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
}

void glDisable2D()
{
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void glDrawIcon(GLfloat x, GLfloat y, GLfloat Size, GLfloat Alpha, UINT ID)
{
	x -= 0.375;
	y -= 0.375;
	Size /= 2.0;

	GLfloat s = (ID%2) ? 0.5f : 0.0f;
	GLfloat t = (ID/2) ? 0.5f : 0.0f;

	glColor4f(1.0f, 1.0f, 1.0f, Alpha);

	glTexCoord2d(s, t);
	glVertex2d(x-Size, y-Size);
	glTexCoord2d(s+0.5, t);
	glVertex2d(x+Size, y-Size);
	glTexCoord2d(s+0.5, t+0.5);
	glVertex2d(x+Size, y+Size);
	glTexCoord2d(s, t+0.5);
	glVertex2d(x-Size, y+Size);
}


// CGlobeView
//

CGlobeView::CGlobeView()
	: CWnd()
{
	m_pDC = NULL;
	hRC = NULL;

	lpszCursorName = IDC_WAIT;
	hCursor = theApp.LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;

	m_FocusItem = m_HotItem = -1;
	m_Width = m_Height = 0;
	m_GlobeModel = -1;
	m_TextureGlobe = m_TextureIcons = NULL;
	m_CurrentGlobeTexture = -1;
	m_Scale = 1.0f;
	m_Radius = m_Momentum = 0.0f;
	m_IsSelected = m_Grabbed = m_Hover = m_LockUpdate = FALSE;
	m_AnimCounter = m_MoveCounter = 0;

	ENSURE(m_YouLookAt.LoadString(IDS_YOULOOKAT));
	ENSURE(m_FlightCount_Singular.LoadString(IDS_FLIGHTCOUNT_SINGULAR));
	ENSURE(m_FlightCount_Plural.LoadString(IDS_FLIGHTCOUNT_PLURAL));

	m_FlightAirports.InitHashTable(2048);
	m_FlightAirportCounts.InitHashTable(2048);
}

BOOL CGlobeView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS | CS_OWNDC, LoadCursor(NULL, IDC_ARROW));

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;

	CRect rect;
	rect.SetRectEmpty();
	if (!CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID))
		return FALSE;

	UpdateViewOptions(TRUE);
	return TRUE;
}

FMAirport* CGlobeView::AddAirport(CHAR* Code)
{
	ASSERT(Code);

	if (strlen(Code)!=3)
		return NULL;

	FMAirport* pAirport;
	if (m_FlightAirports.Lookup(Code, pAirport))
	{
		m_FlightAirportCounts[Code]++;
		return pAirport;
	}

	if (!FMIATAGetAirportByCode(Code, &pAirport))
		return NULL;

	m_FlightAirports[Code] = pAirport;
	m_FlightAirportCounts[Code] = 1;

	return pAirport;
}

void CGlobeView::AddFlight(CHAR* From, CHAR* To, COLORREF Color)
{
	FMAirport* pFrom = AddAirport(From);
	FMAirport* pTo = AddAirport(To);
}

void CGlobeView::CalcFlights()
{
	// Airports
	CFlightAirports::CPair* pPair = m_FlightAirports.PGetFirstAssoc();
	while (pPair)
	{
		GlobeAirport ga;
		ZeroMemory(&ga, sizeof(ga));
		ga.pAirport = pPair->value;

		strcpy_s(ga.NameString, 130, ga.pAirport->Name);
		FMCountry* Country = FMIATAGetCountry(ga.pAirport->CountryID);
		if (Country)
		{
			strcat_s(ga.NameString, 130, ", ");
			strcat_s(ga.NameString, 130, Country->Name);
		}

		FMGeoCoordinatesToString(ga.pAirport->Location, ga.CoordString, 32, FALSE);

		CalculateWorldCoords(ga.pAirport->Location.Latitude, ga.pAirport->Location.Longitude, ga.World);

		UINT Cnt = 0;
		m_FlightAirportCounts.Lookup(ga.pAirport->Code, Cnt);
		CString tmpStr;
		tmpStr.Format(Cnt==1 ? m_FlightCount_Singular : m_FlightCount_Plural, Cnt);
		wcscpy_s(ga.CountString, 64, tmpStr.GetBuffer());

		m_Airports.AddItem(ga);
		pPair = m_FlightAirports.PGetNextAssoc(pPair);
	}

	// Routes

	// Flush
	m_FlightAirports.RemoveAll();
	m_FlightAirportCounts.RemoveAll();
}

void CGlobeView::UpdateViewOptions(BOOL Force)
{
	m_TooltipCtrl.Deactivate();

	if (Force)
	{
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude = theApp.m_GlobeLatitude/1000.0f;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude = theApp.m_GlobeLongitude/1000.0f;
		m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom = theApp.m_GlobeZoom;
	}

	PrepareTexture();

	if (Force || (m_IsHQModel!=theApp.m_GlobeHQModel))
	{
		PrepareModel();
		m_IsHQModel = theApp.m_GlobeHQModel;
	}

	Invalidate();
}

INT CGlobeView::ItemAtPosition(CPoint point)
{
	INT res = -1;
	GLfloat Alpha = 0.0f;

	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		GlobeAirport* ga = &m_Airports.m_Items[a];

		if ((ga->Alpha>0.75f) || ((ga->Alpha>0.1f) && (ga->Alpha>Alpha-0.05f)))
			if (PtInRect(&ga->Rect, point))
			{
				res = a;
				Alpha = ga->Alpha;
			}
	}

	return res;
}

void CGlobeView::InvalidateItem(INT idx)
{
	if ((idx>=0) && (idx<(INT)m_Airports.m_ItemCount))
		InvalidateRect(&m_Airports.m_Items[idx].Rect);
}

void CGlobeView::SelectItem(INT idx, BOOL Select)
{
	if ((idx!=m_FocusItem) || (Select!=m_IsSelected))
	{
		InvalidateItem(m_FocusItem);
		m_FocusItem = idx;
		m_IsSelected = Select;
		InvalidateItem(m_FocusItem);
	}
}

BOOL CGlobeView::CursorOnGlobe(CPoint point)
{
	GLfloat distX = point.x-(GLfloat)m_Width/2;
	GLfloat distY = point.y-(GLfloat)m_Height/2;

	return distX*distX+distY*distY<m_Radius*m_Radius;
}

void CGlobeView::UpdateCursor()
{
	LPCTSTR csr;
	if (m_Grabbed)
	{
		csr = IDC_HAND;
	}
	else
	{
		csr = IDC_ARROW;

		if (CursorOnGlobe(m_CursorPos))
			if (ItemAtPosition(m_CursorPos)==-1)
				csr = IDC_HAND;
	}

	if (csr!=lpszCursorName)
	{
		hCursor = theApp.LoadStandardCursor(csr);

		SetCursor(hCursor);
		lpszCursorName = csr;
	}
}


// OpenGL
//

__forceinline void CGlobeView::PrepareModel()
{
	// 3D-Modelle einbinden
	#include "Globe_Low.h"
	#include "Globe_High.h"
	UINT Count = (theApp.m_GlobeHQModel ? GlobeHighCount : GlobeLowCount);
	GLfloat* Nodes = (theApp.m_GlobeHQModel ? &GlobeHighNodes[0] : &GlobeLowNodes[0]);

	// Display-Liste für das 3D-Modell erstellen
	m_LockUpdate = TRUE;
	wglMakeCurrent(*m_pDC, hRC);

	m_GlobeModel = glGenLists(1);
	glNewList(m_GlobeModel, GL_COMPILE);
	glEnable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);

	UINT Pos = 0;
	for (UINT a=0; a<Count; a++)
	{
		GLfloat s = Nodes[Pos++];
		GLfloat t = Nodes[Pos++];
		glTexCoord2f(s, t);

		GLfloat x = Nodes[Pos++];
		GLfloat y = Nodes[Pos++];
		GLfloat z = Nodes[Pos++];
		glNormal3f(x, y, z);
		glVertex3f(x, y, z);
	}

	glEnd();
	glDisable(GL_CULL_FACE);
	glEndList();

	m_LockUpdate = FALSE;
}

__forceinline void CGlobeView::PrepareTexture()
{
	// Automatisch höchstens 4096x4096 laden, da quadratisch und von den meisten Grafikkarten unterstützt
	UINT Tex = theApp.m_nTextureSize;
	if (Tex==FMTextureAuto)
		Tex = FMTexture4096;

	// Texture prüfen
	GLint TexSize = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &TexSize);

Smaller:
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, TexSize, TexSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	GLint ProxySize = 0;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ProxySize);

	if ((ProxySize==0) && (TexSize>1024))
	{
		TexSize /= 2;
		goto Smaller;
	}

	theApp.m_nMaxTextureSize = (TexSize>=8192) ? FMTexture8192 : (TexSize>=4096) ? FMTexture4096 : (TexSize>=2048) ? FMTexture2048 : FMTexture1024;
	if (Tex>theApp.m_nMaxTextureSize)
		Tex = theApp.m_nMaxTextureSize;

	if ((INT)Tex!=m_CurrentGlobeTexture)
	{
		SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
		m_LockUpdate = TRUE;

		wglMakeCurrent(*m_pDC, hRC);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		m_TextureGlobe = new GLTextureBlueMarble(Tex);
		m_CurrentGlobeTexture = Tex;

		m_LockUpdate = FALSE;
		SetCursor(hCursor);

		Invalidate();
	}
}

__forceinline void CGlobeView::Normalize()
{
	// Zoom
	if (m_GlobeTarget.Zoom<0)
		m_GlobeTarget.Zoom = 0;
	if (m_GlobeTarget.Zoom>1000)
		m_GlobeTarget.Zoom = 1000;

	// Nicht über die Pole rollen
	if (m_GlobeTarget.Latitude<-75.0f)
		m_GlobeTarget.Latitude = -75.0f;
	if (m_GlobeTarget.Latitude>75.0f)
		m_GlobeTarget.Latitude = 75.0f;

	// Rotation normieren
	if (m_GlobeTarget.Longitude<0.0f)
		m_GlobeTarget.Longitude += 360.0f;
	if (m_GlobeTarget.Longitude>360.0f)
		m_GlobeTarget.Longitude -= 360.0f;
}

__forceinline void CGlobeView::CalcAndDrawSpots(GLfloat ModelView[4][4], GLfloat Projection[4][4])
{
	GLfloat SizeX = m_Width/2.0f;
	GLfloat SizeY = m_Height/2.0f;

	GLfloat MVP[4][4];
	MatrixMul(MVP, ModelView, Projection);

	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		GlobeAirport* ga = &m_Airports.m_Items[a];
		ga->Alpha = 0.0f;

		GLfloat z = ModelView[0][2]*ga->World[0] + ModelView[1][2]*ga->World[1] + ModelView[2][2]*ga->World[2];
		if ((z>m_FogEnd) && (m_Width) && (m_Height))
		{
			GLfloat w = MVP[0][3]*ga->World[0] + MVP[1][3]*ga->World[1] + MVP[2][3]*ga->World[2] + MVP[3][3];
			GLfloat x = (MVP[0][0]*ga->World[0] + MVP[1][0]*ga->World[1] + MVP[2][0]*ga->World[2] + MVP[3][0])*SizeX/w + SizeX + 0.5f;
			GLfloat y = -(MVP[0][1]*ga->World[0] + MVP[1][1]*ga->World[1] + MVP[2][1]*ga->World[2] + MVP[3][1])*SizeY/w + SizeY + 0.5f;

			ga->ScreenPoint[0] = (INT)x;
			ga->ScreenPoint[1] = (INT)y;
			ga->Alpha = 1.0f;
			if (z<m_FogStart)
				ga->Alpha -= (GLfloat)((m_FogStart-z)/(m_FogStart-m_FogEnd));

			if (theApp.m_GlobeShowSpots)
				glDrawIcon(x, y, 6.0f+8.0f*ga->Alpha, ga->Alpha, SPOT);
		}
	}
}

__forceinline void CGlobeView::CalcAndDrawLabel()
{
	for (UINT a=0; a<m_Airports.m_ItemCount; a++)
	{
		GlobeAirport* ga = &m_Airports.m_Items[a];

		if (ga->Alpha>0.0f)
		{
			// Beschriftung
			CHAR* Caption = ga->pAirport->Code;
			CHAR* Subcaption = (theApp.m_GlobeShowAirportNames ? ga->NameString : NULL);
			CHAR* Coordinates = (theApp.m_GlobeShowGPS ? ga->CoordString : NULL);
			WCHAR* Count = (theApp.m_GlobeShowFlightCount ? ga->CountString : NULL);

			DrawLabel(ga, Caption, Subcaption, Coordinates, Count, m_FocusItem==(INT)a);
		}
	}
}

__forceinline void CGlobeView::DrawLabel(GlobeAirport* ga, CHAR* Caption, CHAR* Subcaption, CHAR* Coordinates, WCHAR* Description, BOOL Focused)
{
	ASSERT(ARROWSIZE>3);

	// Breite
	UINT W1 = m_Fonts[1].GetTextWidth(Caption);
	UINT W2 = m_Fonts[0].GetTextWidth(Subcaption);
	UINT W3 = m_Fonts[0].GetTextWidth(Coordinates);
	UINT W4 = m_Fonts[0].GetTextWidth(Description);
	UINT Width = max(W1, max(W2, max(W3, W4)))+8;

	// Höhe
	UINT Height = 3;
	Height += m_Fonts[1].GetTextHeight(Caption);
	Height += m_Fonts[0].GetTextHeight(Subcaption);
	Height += m_Fonts[0].GetTextHeight(Coordinates);
	Height += m_Fonts[0].GetTextHeight(Description);

	// Position
	INT top = (ga->ScreenPoint[1]<m_Height/2) ? -1 : 1;

	INT x = ga->Rect.left = ga->ScreenPoint[0]-ARROWSIZE-(((INT)Width-2*ARROWSIZE)*(m_Width-ga->ScreenPoint[0])/m_Width);
	INT y = ga->Rect.top = ga->ScreenPoint[1]+(ARROWSIZE-2)*top-(top<0 ? (INT)Height : 0);
	ga->Rect.right = x+Width;
	ga->Rect.bottom = y+Height;

	// Sichtbar?
	if ((x+Width+6<0) || (x-1>m_Width) || (y+Height+ARROWSIZE+6<0) || (y-ARROWSIZE-6>m_Height))
	{
		ga->Alpha = 0.0f;
		return;
	}

	// Farben
	COLORREF BaseColorRef = GetSysColor(COLOR_WINDOW);
	COLORREF TextColorRef = GetSysColor(COLOR_WINDOWTEXT);
	if (Focused && m_IsSelected)
		if (this==GetFocus())
		{
			BaseColorRef = GetSysColor(COLOR_HIGHLIGHT);
			TextColorRef = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			BaseColorRef = GetSysColor(COLOR_BTNFACE);
			TextColorRef = GetSysColor(COLOR_BTNTEXT);
		}

	GLfloat BaseColor[4];
	ColorRef2GLColor(&BaseColor[0], BaseColorRef);
	GLfloat TextColor[4];
	ColorRef2GLColor(&TextColor[0], TextColorRef);

	// Schatten
	if (theApp.m_GlobeShadows)
	{
		for (INT s=3; s>0; s--)
		{
			glColor4f(0.0f, 0.0f, 0.0f, ga->Alpha/(s+2.5f));
			glBegin(GL_LINES);
			glVertex2i(x+2, y+Height+s);
			glVertex2i(x+Width+s, y+Height+s);
			glVertex2i(x+Width+s, y+2);
			glVertex2i(x+Width+s, y+Height+s+1);
			glEnd();
		}

		glColor4f(0.0f, 0.0f, 0.0f, ga->Alpha/2.5f);
		glBegin(GL_LINES);
		glVertex2i(x+Width, y+Height);
		glVertex2i(x+Width+1, y+Height);
	}
	else
	{
		glBegin(GL_LINES);
	}

	// Grauer Rand
	glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, ga->Alpha);
	glVertex2i(x, y-1);						// Oben
	glVertex2i(x+Width, y-1);
	glVertex2i(x, y+Height);				// Unten
	glVertex2i(x+Width, y+Height);
	glVertex2i(x-1, y);						// Links
	glVertex2i(x-1, y+Height);
	glVertex2i(x+Width, y);					// Rechts
	glVertex2i(x+Width, y+Height);
	glEnd();

	// Pfeil
	glBegin(GL_TRIANGLES);
	GLfloat alpha = pow(ga->Alpha, 3);
	for (INT a=0; a<=3; a++)
	{
		switch (a)
		{
		case 0:
			glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, alpha/2);
			break;
		case 1:
			glColor4f(BaseColor[0]/2, BaseColor[1]/2, BaseColor[2]/2, alpha);
			break;
		case 2:
			glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], alpha/2);
			break;
		default:
			glColor4f(BaseColor[0], BaseColor[1], BaseColor[2], ga->Alpha);
		}

		glVertex2i(ga->ScreenPoint[0], ga->ScreenPoint[1]+(a-2)*top);
		glVertex2i(ga->ScreenPoint[0]+(ARROWSIZE-a)*top, ga->ScreenPoint[1]+(ARROWSIZE-2)*top);
		glVertex2i(ga->ScreenPoint[0]-(ARROWSIZE-a)*top, ga->ScreenPoint[1]+(ARROWSIZE-2)*top);
	}
	glEnd();

	// Innen
	glRecti(x, y, x+(INT)Width, y+(INT)Height);
	if ((Focused) && (GetFocus()==this))
	{
		glColor4f(1.0f-BaseColor[0], 1.0f-BaseColor[1], 1.0f-BaseColor[2], ga->Alpha);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xAAAA);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x, y);
		glVertex2i(x+Width-1, y);
		glVertex2i(x+Width-1, y+Height-1);
		glVertex2i(x, y+Height-1);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	x += 3;

	glColor4f(TextColor[0], TextColor[1], TextColor[2], ga->Alpha);
	y += m_Fonts[1].Render(Caption, x, y);
	if (Subcaption)
		y += m_Fonts[0].Render(Subcaption, x, y);

	if (!(Focused && m_IsSelected) && (BaseColorRef==0xFFFFFF) && (TextColorRef==0x000000))
		glColor4f(TextColor[0], TextColor[1], TextColor[2], ga->Alpha/2);
	if (Coordinates)
		y += m_Fonts[0].Render(Coordinates, x, y);
	if (Description)
		y += m_Fonts[0].Render(Description, x, y);
}

__forceinline void CGlobeView::DrawStatusBar(INT Height)
{
	WCHAR Copyright[] = L"© NASA's Earth Observatory";
	INT CopyrightWidth = (INT)m_Fonts[0].GetTextWidth(Copyright);
	if (m_Width<CopyrightWidth)
		return;

	WCHAR Viewpoint[256] = L"";
	INT ViewpointWidth = -1;
	if (theApp.m_GlobeShowViewport)
	{
		FMGeoCoordinates c;
		c.Latitude = -m_GlobeCurrent.Latitude;
		c.Longitude = (m_GlobeCurrent.Longitude>180.0) ? 360-m_GlobeCurrent.Longitude : -m_GlobeCurrent.Longitude;

		CString Coord;
		FMGeoCoordinatesToString(c, Coord, TRUE);

		swprintf(Viewpoint, 256, m_YouLookAt, Coord);

		ViewpointWidth = (INT)m_Fonts[0].GetTextWidth(Viewpoint);
		if (m_Width<CopyrightWidth+ViewpointWidth+48)
			ViewpointWidth = -1;
	}

	// Kante
	glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
	glBegin(GL_LINES);
	glVertex2i(0, m_Height-Height);
	glVertex2i(m_Width, m_Height-Height);
	glEnd();

	// Füllen
	glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
	glRecti(0, m_Height-Height, m_Width, m_Height);

	// Text
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	INT Gutter = (ViewpointWidth>0) ? (m_Width-CopyrightWidth-ViewpointWidth)/3 : (m_Width-CopyrightWidth)/2;

	m_Fonts[0].Render(Copyright, Gutter, m_Height-Height);
	if (ViewpointWidth>0)
		m_Fonts[0].Render(Viewpoint, m_Width-ViewpointWidth-Gutter, m_Height-Height);
}

void CGlobeView::DrawScene(BOOL InternalCall)
{
	if (!InternalCall)
	{
		if (m_LockUpdate)
			return;
		m_LockUpdate = TRUE;
	}

	wglMakeCurrent(*m_pDC, hRC);
	glRenderMode(GL_RENDER);

	// Hintergrund
	GLfloat BackColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glFogfv(GL_FOG_COLOR, BackColor);

	glClearColor(BackColor[0], BackColor[1], BackColor[2], 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Globus berechnen
	m_Scale = 1.0f;
	if (m_Height>m_Width)
		m_Scale = 1-((GLfloat)(m_Height-m_Width))/m_Height;

	GLfloat zoomfactor = ((m_GlobeCurrent.Zoom+400)/1000.0f);
	m_Scale /= zoomfactor*zoomfactor;
	m_Radius = 0.49f*m_Height*m_Scale;
	m_FogStart = 0.40f*m_Scale;
	m_FogEnd = 0.025f*m_Scale;

	// Globus zeichnen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(DISTANCE, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	// Beleuchtung mit FESTER Lichtquelle
	if (theApp.m_GlobeLighting)
	{
		GLfloat lAmbient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat lSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lSpecular);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbient);
	}

	// Rotationsmatrix (erst NACH Lichtquelle)
	glRotatef(m_GlobeCurrent.Latitude, 0.0f, 1.0f, 0.0f);
	glRotatef(m_GlobeCurrent.Longitude, 0.0f, 0.0f, 1.0f);
	glScalef(m_Scale, m_Scale, m_Scale);

	// Atmosphäre/Nebel
	if (theApp.m_GlobeAtmosphere)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_START, DISTANCE-m_FogStart);
		glFogf(GL_FOG_END, DISTANCE-m_FogEnd);
	}

	// Globus-Textur
	if (m_TextureGlobe)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_TextureGlobe->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, theApp.m_GlobeLighting ? GL_MODULATE : GL_REPLACE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	// Modell rendern
	glCallList(m_GlobeModel);

	// Atmosphäre aus
	if (theApp.m_GlobeAtmosphere)
		glDisable(GL_FOG);

	// Licht aus
	if (theApp.m_GlobeLighting)
	{
		glDisable(GL_NORMALIZE);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	// Matritzen speichern
	GLfloat ModelView[4][4];
	GLfloat Projection[4][4];
	glGetFloatv(GL_MODELVIEW_MATRIX, &ModelView[0][0]);
	glGetFloatv(GL_PROJECTION_MATRIX, &Projection[0][0]);

	// Für Icons vorbereiten
	if (m_TextureIcons)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_TextureIcons->GetID());
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
	glEnable2D();
	glBegin(GL_QUADS);

	// Koordinaten bestimmen und Spots zeichnen
	if (m_Airports.m_ItemCount)
		CalcAndDrawSpots(ModelView, Projection);

	// Fadenkreuz zeichnen
	if (theApp.m_GlobeShowViewport && theApp.m_GlobeShowCrosshairs)
		glDrawIcon(m_Width/2.0f, m_Height/2.0f, 64.0f, 1.0f, CROSSHAIRS);

	// Icons beenden
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// Label zeichnen
	if (m_Airports.m_ItemCount)
		CalcAndDrawLabel();

	// Statuszeile
	const INT Height = m_Fonts[0].GetTextHeight("Wy")+1;
	if (m_Height>=Height)
		DrawStatusBar(Height);

	// Beenden
	glDisable2D();

	SwapBuffers(*m_pDC);
	m_LockUpdate = FALSE;
}

BOOL CGlobeView::UpdateScene(BOOL Redraw)
{
	if (m_LockUpdate)
		return FALSE;
	m_LockUpdate = TRUE;

	BOOL res = Redraw;
	Normalize();

	// Zoom
	if (m_GlobeCurrent.Zoom<=m_GlobeTarget.Zoom-5)
	{
		res = TRUE;
		m_GlobeCurrent.Zoom += 5;
		m_HotItem = -1;
	}
	else
		if (m_GlobeCurrent.Zoom>=m_GlobeTarget.Zoom+5)
		{
			res = TRUE;
			m_GlobeCurrent.Zoom -= 5;
			m_HotItem = -1;
		}
		else
		{
			res |= (m_GlobeCurrent.Zoom!=m_GlobeTarget.Zoom);
			m_GlobeCurrent.Zoom = m_GlobeTarget.Zoom;
		}

	// Animation
	if (m_AnimCounter)
	{
		GLfloat f = (GLfloat)((cos(PI*m_AnimCounter/ANIMLENGTH)+1.0)/2.0);
		m_GlobeCurrent.Latitude = m_AnimStartLatitude*(1.0f-f) + m_GlobeTarget.Latitude*f;
		m_GlobeCurrent.Longitude = m_AnimStartLongitude*(1.0f-f) + m_GlobeTarget.Longitude*f;

		if (m_GlobeTarget.Zoom<600)
		{
			INT Dist = 600-m_GlobeTarget.Zoom;
			INT MaxDist = (INT)((m_GlobeTarget.Zoom+100)*1.2f);
			if (Dist>MaxDist)
				Dist = MaxDist;

			GLfloat f = (GLfloat)sin(PI*m_AnimCounter/ANIMLENGTH);
			m_GlobeCurrent.Zoom = (INT)(m_GlobeTarget.Zoom*(1.0f-f)+(m_GlobeTarget.Zoom+Dist)*f);
		}

		res = TRUE;
		m_AnimCounter--;
	}
	else
	{
		if (m_Momentum!=0.0f)
			m_GlobeTarget.Longitude += m_Momentum;

		res |= (m_GlobeCurrent.Latitude!=m_GlobeTarget.Latitude) || (m_GlobeCurrent.Longitude!=m_GlobeTarget.Longitude);
		m_GlobeCurrent.Latitude = m_GlobeTarget.Latitude;
		m_GlobeCurrent.Longitude = m_GlobeTarget.Longitude;
	}

	if (res)
	{
		DrawScene(TRUE);
		UpdateCursor();
	}
	else
	{
		m_LockUpdate = FALSE;
	}

	return res;
}


BEGIN_MESSAGE_MAP(CGlobeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	/*ON_COMMAND(IDM_GLOBE_JUMPTOLOCATION, OnJumpToLocation)
	ON_COMMAND(IDM_GLOBE_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_GLOBE_ZOOMOUT, OnZoomOut)
	ON_COMMAND(IDM_GLOBE_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_GLOBE_SETTINGS, OnSettings)
	ON_COMMAND(IDM_GLOBE_GOOGLEEARTH, OnGoogleEarth)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_GLOBE_JUMPTOLOCATION, IDM_GLOBE_GOOGLEEARTH, OnUpdateCommands)*/
END_MESSAGE_MAP()

INT CGlobeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_TooltipCtrl.Create(this);

	m_pDC = new CClientDC(this);
	if (!m_pDC)
		return -1;

	if (!SetupPixelFormat(*m_pDC))
		return -1;

	hRC = wglCreateContext(*m_pDC);
	wglMakeCurrent(*m_pDC, hRC);

	// 3D-Einstellungen
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_NICEST);

	// Fonts
	m_Fonts[0].Create(&theApp.m_DefaultFont);
	m_Fonts[1].Create(&theApp.m_LargeFont);

	// Icons
	CGdiPlusBitmapResource Tex0(IDB_GLOBEICONS_RGB, _T("PNG"));
	CGdiPlusBitmapResource Tex1(IDB_GLOBEICONS_ALPHA, _T("PNG"));
	m_TextureIcons = new GLTextureCombine(&Tex0, &Tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Animations-Timer
	SetTimer(1, 10, NULL);

	return 0;
}

void CGlobeView::OnDestroy()
{
	KillTimer(1);

	if (m_pDC)
	{
		wglMakeCurrent(*m_pDC, hRC);

		if (m_TextureGlobe)
			delete m_TextureGlobe;
		if (m_TextureIcons)
			delete m_TextureIcons;
		if (m_GlobeModel!=-1)
			glDeleteLists(m_GlobeModel, 1);

		wglMakeCurrent(NULL, NULL);
		if (hRC)
			wglDeleteContext(hRC);
		delete m_pDC;
	}

	theApp.m_GlobeLatitude = (INT)(m_GlobeTarget.Latitude*1000.0f);
	theApp.m_GlobeLongitude = (INT)(m_GlobeTarget.Longitude*1000.0f);
	theApp.m_GlobeZoom = m_GlobeTarget.Zoom;

	CWnd::OnDestroy();
}

void CGlobeView::OnSysColorChange()
{
	Invalidate();
}

BOOL CGlobeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGlobeView::OnPaint()
{
	CPaintDC pDC(this);
	DrawScene();
}

void CGlobeView::OnSize(UINT nType, INT cx, INT cy)
{
	if (cy>0)
	{
		m_Width = cx;
		m_Height = cy;

		wglMakeCurrent(*m_pDC, hRC);
		glViewport(0, 0, cx, cy);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(3.0f, (GLfloat)cx/cy, 0.1f, 500.0f);
	}

	SetRedraw(FALSE);
	CWnd::OnSize(nType, cx, cy);
	SetRedraw(TRUE);

	Invalidate();
}

BOOL CGlobeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
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

		CSize rotate = m_GrabPoint - point;
		m_GrabPoint = point;

		m_LastMove = -rotate.cx/m_Scale*0.12f;
		m_GlobeTarget.Longitude = m_GlobeCurrent.Longitude += m_LastMove;
		m_GlobeTarget.Latitude = m_GlobeCurrent.Latitude -= rotate.cy/m_Scale*0.12f;

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
		tme.dwHoverTime = FMHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
			m_TooltipCtrl.Deactivate();

	if (m_HotItem!=Item)
		m_HotItem = Item;
}

BOOL CGlobeView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
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

void CGlobeView::OnMouseHover(UINT nFlags, CPoint point)
{
	if (m_Momentum==0.0f)
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
			if (m_HotItem!=-1)
			{
				if (!m_TooltipCtrl.IsWindowVisible())
				{
					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, m_Airports.m_Items[m_HotItem].pAirport, m_Airports.m_Items[m_HotItem].CountString);
				}
			}
			else
			{
				m_TooltipCtrl.Deactivate();
			}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = FMHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CGlobeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	INT idx = ItemAtPosition(point);
	if (idx==-1)
	{
		if (CursorOnGlobe(point))
		{
			m_GrabPoint = point;
			m_Grabbed = TRUE;
			m_Momentum = m_LastMove = 0.0f;

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
		SelectItem(idx, (nFlags & MK_CONTROL) && (m_FocusItem==idx) && m_IsSelected ? !m_IsSelected : TRUE);
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
		INT idx = ItemAtPosition(point);
		if (idx!=-1)
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
	INT idx = ItemAtPosition(point);
	if (idx!=-1)
	{
		SelectItem(idx, TRUE);
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CGlobeView::OnRButtonUp(UINT nFlags, CPoint point)
{
	INT idx = ItemAtPosition(point);
	if ((idx!=-1) && (GetFocus()!=this))
		SetFocus();

	SelectItem(idx, idx!=-1);

	GetParent()->UpdateWindow();
	CWnd::OnRButtonUp(nFlags, point);
}

void CGlobeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case 'N':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
		{
			m_IsSelected = FALSE;
			InvalidateItem(m_FocusItem);
		}
		break;
	case VK_SPACE:
		if (m_FocusItem!=-1)
		{
			m_IsSelected = (GetKeyState(VK_CONTROL)>=0) ? TRUE : !m_IsSelected;
			InvalidateItem(m_FocusItem);
		}
		break;
	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CGlobeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		UpdateScene();

	if (m_MoveCounter<1000)
		m_MoveCounter++;

	CWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void CGlobeView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CGlobeView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}


/*void CGlobeView::OnJumpToLocation()
{
	LFSelectLocationIATADlg dlg(IDD_JUMPTOIATA, this);

	if (dlg.DoModal()==IDOK)
	{
		ASSERT(dlg.m_Airport);

		m_AnimCounter = ANIMLENGTH;
		m_AnimStartLatitude = m_GlobeCurrent.Latitude;
		m_AnimStartLongitude = m_GlobeCurrent.Longitude;
		m_GlobeTarget.Latitude = (GLfloat)-dlg.m_Airport->Location.Latitude;
		m_GlobeTarget.Longitude = (GLfloat)-dlg.m_Airport->Location.Longitude;
		m_Momentum = 0.0f;

		UpdateScene();
	}
}*/

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

/*void CGlobeView::OnSettings()
{
	GlobeOptionsDlg dlg(this, p_ViewParameters, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(-1, LFViewGlobe);
}

void CGlobeView::OnGoogleEarth()
{
	// TODO
}

void CGlobeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_GLOBE_ZOOMIN:
		b = m_GlobeTarget.Zoom>0;
		break;
	case IDM_GLOBE_ZOOMOUT:
		b = m_GlobeTarget.Zoom<1000;
		break;
	case IDM_GLOBE_AUTOSIZE:
		b = m_GlobeTarget.Zoom!=600;
		break;
	case IDM_GLOBE_GOOGLEEARTH:
		b = (GetNextSelectedItem(-1)!=-1) && (!theApp.m_PathGoogleEarth.IsEmpty());
		break;
	}

	pCmdUI->Enable(b);
}*/
