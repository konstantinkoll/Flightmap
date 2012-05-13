
// CTooltipHeader.cpp: Implementierung der Klasse CTooltipHeader
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <algorithm>


// CTooltipHeader
//

CTooltipHeader::CTooltipHeader()
	: CHeaderCtrl()
{
	m_Hover = FALSE;
	m_PressedItem = m_TooltipItem = -1;
}

BOOL CTooltipHeader::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
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
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CHeaderCtrl::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CTooltipHeader, CHeaderCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

INT CTooltipHeader::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHeaderCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	SetFont(&((FMApplication*)AfxGetApp())->m_DefaultFont);

	m_SortIndicators.Create(IDB_SORTINDICATORS, NULL, 0, 3, 7, 4);

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

BOOL CTooltipHeader::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTooltipHeader::OnPaint()
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
	BOOL IsXP = ((FMApplication*)AfxGetApp())->OSVersion==OS_XP;
	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_3DFACE));

	CFont* pOldFont = dc.SelectObject(GetFont());
	Graphics g(dc);

	for (INT a=0; a<GetItemCount(); a++)
	{
		CRect rectItem;
		if (GetItemRect(a, rectItem))
		{
			HDITEM hdi;
			WCHAR lpBuffer[256];
			hdi.mask = HDI_TEXT | HDI_STATE | HDI_FORMAT | HDI_WIDTH;
			hdi.pszText = lpBuffer;
			hdi.cchTextMax = 256;

			if (GetItem(a, &hdi) && (hdi.cxy))
			{
				const COLORREF colBorder = IsXP ? GetSysColor(COLOR_SCROLLBAR) : 0xBAB5B1;
				dc.FillSolidRect(rectItem.right-1, rectItem.top, 1, rectItem.Height(), colBorder);
				dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, colBorder);

				if (Themed)
				{
					rectItem.right--;
					rectItem.bottom--;

					if (IsXP || (m_PressedItem==a))
					{
						dc.FillSolidRect(rectItem, m_PressedItem==a ? colBorder : GetSysColor(COLOR_MENUBAR));
					}
					else
					{
						LinearGradientBrush brush(Point(0, rectItem.top), Point(0, rectItem.bottom), Color(0xFF, 0xFF, 0xFF), Color(0xDC, 0xE6, 0xF4));
						g.FillRectangle(&brush, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height());
					}

					if (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP))
						m_SortIndicators.Draw(&dc, (hdi.fmt & HDF_SORTUP) ? 0 : 1, CPoint(rectItem.left+(rectItem.Width()-7)/2, rectItem.top+2), ILD_TRANSPARENT);

					rectItem.bottom --;
				}
				else
				{
					COLORREF c1 = GetSysColor(COLOR_3DHIGHLIGHT);
					COLORREF c2 = GetSysColor(COLOR_3DFACE);
					COLORREF c3 = GetSysColor(COLOR_3DSHADOW);
					COLORREF c4 = 0x000000;

					if (m_PressedItem==a)
					{
						std::swap(c1, c4);
						std::swap(c2, c3);
					}

					dc.Draw3dRect(rectItem, c1, c4);
					rectItem.DeflateRect(1, 1);
					dc.Draw3dRect(rectItem, c2, c3);
					rectItem.InflateRect(1, 1);
				}

				rectItem.DeflateRect(4, 0);

				UINT nFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX;
				switch (hdi.fmt & HDF_JUSTIFYMASK)
				{
				case HDF_LEFT:
					nFormat |= DT_LEFT;
					break;
				case HDF_CENTER:
					nFormat |= DT_CENTER;
					break;
				case HDF_RIGHT:
					nFormat |= DT_RIGHT;
					break;
				}

				dc.SetTextColor(Themed ? IsXP ? GetSysColor(COLOR_MENUTEXT) : m_PressedItem==a ? 0x000000 : 0x7A604C : GetSysColor(COLOR_WINDOWTEXT));
				dc.DrawText(lpBuffer, rectItem, nFormat);

				if ((!Themed) && (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP)))
				{
					rectItem.left += dc.GetTextExtent(lpBuffer, (INT)wcslen(lpBuffer)).cx+2;
					if (rectItem.left+5<rectItem.right)
						m_SortIndicators.Draw(&dc, (hdi.fmt & HDF_SORTUP) ? 2 : 3, CPoint(rectItem.left, rectItem.top+(rectItem.Height()-3)/2), ILD_TRANSPARENT);
				}
			}
		}
	}

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTooltipHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
	HDHITTESTINFO htt;
	htt.pt = point;
	INT idx = HitTest(&htt);
	m_PressedItem = (htt.flags==HHT_ONHEADER) ? idx : -1;

	CHeaderCtrl::OnLButtonDown(nFlags, point);
	Invalidate();
}

void CTooltipHeader::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_PressedItem = -1;

	CHeaderCtrl::OnLButtonUp(nFlags, point);
	Invalidate();
}

void CTooltipHeader::OnMouseMove(UINT nFlags, CPoint point)
{
	HDHITTESTINFO htt;
	htt.pt = point;
	INT HoverItem = HitTest(&htt);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = FMHOVERTIME;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (HoverItem!=m_TooltipItem))
			m_TooltipCtrl.Deactivate();

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;

	CHeaderCtrl::OnMouseLeave();
}

void CTooltipHeader::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		HDHITTESTINFO htt;
		htt.pt = point;

		m_TooltipItem = HitTest(&htt);
		if (m_TooltipItem!=-1)
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				HDITEMW i;
				WCHAR TooltipTextBuffer[256];
				i.mask = HDI_TEXT;
				i.pszText = TooltipTextBuffer;
				i.cchTextMax = 256;

				if (GetItem(m_TooltipItem, &i))
					if (TooltipTextBuffer[0]!=L'\0')
					{
						ClientToScreen(&point);
						m_TooltipCtrl.Track(point, NULL, NULL, CSize(0, 0), _T(""), TooltipTextBuffer);
					}
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = FMHOVERTIME;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
