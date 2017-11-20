
// CColorPicker.cpp: Implementierung der Klasse CColorPicker
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <math.h>


// CHueWheel
//

#define SHADOWSIZE     10

FMDynArray<HueWheelBitmaps, 2, 2> CHueWheel::m_Bitmaps;

CHueWheel::CHueWheel()
	: CWnd()
{
	m_Hue = 225.0;				// Deep sea blue
	m_Grabbed = FALSE;

	lpszCursorName = IDC_WAIT;
	hCursor = FMGetApp()->LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;
}

BOOL CHueWheel::Create(CWnd* pParentWnd, const CRect& rect, UINT nID)
{
	ASSERT(rect.Width()==rect.Height());

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_GROUP, rect, pParentWnd, nID);
}

void CHueWheel::SetColor(COLORREF clr)
{
	DOUBLE H;
	DOUBLE S;
	DOUBLE L;

	CDrawingManager::RGBtoHSL(clr, &H, &S, &L);
	SetHue(H*360.0);
}

void CHueWheel::SetHue(DOUBLE Hue)
{
	ASSERT(Hue>=0.0);
	ASSERT(Hue<=360.0);

	m_Hue = Hue;
	Invalidate();

	// Notify owner
	NM_HUEDATA tag;
	tag.hdr.code = HUEWHEEL_UPDATE_HUE;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.Hue = Hue;

	GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
}

inline void CHueWheel::CreateBitmaps(HueWheelBitmaps* pBitmaps)
{
	ASSERT(pBitmaps);

	const REAL Radius = (REAL)pBitmaps->Size/2.0f;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	// Wheel
	if (!pBitmaps->hBitmapWheel)
	{
		pBitmaps->hBitmapWheel = CreateTransparentBitmap(pBitmaps->Size, pBitmaps->Size);
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(pBitmaps->hBitmapWheel);

		for (INT Row=0; Row<pBitmaps->Size; Row++)
			for (INT Col=0; Col<pBitmaps->Size; Col++)
			{
				const REAL x = Radius-(REAL)Col;
				const REAL y = Radius-(REAL)Row;

				const REAL Distance = x*x+y*y;
				if ((Distance<=m_OuterRadiusSqDraw) && (Distance>=m_InnerRadiusSqDraw))
					dc.SetPixel(Col, Row, CDrawingManager::HLStoRGB_ONE(-atan2(x, y)/(2*PI), 0.5, 1.0));
			}

		dc.SelectObject(hOldBitmap);
	}

	// Top
	if (!pBitmaps->hBitmapTop)
	{
		pBitmaps->hBitmapTop = CreateTransparentBitmap(pBitmaps->Size, pBitmaps->Size);
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(pBitmaps->hBitmapTop);

		Graphics g(dc);
		g.SetSmoothingMode(FMGetApp()->m_SmoothingModeAntiAlias8x8);

		GraphicsPath Path;
		Path.AddRectangle(Rect(-SHADOWSIZE, -SHADOWSIZE, pBitmaps->Size+SHADOWSIZE, pBitmaps->Size+SHADOWSIZE));
		Path.AddArc(Radius-m_OuterRadius, Radius-m_OuterRadius, 2*m_OuterRadius, 2*m_OuterRadius, 0.0f, 360.0f);
		Path.AddArc(Radius-m_InnerRadius, Radius-m_InnerRadius, 2*m_InnerRadius, 2*m_InnerRadius, 0.0f, 360.0f);

		// Shadow
		if (pBitmaps->WindowColor==(COLORREF)-1)
		{
			Matrix m;
			m.Translate(SHADOWSIZE, SHADOWSIZE);
			Path.Transform(&m);

			m.Reset();
			m.Translate(-1, -1);

			for (UINT a=0; a<SHADOWSIZE; a++)
			{
				SolidBrush brush(Color((BYTE)(a*a/2)<<24));
				g.FillPath(&brush, &Path);

				Path.Transform(&m);
			}
		}

		SolidBrush brush(Color(COLORREF2RGB(pBitmaps->WindowColor)));
		g.FillPath(&brush, &Path);

		// 3D effect
		if (pBitmaps->WindowColor==(COLORREF)-1)
		{
			Pen pen(Color(0x00000000));

			// Inner bevel
			Path.Reset();
			Path.AddArc(Radius-m_InnerRadius+1.0f, Radius-m_InnerRadius+1.0f, 2.0f*m_InnerRadius-2.0f, 2.0f*m_InnerRadius-2.0f, 0.0f, 360.0f);

			LinearGradientBrush brush3(PointF(0, Radius-m_InnerRadius), PointF(0, Radius+m_InnerRadius), Color(0x0C000000), Color(0x00000000));
			g.FillPath(&brush3, &Path);

			// Top shadow
			Path.Reset();
			Path.AddArc(Radius-m_OuterRadius-1.0f, Radius-m_OuterRadius-0.5f, 2.0f*m_OuterRadius+1.0f, 2.0f*m_OuterRadius-1.0f, 180.0f, 180.0f);

			LinearGradientBrush brush1(PointF(0, Radius-m_OuterRadius), PointF(0, Radius), Color(0x10000000), Color(0x00000000));
			pen.SetBrush(&brush1);

			g.DrawPath(&pen, &Path);

			// Bottom shadow
			Path.Reset();
			Path.AddArc(Radius-m_InnerRadius+0.5f, Radius-m_InnerRadius+0.5f, 2.0f*m_InnerRadius-1.0f, 2.0f*m_InnerRadius-1.0f, 0.0f, 180.0f);

			LinearGradientBrush brush2(PointF(0, Radius-1.0f), PointF(0, Radius+m_InnerRadius), Color(0x00000000), Color(0x10000000));
			pen.SetBrush(&brush2);

			g.DrawPath(&pen, &Path);
		}

		dc.SelectObject(hOldBitmap);
	}
}

