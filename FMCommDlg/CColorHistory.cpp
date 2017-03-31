
// CColorHistory.cpp: Implementierung der Klasse CColorHistory
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CColorHistory
//

#define MARGIN     7

CColorHistory::CColorHistory()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CColorHistory";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CColorHistory", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	memcpy_s(m_Colors, sizeof(m_Colors), FMGetApp()->m_ColorHistory, sizeof(FMGetApp()->m_ColorHistory));

	m_FocusItem = 0;
	m_HotItem = -1;
	m_Hover = FALSE;
}

void CColorHistory::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS);
	SetClassLong(m_hWnd, GCL_STYLE, GetClassLong(m_hWnd, GCL_STYLE) | CS_DBLCLKS);

	CRect rect;
	GetClientRect(rect);

	m_ItemWidth = (rect.Width()-15*MARGIN)/16;
}

void CColorHistory::SetFocusItem(INT FocusItem)
{
	m_FocusItem = FocusItem;

	Invalidate();
}

INT CColorHistory::ItemAtPosition(CPoint point) const
{
	if (point.x % (m_ItemWidth+MARGIN)<m_ItemWidth)
	{
		INT Index = point.x/(m_ItemWidth+MARGIN);

		if (Index>15)
			return -1;

		return (m_Colors[Index]!=(COLORREF)-1) ? Index : -1;
	}

	return -1;
}


BEGIN_MESSAGE_MAP(CColorHistory, CFrontstageWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CColorHistory::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CColorHistory::OnPaint()
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

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Colors
	INT x = 0;

	for (UINT a=0; a<16; a++)
	{
		DrawColor(dc, CRect(x, 0, x+m_ItemWidth, rect.bottom), IsCtrlThemed(), m_Colors[a], IsWindowEnabled(), (GetFocus()==this) && ((INT)a==m_FocusItem), (INT)a==m_HotItem);

		x += m_ItemWidth+MARGIN;
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CColorHistory::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);

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
		if ((FMGetApp()->IsTooltipVisible()) && (Index!=m_HotItem))
			FMGetApp()->HideTooltip();

	if (m_HotItem!=Index)
	{
		m_HotItem = Index;
		Invalidate();
	}
}

void CColorHistory::OnMouseLeave()
{
	FMGetApp()->HideTooltip();
	Invalidate();

	m_HotItem = -1;
	m_Hover = FALSE;
}

void CColorHistory::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!FMGetApp()->IsTooltipVisible())
				FMGetApp()->ShowTooltip(this, point, _T(""), CColorPicker::FormatColor(m_Colors[m_HotItem]));
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

void CColorHistory::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		SetFocusItem(Index);

	if (GetFocus()!=this)
		SetFocus();
}

void CColorHistory::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

void CColorHistory::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		SetFocusItem(Index);

		// Notify owner
		NM_COLORDATA tag;
		tag.hdr.code = COLORHISTORY_DBLCLK;
		tag.hdr.hwndFrom = m_hWnd;
		tag.hdr.idFrom = GetDlgCtrlID();
		tag.clr = m_Colors[Index];

		GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
	}
}

void CColorHistory::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		SetFocusItem(Index);

	if (GetFocus()!=this)
		SetFocus();
}

void CColorHistory::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		SetFocusItem(Index);

	if (GetFocus()!=this)
		SetFocus();
}

UINT CColorHistory::OnGetDlgCode()
{
	return DLGC_WANTCHARS | DLGC_WANTARROWS;
}

void CColorHistory::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch (nChar)
	{
	case VK_HOME:
		for (INT a=0; a<16; a++)
			if (m_Colors[a]!=(COLORREF)-1)
			{
				SetFocusItem(a);
				break;
			}

		break;

	case VK_END:
		for (INT a=15; a>=0; a--)
			if (m_Colors[a]!=(COLORREF)-1)
			{
				SetFocusItem(a);
				break;
			}

		break;

	case VK_LEFT:
		for (INT a=m_FocusItem-1; a>=0; a--)
			if (m_Colors[a]!=(COLORREF)-1)
			{
				SetFocusItem(a);
				break;
			}

		break;

	case VK_RIGHT:
		for (INT a=m_FocusItem+1; a<16; a++)
			if (m_Colors[a]!=(COLORREF)-1)
			{
				SetFocusItem(a);
				break;
			}

		break;
	}
}

void CColorHistory::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CColorHistory::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}
