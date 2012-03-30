
// CMapView.cpp: Implementierung der Klasse CMapView
//

#include "stdafx.h"
#include "CMapView.h"
#include "Flightmap.h"
#include "Resource.h"


// CMapView
//

#define MARGIN     5

static const DOUBLE ZoomFactors[] = { 0.125, 0.25, 1.0/3.0, 0.5, 2.0/3.0, 0.75, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0 };

CMapView::CMapView()
{
	p_Bitmap = NULL;
	m_Hover = FALSE;
	m_ScrollWidth = m_ScrollHeight = 0;
	m_ZoomFactor = theApp.m_MapZoomFactor;
	m_Autosize = theApp.m_MapAutosize;
}

CMapView::~CMapView()
{
}

BOOL CMapView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CMapView::SetBitmap(CBitmap* pBitmap)
{
	p_Bitmap = pBitmap;
	AdjustLayout();
}

void CMapView::AdjustLayout()
{
	AdjustScrollbars();
	Invalidate();
}

void CMapView::ResetScrollbars()
{
	ScrollWindowEx(0, m_VScrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
	ScrollWindowEx(m_HScrollPos, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
	m_VScrollPos = m_HScrollPos = 0;
	SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
	SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);
}

void CMapView::AdjustScrollbars()
{
	CRect rect;
	GetWindowRect(&rect);

	if (p_Bitmap)
	{
		ASSERT(m_ZoomFactor>=0);
		ASSERT(m_ZoomFactor<=11);

		CSize sz = p_Bitmap->GetBitmapDimension();
		DOUBLE ZoomFactor;
		if (m_Autosize)
		{
			DOUBLE sx = (DOUBLE)rect.Width()/(DOUBLE)sz.cx;
			DOUBLE sy = (DOUBLE)rect.Height()/(DOUBLE)sz.cy;
			ZoomFactor = min(sx, sy);
			if (ZoomFactor>1.0)
				ZoomFactor = 1.0;
		}
		else
		{
			ZoomFactor = ZoomFactors[m_ZoomFactor];
		}

		m_ScrollWidth = (INT)((DOUBLE)sz.cx*ZoomFactor);
		m_ScrollHeight = (INT)((DOUBLE)sz.cy*ZoomFactor);
		if (m_Autosize)
		{
			m_ScrollWidth = min(m_ScrollWidth, rect.Width());
			m_ScrollHeight = min(m_ScrollHeight, rect.Height());
		}
	}

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

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height();
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	INT oldHScrollPos = m_HScrollPos;
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Width();
	si.nMin = 0;
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);

	if ((oldVScrollPos!=m_VScrollPos) || (oldHScrollPos!=m_HScrollPos))
		Invalidate();
}


BEGIN_MESSAGE_MAP(CMapView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()

	ON_COMMAND(IDM_MAPVIEW_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_MAPVIEW_ZOOMOUT, OnZoomOut)
	ON_COMMAND(IDM_MAPVIEW_AUTOSIZE, OnAutosize)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPVIEW_ZOOMIN, IDM_MAPVIEW_AUTOSIZE, OnUpdateCommands)
END_MESSAGE_MAP()

