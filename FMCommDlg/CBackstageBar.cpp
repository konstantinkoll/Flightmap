
// CBackstageBar.cpp: Implementierung der Klasse CBackstageBar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CBackstageBar
//

HBITMAP BackstageBarIcons[2][3][6] = { NULL };

CBackstageBar::CBackstageBar(BOOL Small)
	: CFrontstageWnd()
{
	if (Small)
	{
		p_Font = &FMGetApp()->m_SmallFont;
		m_IconSize = (p_Font->GetFontHeight()-3) | 1;

		if (m_IconSize<9)
			m_IconSize = 9;

		if (m_IconSize>27)
			m_IconSize = 27;

		m_ButtonSize = m_IconSize+8;
	}
	else
	{
		p_Font = &FMGetApp()->m_DefaultFont;
		m_ButtonSize = GetPreferredHeight()-1;
		m_IconSize = (m_ButtonSize-4) & ~3;

		if (m_IconSize<16)
			m_IconSize = 16;

		if (m_IconSize>32)
			m_IconSize = 32;
	}

	m_Small = Small;
	m_HoverItem = m_PressedItem = -1;
}

BOOL CBackstageBar::Create(CWnd* pParentWnd, UINT nID, INT Spacer, BOOL ReverseOrder)
{
	m_Spacer = Spacer;
	m_ReverseOrder = ReverseOrder;

	const CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_NOACTIVATE, className, _T(""), WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

UINT CBackstageBar::GetPreferredHeight()
{
	return FMGetApp()->m_DefaultFont.GetFontHeight()+7;
}

UINT CBackstageBar::GetPreferredWidth() const
{
	return HasButtons() ? m_BarItems.m_ItemCount*m_ButtonSize+(m_BarItems.m_ItemCount-1)*m_Spacer+2 : 0;
}

void CBackstageBar::Reset()
{
	m_BarItems.m_ItemCount = 0;
	m_HoverItem = m_PressedItem = -1;
}

void CBackstageBar::AddItem(UINT Command, INT IconID, INT PreferredWidth, BOOL Red, LPCWSTR pName, BOOL Enabled)
{
	BarItem Item;
	ZeroMemory(&Item, sizeof(Item));

	Item.Command = Command;
	Item.IconID = IconID;
	Item.PreferredWidth = PreferredWidth ? PreferredWidth : m_ButtonSize;
	Item.Red = Red;
	Item.Enabled = Enabled;

	if (pName)
		wcscpy_s(Item.Name, 256, pName);

	m_BarItems.AddItem(Item);
}

INT CBackstageBar::ItemAtPosition(CPoint point) const
{
	CRect rectClient;
	GetClientRect(rectClient);

	if ((point.y>=1) && (point.y<rectClient.bottom-1))
		for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
			if (m_BarItems[a].Enabled && ((m_PressedItem<0) || (m_PressedItem==(INT)a)))
				if ((point.x>=m_BarItems[a].Left) && (point.x<m_BarItems[a].Right))
					return a;

	return -1;
}

void CBackstageBar::InvalidateItem(INT Index)
{
	if (Index>=0)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		InvalidateRect(CRect(m_BarItems[Index].Left, 1, m_BarItems[Index].Right, rectClient.bottom-1));
	}
}

void CBackstageBar::AdjustLayout()
{
	if (!HasButtons())
		return;

	CRect rect;
	GetClientRect(rect);

	// Reset width
	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
		m_BarItems[a].Width = 0;

	ASSERT(m_BarItems.m_ItemCount>=1);
	INT UnallocatedWidth = rect.Width()-2-(m_BarItems.m_ItemCount-1)*m_Spacer;

	// Calc layout
Iterate:
	INT TooSmallCount = 0;
	INT Chunk = UnallocatedWidth;

	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		const INT Diff = m_BarItems[a].PreferredWidth-m_BarItems[a].Width;

		if (Diff>0)
		{
			Chunk = min(Chunk, Diff);
			TooSmallCount++;
		}
	}

	if (TooSmallCount)
	{
		Chunk = min(Chunk, UnallocatedWidth/TooSmallCount);

		if (Chunk>0)
		{
			for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
				if (m_BarItems[a].Width<m_BarItems[a].PreferredWidth)
				{
					m_BarItems[a].Width += Chunk;
					UnallocatedWidth -= Chunk;
				}

			goto Iterate;
		}
	}

	// Set layout
	INT Left = 1;
	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		BarItem* pBarItem = &m_BarItems[m_ReverseOrder ? m_BarItems.m_ItemCount-a-1 : a];

		pBarItem->Left = Left;
		Left = (pBarItem->Right=pBarItem->Left+pBarItem->Width)+m_Spacer;
	}

	Invalidate();
}

