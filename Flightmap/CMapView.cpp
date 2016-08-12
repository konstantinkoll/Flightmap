
// CMapView.cpp: Implementierung der Klasse CMapView
//

#include "stdafx.h"
#include "CMapFactory.h"
#include "CMapView.h"
#include "Flightmap.h"


// CMapView
//

#define MARGIN          5
#define BORDER          3*MARGIN
#define WHITE           100

CMapView::CMapView()
{
	p_BitmapOriginal = m_pBitmapScaled = NULL;

	m_Hover = FALSE;
	m_ScrollWidth = m_ScrollHeight = 0;
	m_ZoomFactor = theApp.m_MapZoomFactor;
}

CMapView::~CMapView()
{
	DeleteScaledBitmap();
}

BOOL CMapView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CMapView::DeleteScaledBitmap()
{
	if (m_pBitmapScaled!=p_BitmapOriginal)
		delete m_pBitmapScaled;

	m_pBitmapScaled = NULL;
}

void CMapView::SetBitmap(CBitmap* pBitmap)
{
	p_BitmapOriginal = pBitmap;

	ScaleBitmap();
}

void CMapView::ScaleBitmap()
{
	DeleteScaledBitmap();

	if (p_BitmapOriginal)
	{
		CSize Size = p_BitmapOriginal->GetBitmapDimension();

		const DOUBLE ZoomFactor = GetZoomFactor();
		INT Width = (INT)((DOUBLE)Size.cx*ZoomFactor);
		INT Height = (INT)((DOUBLE)Size.cy*ZoomFactor);

		if (ZoomFactor<1.0)
		{
			// Create a pre-scaled copy of the map
			m_pBitmapScaled = CreateTruecolorBitmapObject(Width, Height);

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			CBitmap* pOldBitmap = dc.SelectObject(m_pBitmapScaled);

			Graphics g(dc);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			Bitmap Map(*p_BitmapOriginal, NULL);
			g.DrawImage(&Map, Rect(-1, -1, Width+1, Height+1), 0, 0, Size.cx, Size.cy, UnitPixel);

			dc.SelectObject(pOldBitmap);
		}
		else
		{
			// Reference original map
			m_pBitmapScaled = p_BitmapOriginal;
		}

		// Add border pixels
		m_ScrollWidth = Width+2*BORDER;
		m_ScrollHeight = Height+2*BORDER;
	}
	else
	{
		m_ScrollWidth = m_ScrollHeight = 0;
	}

	AdjustLayout();
}

void CMapView::AdjustLayout()
{
	AdjustScrollbars();
	Invalidate();
}

void CMapView::ResetScrollbars()
{
	ScrollWindow(m_HScrollPos, m_VScrollPos);

	SetScrollPos(SB_VERT, m_VScrollPos=0);
	SetScrollPos(SB_HORZ, m_HScrollPos=0);
}

void CMapView::AdjustScrollbars()
{
	// Dimensions
	CRect rect;
	GetWindowRect(rect);

	BOOL HScroll = FALSE;
	if (m_ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}

	if (m_ScrollHeight>rect.Height())
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);

	if ((m_ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	// Set vertical bars
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height();
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	// Set horizontal bars
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	si.nPage = rect.Width();
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);
}


BEGIN_MESSAGE_MAP(CMapView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()

	ON_COMMAND(IDM_MAPWND_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_MAPWND_ZOOMOUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPWND_ZOOMIN, IDM_MAPWND_ZOOMOUT, OnUpdateCommands)
END_MESSAGE_MAP()

INT CMapView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	ResetScrollbars();

	return 0;
}