INT CMapView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
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

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	BOOL Themed = IsCtrlThemed();
	dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	Graphics g(dc);

	if (p_Bitmap)
	{
		INT x = m_ScrollWidth<rect.Width() ? (rect.Width()-m_ScrollWidth)/2 : -m_HScrollPos;
		INT y = m_ScrollHeight<rect.Height() ? (rect.Height()-m_ScrollHeight)/2 : -m_VScrollPos;

		CRect rectBorder(x, y, x+m_ScrollWidth, y+m_ScrollHeight);

		if (Themed && theApp.m_UseBgImages)
		{
			CRect rectShadow(rectBorder);
			rectShadow.InflateRect(MARGIN+1, MARGIN+1);

			SolidBrush brush(Color(0x08, 0x00, 0x00, 0x00));

			for (UINT a=0; a<6; a++)
			{
				rectShadow.OffsetRect(1, 1);
				g.FillRectangle(&brush, rectShadow.left, rectBorder.bottom+MARGIN+1, rectShadow.Width(), rectShadow.bottom-(rectBorder.bottom+MARGIN+1));
				g.FillRectangle(&brush, rectBorder.right+MARGIN+1, rectShadow.top, rectShadow.right-(rectBorder.right+MARGIN+1), rectShadow.Height()-(rectShadow.bottom-(rectBorder.bottom+MARGIN+1)));
			}

			rectBorder.InflateRect(MARGIN, MARGIN);
			dc.FillSolidRect(rectBorder, 0xFFFFFF);

			rectBorder.InflateRect(1, 1);
			dc.Draw3dRect(rectBorder, 0xECECEC, 0xECECEC);
		}
		else
		{
			rectBorder.InflateRect(1, 1);
			dc.Draw3dRect(rectBorder, 0x000000, 0x000000);
		}

		if ((!m_Autosize) && (ZoomFactors[m_ZoomFactor]==1.0))
		{
			CDC dcMap;
			dcMap.CreateCompatibleDC(&pDC);

			CBitmap* pOldBitmap = dcMap.SelectObject(p_Bitmap);
			dc.BitBlt(x, y, m_ScrollWidth, m_ScrollHeight, &dcMap, 0, 0, SRCCOPY);
			dcMap.SelectObject(pOldBitmap);
		}
		else
		{
			CSize sz = p_Bitmap->GetBitmapDimension();
			g.SetInterpolationMode((m_ScrollWidth==sz.cx) || (m_ScrollWidth>=2*sz.cx) ? InterpolationModeNearestNeighbor : InterpolationModeHighQualityBicubic);

			Bitmap bmp((HBITMAP)p_Bitmap->m_hObject, NULL);
			g.DrawImage(&bmp, Rect(x, y, m_ScrollWidth, m_ScrollHeight), 0, 0, sz.cx, sz.cy, UnitPixel);
		}
	}

	if (Themed && theApp.m_UseBgImages)
	{
		SolidBrush brush(Color(0x14, 0x00, 0x00, 0x00));
		for (INT a=0; a<5; a++)
			g.FillRectangle(&brush, 0, 0, rect.Width(), a+1);
	}
	else
	{
		if ((theApp.OSVersion==OS_XP) || (!Themed))
			dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, GetSysColor(COLOR_SCROLLBAR));
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CMapView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMapView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(&rect);

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
		Invalidate();

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
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

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CMapView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

BOOL CMapView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
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

void CMapView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
	case VK_PRIOR:
		OnVScroll(SB_PAGEUP, 0, NULL);
		break;
	case VK_NEXT:
		OnVScroll(SB_PAGEDOWN, 0, NULL);
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
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			OnZoomIn();
		break;
	case VK_SUBTRACT:
	case VK_OEM_MINUS:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			OnZoomOut();
		break;
	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
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

	CDialogMenuPopup* pPopup = new CDialogMenuPopup();
	pPopup->Create(GetOwner(), IDB_MENUMAPVIEW_32, IDB_MENUMAPVIEW_16);
	pPopup->AddCommand(IDM_MAPVIEW_ZOOMIN, 0, CDMB_SMALL, FALSE);
	pPopup->AddCommand(IDM_MAPVIEW_ZOOMOUT, 1, CDMB_SMALL, FALSE);
	pPopup->AddSeparator();
	pPopup->AddCheckbox(IDM_MAPVIEW_AUTOSIZE, FALSE, TRUE);

	pPopup->Track(pos);
}


void CMapView::OnZoomIn()
{
	if (m_ZoomFactor<11)
	{
		theApp.m_MapZoomFactor = m_ZoomFactor++;
		AdjustLayout();
	}
}

void CMapView::OnZoomOut()
{
	if (m_ZoomFactor>0)
	{
		theApp.m_MapZoomFactor = m_ZoomFactor--;
		AdjustLayout();
	}
}

void CMapView::OnAutosize()
{
	theApp.m_MapAutosize = m_Autosize = !m_Autosize;
	AdjustLayout();
}

void CMapView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_MAPVIEW_ZOOMIN:
		b = (!m_Autosize) && (m_ZoomFactor<11);
		break;
	case IDM_MAPVIEW_ZOOMOUT:
		b = (!m_Autosize) && (m_ZoomFactor>0);
		break;
	case IDM_MAPVIEW_AUTOSIZE:
		pCmdUI->SetCheck(m_Autosize);
		break;
	}

	pCmdUI->Enable(b);
}