HueWheelBitmaps* CHueWheel::GetBitmaps(BOOL Themed)
{
	COLORREF WindowColor= Themed ? (COLORREF)-1 : GetSysColor(COLOR_WINDOW);

	// Search bitmap
	for (UINT a=0; a<m_Bitmaps.m_ItemCount; a++)
		if ((m_Bitmaps[a].Size==m_Size) && (m_Bitmaps[a].WindowColor==WindowColor))
			return &m_Bitmaps[a];

	// Create new bitmaps and add to cache
	HueWheelBitmaps Bitmaps;
	ZeroMemory(&Bitmaps, sizeof(Bitmaps));

	Bitmaps.Size = m_Size;
	Bitmaps.WindowColor = WindowColor;
	CreateBitmaps(&Bitmaps);

	m_Bitmaps.AddItem(Bitmaps);

	return &m_Bitmaps[m_Bitmaps.m_ItemCount-1];
}

void CHueWheel::DrawEtchedText(CDC& dc, CRect rect, LPCWSTR lpStr, BOOL Themed)
{
	ASSERT(lpStr);

	if (Themed)
	{
		rect.OffsetRect(0, 1);

		dc.SetTextColor(0xFFFFFF);
		dc.DrawText(lpStr, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		rect.OffsetRect(0, -2);

		dc.SetTextColor(0xECECEC);
		dc.DrawText(lpStr, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		rect.OffsetRect(0, 1);
	}

	dc.SetTextColor(Themed ? 0xB0B0B0 : GetSysColor(COLOR_GRAYTEXT));
	dc.DrawText(lpStr, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

BOOL CHueWheel::PointInRing(const CPoint& point) const
{
	const REAL x = point.x-m_Size/2.0f;
	const REAL y = point.y-m_Size/2.0f;

	const REAL Distance = x*x+y*y;
	return (Distance<=m_OuterRadiusSq) && (Distance>=m_InnerRadiusSq);
}

void CHueWheel::UpdateCursor()
{
	LPCTSTR Cursor = (m_Grabbed || PointInRing(m_CursorPos)) ? IDC_HAND : IDC_ARROW;

	if (Cursor!=lpszCursorName)
	{
		hCursor = FMGetApp()->LoadStandardCursor(Cursor);

		SetCursor(hCursor);
		lpszCursorName = Cursor;
	}
}


BEGIN_MESSAGE_MAP(CHueWheel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

INT CHueWheel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	CRect rect;
	GetClientRect(rect);

	m_Size = rect.Height();

	m_OuterRadius = m_Size/2.0f-1.0f;
	m_OuterRadiusSq = m_OuterRadius*m_OuterRadius;
	m_OuterRadiusSqDraw = (m_OuterRadius+1.0f)*(m_OuterRadius+1.0f);

	m_InnerRadius = m_OuterRadius*0.8f;
	m_InnerRadiusSq = m_InnerRadius*m_InnerRadius;
	m_InnerRadiusSqDraw = (m_InnerRadius-1.0f)*(m_InnerRadius-1.0f);

	return 0;
}

BOOL CHueWheel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHueWheel::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	const BOOL Themed = IsCtrlThemed();

	// Bitmaps
	const HueWheelBitmaps* pBitmaps = GetBitmaps(Themed);
	ASSERT(pBitmaps);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// Draw hue wheel
	HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(pBitmaps->hBitmapWheel);

	dc.BitBlt(0, 0, m_Size, m_Size, &dcMem, 0, 0, SRCCOPY);

	// Draw marker shadow
	const REAL Radius = (m_OuterRadius+m_InnerRadius)/2.0f;
	const REAL Width = (m_OuterRadius-m_InnerRadius)*0.5f;
	const REAL ShadowWidth = Width*1.25f+2.0f;
	const REAL x = (REAL)((m_Size-Width)/2.0f+Radius*sin(m_Hue*(2.0*PI/360.0)));
	const REAL y = (REAL)((m_Size-Width)/2.0f-Radius*cos(m_Hue*(2.0*PI/360.0)));

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeNone);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	SolidBrush brush(Color(0x48000000));
	g.FillEllipse(&brush, x-2.0f, y-2.0f, ShadowWidth, ShadowWidth);

	brush.SetColor(Color(0x18000000));
	g.FillEllipse(&brush, x-1.0f, y-1.0f, ShadowWidth, ShadowWidth);
	g.FillEllipse(&brush, x, y, ShadowWidth, ShadowWidth);

	// Draw top
	dcMem.SelectObject(pBitmaps->hBitmapTop);
	dc.AlphaBlend(0, 0, m_Size, m_Size, &dcMem, 0, 0, m_Size, m_Size, BF);

	dcMem.SelectObject(hOldBitmap);

	// Draw marker top
	COLORREF clr = GetColor();
	brush.SetColor(Color(COLORREF2RGB(clr)));
	g.FillEllipse(&brush, x, y, Width, Width);

	Pen pen(Color(COLORREF2RGB(Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW))), Width/4.0f);
	g.DrawEllipse(&pen, x, y, Width, Width);

	// Draw status
	DrawColor(dc, CRect(m_Size*27/64, m_Size*27/64, m_Size*37/64, m_Size*37/64), Themed, clr);

	CFont* pOldFont = dc.SelectObject(&FMGetApp()->m_SmallFont);

	CString tmpStr;
	tmpStr.Format(_T("%.1lf°"), m_Hue);

	TCHAR szSep[8];
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szSep, 8);
	tmpStr.Replace(_T("."), szSep);

	DrawEtchedText(dc, CRect(0, m_Size*25/64-FMGetApp()->m_SmallFont.GetFontHeight(), rect.Width(), m_Size*25/64), tmpStr, Themed);
	DrawEtchedText(dc, CRect(0, m_Size*39/64, rect.Width(), m_Size*39/64+FMGetApp()->m_SmallFont.GetFontHeight()), CColorPicker::FormatColor(clr), Themed);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

BOOL CHueWheel::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(hCursor);

	return TRUE;
}

