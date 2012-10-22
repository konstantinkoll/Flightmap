
// CGridHeader.cpp: Implementierung der Klasse CGridHeader
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CGridHeader
//

BEGIN_MESSAGE_MAP(CGridHeader, CTooltipHeader)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CGridHeader::OnPaint()
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

	FMApplication* pApp = (FMApplication*)AfxGetApp();
	BOOL Themed = IsCtrlThemed();
	BOOL Flat = (pApp->OSVersion==OS_XP) || (pApp->OSVersion==OS_Eight);
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
				const COLORREF colBorder = Flat ? pApp->OSVersion==OS_Eight ? 0xEAE9E8 : GetSysColor(COLOR_SCROLLBAR) : 0xBAB5B1;
				dc.FillSolidRect(rectItem.right-1, rectItem.top, 1, rectItem.Height(), colBorder);
				dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, colBorder);

				if (Themed)
				{
					rectItem.right--;
					rectItem.bottom--;

					if (Flat || (m_PressedItem==a))
					{
						dc.FillSolidRect(rectItem, m_PressedItem==a ? colBorder : pApp->OSVersion==OS_Eight ? 0xF7F6F5 : GetSysColor(COLOR_MENUBAR));
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

				dc.SetTextColor(Themed ? Flat ? GetSysColor(COLOR_MENUTEXT) : m_PressedItem==a ? 0x000000 : 0x7A604C : GetSysColor(COLOR_WINDOWTEXT));
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