void CBackstageBar::DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL /*Themed*/)
{
	ASSERT(State<3);

	const INT IconID = m_BarItems[Index].IconID;
	if ((IconID>=0) && (IconID<6))
	{
		// Icon
		HBITMAP* pBitmap = &BackstageBarIcons[m_Small ? 0 : 1][State][IconID];

		if (!*pBitmap)
			*pBitmap = LoadMaskedIcon(IDI_BACKSTAGEBAR_FIRST+IconID, m_IconSize, dc.GetTextColor());

		CDC dcIcon;
		dcIcon.CreateCompatibleDC(&dc);
		HBITMAP hOldBitmap = (HBITMAP)dcIcon.SelectObject(*pBitmap);

		const BLENDFUNCTION BF = { AC_SRC_OVER, 0, State<2 ? 0xFF : 0xA0, AC_SRC_ALPHA };
		AlphaBlend(dc, rectItem.left+(rectItem.Width()-m_IconSize)/2, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_IconSize, m_IconSize, dcIcon, 0, 0, m_IconSize, m_IconSize, BF);

		dcIcon.SelectObject(hOldBitmap);
	}
}

HBITMAP CBackstageBar::LoadMaskedIcon(UINT nID, INT Size, COLORREF clr)
{
	// RGB to BGR
	clr = (_byteswap_ulong(clr) >> 8) | 0xFF000000;

	HBITMAP hBitmap = CreateTransparentBitmap(Size, Size);

	// Get mask from icon
	HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), IMAGE_ICON, Size, Size, LR_SHARED);

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	DrawIconEx(dc, 0, 0, hIcon, Size, Size, 0, NULL, DI_NORMAL);
	DestroyIcon(hIcon);

	dc.SelectObject(hOldBitmap);

	// Set colors
	BITMAP Bitmap;
	if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
		for (LONG Row=0; Row<Bitmap.bmHeight; Row++)
		{
			LPBYTE Ptr = (LPBYTE)Bitmap.bmBits+Bitmap.bmWidthBytes*Row;

			for (LONG Column=0; Column<Bitmap.bmWidth; Column++)
			{
				const BYTE Alpha = *(Ptr+3);

				switch (Alpha)
				{
				case 0xFF:
					*((COLORREF*)Ptr) = clr;

				case 0x00:
					break;

				default:
					*Ptr = (clr & 0xFF)*Alpha/255;
					*(Ptr+1) = ((clr>>8) & 0xFF)*Alpha/255;
					*(Ptr+2) = (BYTE)(clr>>16)*Alpha/255;
				}

				Ptr += 4;
			}
		}

	return hBitmap;
}


BEGIN_MESSAGE_MAP(CBackstageBar, CFrontstageWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CBackstageBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* /*lpncsp*/)
{
}

void CBackstageBar::OnNcPaint()
{
}

void CBackstageBar::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	HBITMAP hBitmap = CreateTransparentBitmap(rect.Width(), rect.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Border
	const BOOL Themed = IsCtrlThemed();
	if (!Themed)
		dc.Draw3dRect(rect, 0x000000, 0x000000);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	// Items
	CFont* pOldFont = dc.SelectObject(p_Font);

	RECT rectIntersect;

	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		const BarItem* pBarItem = &m_BarItems[a];
		CRect rectItem(pBarItem->Left, 0, pBarItem->Right, rect.bottom-1);

		if (IntersectRect(&rectIntersect, rectItem, rectUpdate))
		{
			DrawBackstageButtonBackground(dc, g, rectItem, m_HoverItem==(INT)a, m_PressedItem==(INT)a, pBarItem->Enabled, Themed, pBarItem->Red);
			DrawItem(dc, rectItem, a, pBarItem->Enabled ? (m_HoverItem==(INT)a) || (m_PressedItem==(INT)a) ? 1 : 0 : 2, Themed);
		}
	}

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);
}

LRESULT CBackstageBar::OnThemeChanged()
{
	HBITMAP* pBitmap = &BackstageBarIcons[0][0][0];

	for (UINT a=0; a<sizeof(BackstageBarIcons)/sizeof(HBITMAP); a++)
	{
		DeleteObject(pBitmap[a]);
		pBitmap[a] = NULL;
	}

	return NULL;
}

void CBackstageBar::OnMouseLeave()
{
	m_PressedItem = -1;

	CFrontstageWnd::OnMouseLeave();
}

void CBackstageBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	InvalidateItem(m_HoverItem=m_PressedItem=ItemAtPosition(point));

	if (m_HoverItem>=0)
		SetCapture();
}

void CBackstageBar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	const INT Index = ItemAtPosition(point);
	if ((Index>=0) && (Index==m_PressedItem))
		OnClickButton(Index);

	m_PressedItem = -1;
	Invalidate(m_HoverItem=Index);

	ReleaseCapture();
}

void CBackstageBar::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}