void CHueWheel::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	m_CursorPos = point;

	if (m_Grabbed)
	{
		DOUBLE Hue = atan2((DOUBLE)point.x-m_Size/2.0, m_Size/2.0-(DOUBLE)point.y)*(180.0/PI);
		if (Hue<0.0)
			Hue += 360.0;

		SetHue(Hue);
	}
	else
	{
		UpdateCursor();
	}
}

void CHueWheel::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (PointInRing(point))
	{
		if (GetFocus()!=this)
			SetFocus();

		m_Grabbed = TRUE;
		SetCapture();

		UpdateCursor();

		OnMouseMove(nFlags, point);
	}
}

void CHueWheel::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_Grabbed)
	{
		m_Grabbed = FALSE;
		ReleaseCapture();

		UpdateCursor();
	}
}

UINT CHueWheel::OnGetDlgCode()
{
	return DLGC_WANTCHARS;
}

void CHueWheel::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	BOOL Ctrl = (GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0);
	INT Hue = (INT)m_Hue;

	switch (nChar)
	{
	case VK_PRIOR:
		Hue += 15-(Hue % 15);

		if (Hue>=360)
			Hue -= 360;

		SetHue(Hue);
		break;

	case VK_ADD:
	case VK_OEM_PLUS:
		Hue += 5-(Hue % 5);

		if (Hue>=360)
			Hue -= 360;

		SetHue(Hue);
		break;

	case VK_SUBTRACT:
	case VK_OEM_MINUS:
		Hue -= (Hue % 5) ? (Hue % 5) : 5;

		if (Hue<0)
			Hue += 360;

		SetHue(Hue);
		break;

	case VK_NEXT:
		Hue -= (Hue % 15) ? (Hue % 15) : 15;

		if (Hue<0)
			Hue += 360;

		SetHue(Hue);
		break;

	case VK_HOME:
		SetHue(Ctrl ? 0.0 : 270.0);
		break;

	case VK_END:
		SetHue(Ctrl ? 180.0 : 90.0);
		break;
	}
}


