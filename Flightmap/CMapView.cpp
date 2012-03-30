
// CMapView.cpp: Implementierung der Klasse CMapView
//

#include "stdafx.h"
#include "CMapView.h"
#include "Flightmap.h"
#include "Resource.h"


// CMapView
//

#define MARGIN     5

CMapView::CMapView()
{
	p_Bitmap = NULL;
	m_Hover = FALSE;
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

BOOL CMapView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
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
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
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

	INT ScrollHeight = 0;
	INT ScrollWidth = 0;
	if (p_Bitmap)
	{
		CSize sz = p_Bitmap->GetBitmapDimension();
		ScrollHeight = sz.cy;
		ScrollWidth = sz.cx;
	}

	BOOL HScroll = FALSE;
	if (ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}
	if (ScrollHeight>rect.Height())
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);
	if ((ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, ScrollHeight-rect.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height();
	si.nMin = 0;
	si.nMax = ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	INT oldHScrollPos = m_HScrollPos;
	m_HScrollMax = max(0, ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Width();
	si.nMin = 0;
	si.nMax = ScrollWidth-1;
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
END_MESSAGE_MAP()

INT CMapView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_TooltipCtrl.Create(this);

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

	if (p_Bitmap)
	{
		CSize sz = p_Bitmap->GetBitmapDimension();
		INT x = sz.cx<rect.Width() ? (rect.Width()-sz.cx)/2 : -m_HScrollPos;
		INT y = sz.cy<rect.Height() ? (rect.Height()-sz.cy)/2 : -m_VScrollPos;

		CRect rectBorder(x, y, x+sz.cx, y+sz.cy);

		if (Themed && theApp.m_UseBgImages)
		{
			CRect rectShadow(rectBorder);
			rectShadow.InflateRect(MARGIN+1, MARGIN+1);

			Graphics g(dc);
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

		CDC dcMap;
		dcMap.CreateCompatibleDC(&pDC);

		CBitmap* pOldBitmap = dcMap.SelectObject(p_Bitmap);
		dc.BitBlt(x, y, sz.cx, sz.cy, &dcMap, 0, 0, SRCCOPY);
		dcMap.SelectObject(pOldBitmap);
	}

	if (Themed && theApp.m_UseBgImages)
	{
		Graphics g(dc);
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

void CMapView::OnMouseMove(UINT nFlags, CPoint point)
{
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
}

void CMapView::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();

	m_Hover = FALSE;
}

void CMapView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		CString caption;
		CString hint;
		//TooltipDataFromPIDL(m_Tree[MAKEPOSI(m_HotItem)].pItem->pidlFQ, &theApp.m_SystemImageListExtraLarge, hIcon, size, caption, hint);

		ClientToScreen(&point);
		m_TooltipCtrl.Track(point, NULL, NULL, CSize(0, 0), caption, hint);
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

BOOL CMapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//m_TooltipCtrl.Deactivate();

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
	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
}

void CMapView::OnContextMenu(CWnd* pWnd, CPoint pos)
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
	pPopup->AddCheckbox(IDM_MAPVIEW_AUTOSIZE);

	pPopup->Track(pos);
}