BOOL CMapView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMapView::OnPaint()
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

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	// Background
	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xF8F5F4 : GetSysColor(COLOR_3DFACE));

	if (Themed)
	{
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		LinearGradientBrush brush(Point(0, 0), Point(0, WHITE), Color(0xFFFFFFFF), Color(0xFFF4F5F8));
		g.FillRectangle(&brush, Rect(0, 0, rect.Width(), WHITE));

		g.SetPixelOffsetMode(PixelOffsetModeNone);
	}

	// Item
	if (m_pBitmapScaled)
	{
		INT PosX = m_ScrollWidth<rect.Width() ? (rect.Width()-m_ScrollWidth)/2 : -m_HScrollPos;
		INT PosY = m_ScrollHeight<rect.Height() ? (rect.Height()-m_ScrollHeight)/2 : -m_VScrollPos;

		CRect rectMap(PosX+BORDER, PosY+BORDER, PosX+m_ScrollWidth-BORDER, PosY+m_ScrollHeight-BORDER);

		CRect rectItem(rectMap);
		rectItem.InflateRect(MARGIN, MARGIN);

		// Shadow
		GraphicsPath Path;

		if (Themed)
		{
			CRect rectShadow(rectItem);
			rectShadow.OffsetRect(1, 1);

			CreateRoundRectangle(rectShadow, 3, Path);

			Pen pen(Color(0x0C000000));
			g.DrawPath(&pen, &Path);
		}

		// Background
		CRect rectBorder(rectItem);
		rectItem.DeflateRect(1, 1);

		dc.FillSolidRect(rectItem, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		if (Themed)
		{
			Matrix m;
			m.Translate(-1.0, -1.0);
			Path.Transform(&m);

			Pen pen(Color(0xFFD0D1D5));
			g.DrawPath(&pen, &Path);
		}
		else
		{
			dc.Draw3dRect(rectItem, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
		}

		// Map picture (either pre-scaled or streched)
		CSize Size = m_pBitmapScaled->GetBitmapDimension();

		CDC dcMap;
		dcMap.CreateCompatibleDC(&pDC);

		CBitmap* pOldBitmap = dcMap.SelectObject(m_pBitmapScaled);

		if ((Size.cx<rectMap.Width()) || (Size.cy<rectMap.Height()))
		{
			dc.StretchBlt(rectMap.left, rectMap.top, rectMap.Width(), rectMap.Height(), &dcMap, 0, 0, Size.cx, Size.cy, SRCCOPY);
		}
		else
		{
			dc.BitBlt(rectMap.left, rectMap.top, rectMap.Width(), rectMap.Height(), &dcMap, 0, 0, SRCCOPY);
		}

		dcMap.SelectObject(pOldBitmap);
	}

	DrawWindowEdge(g, Themed);

	if (Themed)
		CTaskbar::DrawTaskbarShadow(g, rect);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CMapView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CMapView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;

	case SB_LINEUP:
		nInc = -64;
		break;

	case SB_LINEDOWN:
		nInc = 64;
		break;

	case SB_PAGEUP:
		nInc = min(-1, -rect.Height());
		break;

	case SB_PAGEDOWN:
		nInc = max(1, rect.Height());
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		SetScrollPos(SB_VERT, m_VScrollPos);

		Invalidate();
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMapView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_HScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_HScrollMax-m_HScrollPos;
		break;

	case SB_PAGEUP:
	case SB_LINEUP:
		nInc = -64;
		break;

	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = 64;
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);

		nInc = si.nTrackPos-m_HScrollPos;
		break;
	}

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_HORZ, m_HScrollPos);
	}

	CFrontstageWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMapView::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

BOOL CMapView::OnMouseWheel(UINT /*nFlags*/, SHORT zDelta, CPoint /*pt*/)
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

void CMapView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch (nChar)
	{
	case VK_LEFT:
		OnHScroll(SB_LINEUP, 0, NULL);
		break;

	case VK_RIGHT:
		OnHScroll(SB_LINEDOWN, 0, NULL);
		break;

	case VK_UP:
		OnVScroll(SB_LINEUP, 0, NULL);
		break;

	case VK_DOWN:
		OnVScroll(SB_LINEDOWN, 0, NULL);
		break;

	case VK_HOME:
		if (GetKeyState(VK_CONTROL)<0)
			OnVScroll(SB_TOP, 0, NULL);

		OnHScroll(SB_TOP, 0, NULL);

		break;

	case VK_END:
		if (GetKeyState(VK_CONTROL)<0)
			OnVScroll(SB_BOTTOM, 0, NULL);

		OnHScroll(SB_BOTTOM, 0, NULL);

		break;

	case VK_ADD:
	case VK_OEM_PLUS:
	case VK_PRIOR:
		OnZoomIn();
		break;

	case VK_SUBTRACT:
	case VK_OEM_MINUS:
	case VK_NEXT:
		OnZoomOut();
	}
}

void CMapView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu Menu;
	Menu.LoadMenu(IDM_MAPWND);
	ASSERT_VALID(&Menu);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
}


void CMapView::OnZoomIn()
{
	if (m_ZoomFactor<ZOOMFACTORS-1)
	{
		theApp.m_MapZoomFactor = ++m_ZoomFactor;

		ScaleBitmap();
	}
}

void CMapView::OnZoomOut()
{
	if (m_ZoomFactor>0)
	{
		theApp.m_MapZoomFactor = --m_ZoomFactor;

		ScaleBitmap();
	}
}

void CMapView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (p_BitmapOriginal!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_MAPWND_ZOOMIN:
		bEnable &= (m_ZoomFactor<ZOOMFACTORS);
		break;

	case IDM_MAPWND_ZOOMOUT:
		bEnable &= (m_ZoomFactor>0);
		break;
	}

	pCmdUI->Enable(bEnable);
}