// CGradationPyramid
//

#define ROWS        9
#define COLUMNS     (ROWS*2+1)

FMDynArray<GradationPyramidBitmaps, 2, 2> CGradationPyramid::m_Bitmaps;

CGradationPyramid::CGradationPyramid()
	: CWnd()
{
	m_Hue = 225.0;				// Deep sea blue
	m_FocusItem.x = m_FocusItem.y = 0;
	m_HotItem.x = m_HotItem.y = -1;
	m_Hover = FALSE;

	lpszCursorName = IDC_WAIT;
	hCursor = FMGetApp()->LoadStandardCursor(IDC_WAIT);
	m_CursorPos.x = m_CursorPos.y = 0;
}

BOOL CGradationPyramid::Create(CWnd* pParentWnd, const CRect& rect, UINT nID)
{
	ASSERT(rect.Width()==rect.Height());

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_GROUP, rect, pParentWnd, nID);
}

void CGradationPyramid::SetHue(DOUBLE Hue)
{
	ASSERT(Hue>=0.0);
	ASSERT(Hue<=360.0);

	m_Hue = Hue;
	Invalidate();
}

inline void CGradationPyramid::CreateBitmaps(GradationPyramidBitmaps* pBitmaps)
{
	ASSERT(pBitmaps);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	// Top
	if (!pBitmaps->hBitmapTop)
	{
		pBitmaps->hBitmapTop = CreateTransparentBitmap(pBitmaps->Width, pBitmaps->Height);
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(pBitmaps->hBitmapTop);

		Graphics g(dc);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(FMGetApp()->m_SmoothingModeAntiAlias8x8);

		GraphicsPath Path;
		Path.AddRectangle(Rect(-SHADOWSIZE, -SHADOWSIZE, pBitmaps->Width+SHADOWSIZE+1, pBitmaps->Height+SHADOWSIZE+1));

		PointF points[] = { PointF(pBitmaps->Width/2.0f, 0.0), PointF(0.0f, (REAL)pBitmaps->Height), PointF((REAL)pBitmaps->Width, (REAL)pBitmaps->Height) };
		Path.AddPolygon(points, 3);

		// Shadow
		if (pBitmaps->WindowColor==(COLORREF)-1)
		{
			Matrix m;
			m.Translate(SHADOWSIZE, SHADOWSIZE);
			Path.Transform(&m);

			m.Reset();
			m.Translate(-1, -1);

			for (UINT a=0; a<SHADOWSIZE; a++)
			{
				SolidBrush brush(Color((BYTE)(a*a/2)<<24));
				g.FillPath(&brush, &Path);

				Path.Transform(&m);
			}
		}

		SolidBrush brush(Color(COLORREF2RGB(pBitmaps->WindowColor)));
		g.FillPath(&brush, &Path);

		Pen pen(Color(0x40000000), 0.5f);
		g.DrawPath(&pen, &Path);

		dc.SelectObject(hOldBitmap);
	}
}

GradationPyramidBitmaps* CGradationPyramid::GetBitmaps(BOOL Themed)
{
	COLORREF WindowColor= Themed ? (COLORREF)-1 : GetSysColor(COLOR_WINDOW);

	// Search bitmap
	for (UINT a=0; a<m_Bitmaps.m_ItemCount; a++)
		if ((m_Bitmaps[a].Height==m_Height) && (m_Bitmaps[a].Width==m_Width) && (m_Bitmaps[a].WindowColor==WindowColor))
			return &m_Bitmaps[a];

	// Create new bitmaps and add to cache
	GradationPyramidBitmaps Bitmaps;
	ZeroMemory(&Bitmaps, sizeof(Bitmaps));

	Bitmaps.Height = m_Height;
	Bitmaps.Width = m_Width;
	Bitmaps.WindowColor = WindowColor;
	CreateBitmaps(&Bitmaps);

	m_Bitmaps.AddItem(Bitmaps);

	return &m_Bitmaps[m_Bitmaps.m_ItemCount-1];
}

void CGradationPyramid::SetColor(INT Row, INT Column)
{
	if ((m_FocusItem.y!=Row) || (m_FocusItem.x!=Column))
	{
		m_FocusItem.x = Column;
		m_FocusItem.y = Row;

		Invalidate();
	}
}

void CGradationPyramid::SetColor(COLORREF clr)
{
	UINT MinDiff = 0x300;

	for (UINT Row=0; Row<ROWS; Row++)
		for (UINT Column=0; Column<ColumnsPerRow(Row); Column++)
		{
			COLORREF clrShard = GetColor(Row, Column);

			UINT Diff = abs((INT)(clrShard & 0xFF)-(INT)(clr & 0xFF))+
				abs((INT)((clrShard>>8) & 0xFF)-(INT)((clr>>8) & 0xFF))+
				abs((INT)((clrShard>>16) & 0xFF)-(INT)((clr>>16) & 0xFF));

			if (Diff<MinDiff)
			{
				m_FocusItem.x = Column;
				m_FocusItem.y = Row;

				MinDiff =Diff;
			}
		}

	Invalidate();
}

COLORREF CGradationPyramid::GetColor(UINT Row, UINT Column) const
{
	return CDrawingManager::HLStoRGB_ONE(m_Hue/360.0, 0.5-(Column-(ColumnsPerRow(Row)-1)/2.0)/(ColumnsPerRow(ROWS-1)-1), 1.0-(DOUBLE)Row/(ROWS-1));
}

void CGradationPyramid::GetCoordinates(PointF* pPoints, UINT Row, UINT Column, REAL Widen) const
{
	ASSERT(pPoints);
	ASSERT(Row<ROWS);
	ASSERT(Column<ColumnsPerRow(Row));

	REAL x = (REAL)m_Width/2.0f+((REAL)(Column/2)-0.5f*Row)*m_BaseWidth;
	REAL y = Row*m_RowHeight;

	if (Column & 1)
	{
		pPoints[0] = PointF(x-Widen, y-Widen/2.0f);
		pPoints[1] = PointF(x+m_BaseWidth+Widen, y-Widen/2.0f);
		pPoints[2] = PointF(x+m_HalfWidth, y+m_RowHeight+Widen);
	}
	else
	{
		pPoints[0] = PointF(x, y-Widen);
		pPoints[1] = PointF(x+m_HalfWidth+Widen, y+m_RowHeight+Widen/2.0f);
		pPoints[2] = PointF(x-m_HalfWidth-Widen, y+m_RowHeight+Widen/2.0f);
	}
}

void CGradationPyramid::UpdateCursor()
{
	LPCTSTR Cursor = PointInPyramid(m_CursorPos) ? IDC_HAND : IDC_ARROW;

	if (Cursor!=lpszCursorName)
	{
		hCursor = FMGetApp()->LoadStandardCursor(Cursor);

		SetCursor(hCursor);
		lpszCursorName = Cursor;
	}
}

void CGradationPyramid::ItemAtPosition(CPoint point, INT& Row, INT& Column) const
{
	Row = (INT)(point.y/m_RowHeight);
	if (Row<0)
		Row = 0;

	if (Row>ROWS-1)
		Row = ROWS-1;

	const REAL AdjustedX = (REAL)point.x-(m_Height-point.y+1)*m_Slope;
	Column = 2*(INT)(AdjustedX/m_BaseWidth);
	if (AdjustedX-Column*m_HalfWidth>point.y-Row*m_RowHeight)
		Column++;

	if (Column<0)
		Column = 0;

	if ((UINT)Column>ColumnsPerRow(Row)-1)
		Column = ColumnsPerRow(Row)-1;
}

BOOL CGradationPyramid::PointInPyramid(CPoint point) const
{
	return (point.x>m_Width/2.0f-point.y*m_Slope) && (point.x<m_Width/2.0f+point.y*m_Slope);
}


BEGIN_MESSAGE_MAP(CGradationPyramid, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

INT CGradationPyramid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	CRect rect;
	GetClientRect(rect);

	m_Width = rect.Width();
	m_Height = rect.Height();
	m_BaseWidth = (REAL)m_Width/ROWS;
	m_RowHeight = (REAL)m_Height/ROWS;
	m_HalfWidth = (m_BaseWidth/2.0f)*(m_RowHeight+1.0f)/m_RowHeight;
	m_Slope = m_Width/(m_Height*2.0f);

	return 0;
}

BOOL CGradationPyramid::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGradationPyramid::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	const BOOL Themed = IsCtrlThemed();

	// Bitmaps
	const GradationPyramidBitmaps* pBitmaps = GetBitmaps(Themed);
	ASSERT(pBitmaps);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// Draw pyramid
	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	REAL y = 0;

	for (UINT Row=0; Row<ROWS; Row++)
	{
		REAL x = (REAL)m_Width/2.0f-(m_BaseWidth/2.0f)*Row;

		// Odd columns
		for (UINT Column=0; Column<ColumnsPerRow(Row)/2; Column++)
		{
			SolidBrush brush(Color(COLORREF2RGB(GetColor(Row, Column*2+1))));
			g.FillRectangle(&brush, x+m_BaseWidth*Column, y, m_BaseWidth, m_RowHeight+1.0f);
		}

		// Even columns
		for (UINT Column=0; Column<ColumnsPerRow(Row)/2+1; Column++)
		{
			PointF points[3] = { PointF(x, y), PointF(x+m_HalfWidth, y+m_RowHeight+1.0f), PointF(x-m_HalfWidth, y+m_RowHeight+1.0f) };

			SolidBrush brush(Color(COLORREF2RGB(GetColor(Row, Column*2))));
			g.FillPolygon(&brush, points, 3);

			x += m_BaseWidth;
		}

		y += m_RowHeight;
	}

	// Draw marker shadow
	const REAL Width = m_Height/80.0f;

	PointF points[3];
	GetCoordinates(points, m_FocusItem.y, m_FocusItem.x, Width*1.25f+1.0f);

	GraphicsPath Path;
	Path.AddLines(points, 3);
	Path.CloseFigure();

	SolidBrush brush(Color(0x48000000));
	g.FillPath(&brush, &Path);

	Matrix m;
	m.Translate(1.0f, 1.0f);
	Path.Transform(&m);

	brush.SetColor(Color(0x18000000));
	g.FillPath(&brush, &Path);

	Path.Transform(&m);
	g.FillPath(&brush, &Path);

	// Draw top
	HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(pBitmaps->hBitmapTop);

	dc.AlphaBlend(0, 0, m_Width, m_Height, &dcMem, 0, 0, m_Width, m_Height, BF);

	dcMem.SelectObject(hOldBitmap);

	// Draw marker
	GetCoordinates(points, m_FocusItem.y, m_FocusItem.x);

	Path.Reset();
	Path.AddLines(points, 3);
	Path.CloseFigure();

	COLORREF clr = GetColor();
	brush.SetColor(Color(COLORREF2RGB(clr)));
	g.FillPath(&brush, &Path);

	Pen pen(Color(COLORREF2RGB(Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW))), Width);
	g.DrawPath(&pen, &Path);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

BOOL CGradationPyramid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(hCursor);

	return TRUE;
}

void CGradationPyramid::OnMouseMove(UINT nFlags, CPoint point)
{
	if (PointInPyramid(m_CursorPos=point))
	{
		INT Row;
		INT Column;
		ItemAtPosition(point, Row, Column);

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
			if ((FMGetApp()->IsTooltipVisible()) && ((Column!=m_HotItem.x) || (Row!=m_HotItem.y)))
				FMGetApp()->HideTooltip();

		m_HotItem.x = Column;
		m_HotItem.y = Row;

		if (nFlags & MK_LBUTTON)
			SetColor(Row, Column);
	}
	else
	{
		OnMouseLeave();
	}

	UpdateCursor();
}

void CGradationPyramid::OnMouseLeave()
{
	FMGetApp()->HideTooltip();

	m_Hover = FALSE;
}

void CGradationPyramid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!FMGetApp()->IsTooltipVisible())
			FMGetApp()->ShowTooltip(this, point, _T(""), CColorPicker::FormatColor(GetColor(m_HotItem.y, m_HotItem.x)));
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

void CGradationPyramid::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if (PointInPyramid(point))
	{
		if (GetFocus()!=this)
			SetFocus();

		INT Row;
		INT Column;
		ItemAtPosition(point, Row, Column);

		SetColor(Row, Column);
	}
}

void CGradationPyramid::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	if (PointInPyramid(point))
	{
		INT Row;
		INT Column;
		ItemAtPosition(point, Row, Column);

		SetColor(Row, Column);

		// Notify owner
		NMHDR hdr;
		hdr.code = GRADATIONPYRAMID_DBLCLK;
		hdr.hwndFrom = m_hWnd;
		hdr.idFrom = GetDlgCtrlID();

		GetOwner()->SendMessage(WM_NOTIFY, hdr.idFrom, LPARAM(&hdr));
	}
}

UINT CGradationPyramid::OnGetDlgCode()
{
	return DLGC_WANTCHARS | DLGC_WANTARROWS;
}

void CGradationPyramid::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	BOOL Ctrl = (GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0);

	switch (nChar)
	{
	case VK_LEFT:
		if (m_FocusItem.x>0)
			SetColor(m_FocusItem.y, m_FocusItem.x-1);

		break;

	case VK_RIGHT:
		if (m_FocusItem.x<m_FocusItem.y*2)
			SetColor(m_FocusItem.y, m_FocusItem.x+1);

		break;

	case VK_UP:
		if (m_FocusItem.y>0)
		{
			if (m_FocusItem.x>0)
				m_FocusItem.x--;

			if (m_FocusItem.x>(m_FocusItem.y-1)*2)
				m_FocusItem.x = (m_FocusItem.y-1)*2;

			SetColor(m_FocusItem.y-1, m_FocusItem.x);
		}

		break;

	case VK_DOWN:
		if (m_FocusItem.y<ROWS-1)
			SetColor(m_FocusItem.y+1, m_FocusItem.x+1);

		break;

	case VK_HOME:
		SetColor(Ctrl ? 0 : m_FocusItem.y, 0);
		break;

	case VK_END:
		SetColor(Ctrl ? ROWS-1 : m_FocusItem.y, ColumnsPerRow(Ctrl ? ROWS-1 : m_FocusItem.y)-1);
		break;
	}
}


// CColorPicker
//

CColorPicker::CColorPicker()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CColorPicker";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CColorPicker", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CColorPicker::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CColorPicker::Init()
{
	ModifyStyle(WS_BORDER, 0);
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CRect rect;
	GetClientRect(rect);

	// Create hue wheel
	m_wndHueWheel.Create(this, CRect(0, 0, rect.bottom, rect.bottom), 1);

	// Create gradation pyramid
	// Width = Height*2/Sqrt(3)
	// Keep width odd for a nice, pointed pyramid
	m_wndGradationPyramid.Create(this, CRect(rect.right-((INT)(rect.bottom*1.1547) | 1), 0, rect.right, rect.bottom), 2);
	m_wndGradationPyramid.SetOwner(GetOwner());
}

void CColorPicker::SetColor(COLORREF clr)
{
	m_wndHueWheel.SetColor(clr);
	m_wndGradationPyramid.SetColor(clr);
}


BEGIN_MESSAGE_MAP(CColorPicker, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()

	ON_NOTIFY(HUEWHEEL_UPDATE_HUE, 1, OnUpdateHue)
END_MESSAGE_MAP()

INT CColorPicker::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

BOOL CColorPicker::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CColorPicker::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	// Background
	FillRect(pDC, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)pDC.m_hDC, (LPARAM)m_hWnd));
}

void CColorPicker::OnUpdateHue(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_HUEDATA* pTag = (NM_HUEDATA*)pNMHDR;

	m_wndGradationPyramid.SetHue(pTag->Hue);

	*pResult = 0;
}
