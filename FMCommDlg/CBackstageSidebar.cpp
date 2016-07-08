
// CBackstageSidebar.cpp: Implementierung der Klasse CBackstageSidebar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CDialogCmdUI
//

CSidebarCmdUI::CSidebarCmdUI()
	: CCmdUI()
{
	m_Enabled = FALSE;
}

void CSidebarCmdUI::Enable(BOOL bOn)
{
	m_Enabled = bOn;
	m_bEnableChanged = TRUE;
}


// CBackstageSidebar
//

#define BORDER         6
#define BORDERLEFT     BACKSTAGERADIUS-BORDER
#define SHADOW         10

HBITMAP hShadow = NULL;

CBackstageSidebar::CBackstageSidebar()
	: CFrontstageWnd()
{
	p_ButtonIcons = p_TooltipIcons = NULL;
	m_Width = max(BACKSTAGERADIUS, SHADOW);
	m_CountWidth = m_IconSize = 0;
	m_SelectedItem = m_HotItem = m_PressedItem = -1;
	m_Hover = m_Keyboard = m_ShowCounts = FALSE;
	m_ShowShadow = TRUE;
}

BOOL CBackstageSidebar::Create(CWnd* pParentWnd, UINT nID, BOOL ShowCounts)
{
	// Sidebar with numbers?
	m_ShowCounts = ShowCounts;

	if (ShowCounts)
		m_CountWidth = FMGetApp()->m_SmallBoldFont.GetTextExtent(_T("888W")).cx+2*BORDER+SHADOW/2;

	// Create
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_GROUP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CBackstageSidebar::Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT nID, BOOL ShowCounts)
{
	// Set icons
	p_ButtonIcons = &SmallIcons;
	p_ButtonIcons->SetGammaMode(TRUE);

	m_IconSize = SmallIcons.GetIconSize();

	p_TooltipIcons = &LargeIcons;

	return Create(pParentWnd, nID, ShowCounts);
}

BOOL CBackstageSidebar::Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID, BOOL ShowCounts)
{
	// Load icons
	p_ButtonIcons = &SmallIcons;
	p_ButtonIcons->SetGammaMode(TRUE);

	m_IconSize = SmallIcons.Load(ResID);

	p_TooltipIcons = &LargeIcons;
	LargeIcons.Load(ResID, LI_FORTOOLTIPS);

	return Create(pParentWnd, nID, ShowCounts);
}

BOOL CBackstageSidebar::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
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

void CBackstageSidebar::AddItem(BOOL Selectable, UINT CmdID, INT IconID, LPCWSTR Caption, COLORREF Color)
{
	// Hinzuf�gen
	SidebarItem Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.Selectable = Selectable;
	Item.CmdID = CmdID;
	Item.IconID = IconID;

	if (Caption)
		if (Selectable)
		{
			wcscpy_s(Item.Caption, 256, Caption);
			Item.Color = Color;
		}
		else
		{
			for (WCHAR* pDst=Item.Caption; *Caption; )
				*(pDst++) = (WCHAR)towupper(*(Caption++));
		}

	// Metrik
	CSize Size;

	if (Item.Caption[0]==L'\0')
	{
		Size.cx = 0;
		Size.cy = BORDER+2;
	}
	else
	{
		Size = (Selectable ? &FMGetApp()->m_LargeFont : &FMGetApp()->m_SmallBoldFont)->GetTextExtent(Item.Caption);

		if (Selectable)
		{
			Size.cx += m_IconSize+3*BORDER+BORDERLEFT+m_CountWidth+SHADOW/2;
			Size.cy = max(m_IconSize, Size.cy)+2*BORDER;
		}
		else
		{
			Size.cx += 2*BORDER+BORDERLEFT+SHADOW/2;
			Size.cy += BORDER+1;
		}
	}

	m_Width = max(m_Width, Size.cx);
	Item.Height = Size.cy;

	m_Items.AddItem(Item);
}

void CBackstageSidebar::AddCommand(UINT CmdID, INT IconID, LPCWSTR Caption, COLORREF Color)
{
	ASSERT(Caption);

	AddItem(TRUE, CmdID, IconID, Caption, Color);
}

void CBackstageSidebar::AddCommand(UINT CmdID, INT IconID, COLORREF Color)
{
	CString Caption;
	ENSURE(Caption.LoadString(CmdID));

	INT Pos = Caption.Find(L'\n');
	if (Pos!=-1)
		Caption.Delete(0, Pos+1);

	AddItem(TRUE, CmdID, IconID, Caption, Color);
}

void CBackstageSidebar::AddCaption(LPCWSTR Caption)
{
	ASSERT(Caption);

	if (m_Items.m_ItemCount)
		if (!m_Items[m_Items.m_ItemCount-1].Selectable)
		{
			if (*Caption==L'\0')
				return;

			m_Items.m_ItemCount--;
		}

	AddItem(FALSE, 0, -1, Caption);
}

void CBackstageSidebar::AddCaption(UINT ResID)
{
	AddCaption(CString((LPCSTR)ResID));
}

void CBackstageSidebar::ResetCounts()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		m_Items[a].Count = 0;

	Invalidate();
}

void CBackstageSidebar::SetCount(UINT CmdID, UINT Count)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items[a].CmdID==CmdID)
		{
			m_Items[a].Count = Count;
			InvalidateItem(a);

			break;
		}
}

INT CBackstageSidebar::GetMinHeight() const
{
	INT Height = m_Items[m_Items.m_ItemCount-1].Rect.bottom;

	if (IsCtrlThemed())
		Height += 2;

	return Height;
}

void CBackstageSidebar::SetSelection(UINT CmdID)
{
	m_SelectedItem = m_HotItem = m_PressedItem = -1;
	m_Hover = FALSE;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if ((m_Items[a].Selectable) && (m_Items[a].CmdID==CmdID))
		{
			m_SelectedItem = a;
			break;
		}

	Invalidate();
}

INT CBackstageSidebar::ItemAtPosition(CPoint point)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items[a].Selectable)
			if (PtInRect(&m_Items[a].Rect, point))
				return a;

	return -1;
}

void CBackstageSidebar::InvalidateItem(INT Index)
{
	if (Index!=-1)
		InvalidateRect(&m_Items[Index].Rect);
}

void CBackstageSidebar::PressItem(INT Index)
{
	if (Index!=m_PressedItem)
	{
		InvalidateItem(m_SelectedItem);
		InvalidateItem(m_PressedItem);

		if (Index!=-1)
			if (!m_Items[Index].Enabled)
				Index = -1;

		m_PressedItem = Index;
		InvalidateItem(m_PressedItem);
	}
}

void CBackstageSidebar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT y = 0;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		m_Items[a].Rect.top = y;
		y = m_Items[a].Rect.bottom = y+m_Items[a].Height;

		m_Items[a].Rect.left = 0;
		m_Items[a].Rect.right = rect.Width();
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CBackstageSidebar, CFrontstageWnd)
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_CONTEXTMENU()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

LRESULT CBackstageSidebar::OnNcHitTest(CPoint point)
{
	LRESULT HitTest = CFrontstageWnd::OnNcHitTest(point);

	if (HitTest==HTCLIENT)
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		if (!m_Items.m_ItemCount || (point.y>=rectWindow.top+m_Items[m_Items.m_ItemCount-1].Rect.bottom))
			HitTest = HTTRANSPARENT;
	}

	return HitTest;
}

BOOL CBackstageSidebar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBackstageSidebar::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

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
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	BOOL Themed = IsCtrlThemed();

	const COLORREF colTex = Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE);
	const COLORREF colSel = Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT);
	const COLORREF colCap = Themed ? 0xF0F0F0 : GetSysColor(COLOR_3DFACE);
	const COLORREF colNum = Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW);

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	// Items
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		RECT rectIntersect;
		if (IntersectRect(&rectIntersect, &m_Items[a].Rect, rectUpdate))
		{
			CRect rectItem(m_Items[a].Rect);

			const BOOL Enabled = m_Items[a].Enabled && GetParent()->IsWindowEnabled();
			const BOOL Highlight = (m_PressedItem!=-1) ? m_PressedItem==(INT)a : m_SelectedItem==(INT)a;
			const BOOL Hot = Enabled && !Highlight && (m_HotItem==(INT)a);

			// Background
			if (m_Items[a].Selectable)
			{
				if (Themed)
				{
					if ((a==0) || m_Items[a-1].Selectable)
					{
						LinearGradientBrush brush3(Point(0, rectItem.top), Point(0, rectItem.top+2), Color(0x20FFFFFF), Color(0x00FFFFFF));
						g.FillRectangle(&brush3, 0, rectItem.top, rectItem.right, 2);
					}
					else
					{
						LinearGradientBrush brush4(Point(0, rectItem.top), Point(0, rectItem.top+8), Color(0x80000000), Color(0x00000000));

						if (m_ShowShadow)
						{
							g.FillRectangle(&brush4, 0, rectItem.top, rectItem.right, 8);
						}
						else
						{
							for (UINT a=0; a<8; a++)
							{
								g.FillRectangle(&brush4, 0, rectItem.top+a, rectItem.right-a/4-1, 1);

								SolidBrush brush5(Color((4-(a & 3))*(8-a)*0x04000000));
								g.FillRectangle(&brush5, rectItem.right-a/4-1, rectItem.top+a, 1, 1);
							}
						}
					}

					LinearGradientBrush brush6(Point(0, rectItem.bottom-3), Point(0, rectItem.bottom), Color(0x00000000), Color(0x60000000));
					g.FillRectangle(&brush6, 0, rectItem.bottom-2, rectItem.right, 2);

					if ((a<m_Items.m_ItemCount-1) && (!m_Items[a+1].Selectable))
					{
						SolidBrush brush7(Color(0x28000000));
						g.FillRectangle(&brush7, 0, rectItem.bottom-1, rectItem.right, 1);
					}
				}

				DrawBackstageSelection(dc, g, rectItem, Highlight, TRUE, Themed);
			}
			else
			{
				if (Themed)
				{
					SolidBrush brush1(Color(0x68000000));
					g.FillRectangle(&brush1, 0, rectItem.top, rectItem.right, rectItem.Height());

					SolidBrush brush2(Color(0x80000000));
					g.FillRectangle(&brush2, rectItem.right-1, rectItem.top, 1, rectItem.Height()-1);
					g.FillRectangle(&brush2, 0, rectItem.bottom-1, rectItem.right, 1);

					SolidBrush brush3(Color(0x28FFFFFF));
					g.FillRectangle(&brush3, 0, rectItem.top, rectItem.right, 1);
				}
				else
				{
					dc.FillSolidRect(0, rectItem.top, rectItem.right, 1, 0x000000);
					dc.FillSolidRect(0, rectItem.bottom-1, rectItem.right, 1, 0x000000);

					if (!m_ShowShadow)
						dc.FillSolidRect(rectItem.right-1, rectItem.top+1, 1, rectItem.Height()-2, 0x000000);
				}
			}

			rectItem.left += BORDERLEFT;

			if (m_Items[a].Selectable)
			{
				rectItem.DeflateRect(BORDER, BORDER);

				// Icon
				if (m_IconSize)
				{
					if (m_Items[a].IconID!=-1)
						p_ButtonIcons->Draw(dc, rectItem.left, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_Items[a].IconID, Themed && Hot, Themed && !Highlight);

					rectItem.left += m_IconSize+BORDER;
				}

				// Count
				if (m_ShowCounts && (m_Items[a].Count))
				{
					CRect rectCount(rectItem.right-m_CountWidth+BORDER/2, rectItem.top-2, rectItem.right-BORDER/2-SHADOW/2, rectItem.bottom);

					const COLORREF clr = m_Items[a].Color;
					if (clr!=(COLORREF)-1)
					{
						if (Themed)
						{
							g.SetSmoothingMode(SmoothingModeAntiAlias);

							GraphicsPath path;
							CreateRoundRectangle(rectCount, 6, path);

							Matrix m1;
							m1.Translate(3.5f, 3.5f);
							path.Transform(&m1);

							if (!Highlight)
							{
								SolidBrush brushShadow(Color(0x40000000));
								g.FillPath(&brushShadow, &path);
							}

							Matrix m2;
							m2.Translate(-2.5f, -2.5f);
							path.Transform(&m2);

							Color clrFill;
							clrFill.SetFromCOLORREF(clr);

							SolidBrush brushFill(clrFill);
							g.FillPath(&brushFill, &path);

							Pen pen(Color(0xFFFFFFFF), 2.0f);
							g.DrawPath(&pen, &path);

							g.SetSmoothingMode(SmoothingModeNone);

							rectCount.right += 3;
							rectCount.bottom++;
						}
						else
						{
							dc.FillSolidRect(rectCount, clr);
							dc.Draw3dRect(rectCount, 0xFFFFFF, 0xFFFFFF);
						}

						dc.SetTextColor(0xFFFFFF);
					}
					else
					{
						rectCount.top += 5;

						dc.SetTextColor(Highlight ? colSel : colNum);
					}

					CString tmpStr;
					if (m_Items[a].Count>=1000000)
					{
						tmpStr.Format(_T("%dm"), m_Items[a].Count/1000000);
					}
					else
						if (m_Items[a].Count>=1000)
						{
							tmpStr.Format(_T("%dk"), m_Items[a].Count/1000);
						}
						else
						{
							tmpStr.Format(_T("%d"), m_Items[a].Count);
						}

					CFont* pOldFont = dc.SelectObject(clr!=(COLORREF)-1 ? &FMGetApp()->m_SmallBoldFont : &FMGetApp()->m_SmallFont);
					dc.DrawText(tmpStr, rectCount, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | (clr!=(COLORREF)-1 ? DT_CENTER : DT_RIGHT));
					dc.SelectObject(pOldFont);
				}

				dc.SetTextColor(Highlight || Hot ? colSel : Enabled ? colTex : colNum);
			}
			else
			{
				rectItem.left += BORDER;
				rectItem.bottom -= 2;

				dc.SetTextColor(colCap);
			}

			// Text
			if (m_Items[a].Caption[0])
			{
				CFont* pOldFont = dc.SelectObject(m_Items[a].Selectable ? &FMGetApp()->m_LargeFont : &FMGetApp()->m_SmallBoldFont);

				if (!m_Items[a].Selectable)
				{
					rectItem.OffsetRect(0, 1);
				}
				else
					if (Themed && !Highlight)
					{
						rectItem.OffsetRect(0, -1);
						COLORREF col = dc.SetTextColor(0x000000);

						dc.DrawText(m_Items[a].Caption, rectItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

						rectItem.OffsetRect(0, 1);
						dc.SetTextColor(col);
					}

				dc.DrawText(m_Items[a].Caption, rectItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

				dc.SelectObject(pOldFont);
			}
		}
	}

	if (Themed)
	{
		// Bottom border
		if (m_Items.m_ItemCount)
		{
			CRect rectItem(m_Items[m_Items.m_ItemCount-1].Rect);

			LinearGradientBrush brush1(Point(0, rectItem.bottom), Point(0, rectItem.bottom+2), Color(0x20FFFFFF), Color(0x00FFFFFF));
			g.FillRectangle(&brush1, 0, rectItem.bottom, rectItem.right, 2);
		}

		// Shadow
		if (m_ShowShadow)
		{
			if (!hShadow)
				hShadow = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_SIDEBARSHADOW));

			HDC hdcMem = CreateCompatibleDC(dc);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hShadow);

			AlphaBlend(dc, rect.right-SHADOW, 0, SHADOW, 10, hdcMem, 0, 0, SHADOW, 10, BF);

			for (INT y=10; y<rect.bottom; y+=6)
				AlphaBlend(dc, rect.right-SHADOW, y, SHADOW, 6, hdcMem, 0, 4, SHADOW, 6, BF);

			SelectObject(hdcMem, hOldBitmap);
			DeleteDC(hdcMem);
		}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CBackstageSidebar::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	OnIdleUpdateCmdUI();
	AdjustLayout();
}

void CBackstageSidebar::OnMouseMove(UINT nFlags, CPoint point)
{
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

	if (nFlags & MK_LBUTTON)
		PressItem(Item);

	if (nFlags & MK_RBUTTON)
		SetFocus();
}

void CBackstageSidebar::OnMouseLeave()
{
	FMGetApp()->HideTooltip();
	InvalidateItem(m_SelectedItem);
	InvalidateItem(m_HotItem);
	InvalidateItem(m_PressedItem);

	m_Hover = FALSE;
	m_HotItem = -1;
	
	if (!m_Keyboard)
		m_PressedItem = -1;
}

void CBackstageSidebar::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!FMGetApp()->IsTooltipVisible())
			{
				const SidebarItem* pSidebarItem = &m_Items[m_HotItem];

				NM_TOOLTIPDATA tag;
				ZeroMemory(&tag, sizeof(tag));

				tag.hdr.code = REQUEST_TOOLTIP_DATA;
				tag.hdr.hwndFrom = m_hWnd;
				tag.hdr.idFrom = GetDlgCtrlID();
				tag.Item = pSidebarItem->CmdID;

				if (GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag)))
					FMGetApp()->ShowTooltip(this, point, tag.Caption[0] ? tag.Caption : pSidebarItem->Caption, tag.Hint, tag.hIcon ? tag.hIcon : (pSidebarItem->IconID!=-1) && (p_TooltipIcons!=NULL) ? p_TooltipIcons->ExtractIcon(pSidebarItem->IconID, IsCtrlThemed()) : NULL, tag.hBitmap);
			}
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

void CBackstageSidebar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	PressItem(ItemAtPosition(point));
}

void CBackstageSidebar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		PressItem(Item);

		if (m_Items[Item].Enabled)
			GetOwner()->PostMessage(WM_COMMAND, m_Items[Item].CmdID);
	}
}

UINT CBackstageSidebar::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

void CBackstageSidebar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT Index = (m_PressedItem!=-1) ? m_PressedItem : m_SelectedItem;

	switch (nChar)
	{
	case VK_RETURN:
	case VK_EXECUTE:
	case VK_SPACE:
		if (Index!=-1)
			GetOwner()->PostMessage(WM_COMMAND, m_Items[Index].CmdID);

		return;

	case VK_HOME:
		for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
			if (m_Items[a].Selectable && m_Items[a].Enabled)
			{
				PressItem(a);
				m_Keyboard = TRUE;

				break;
			}

		return;

	case VK_END:
		for (INT a=(INT)m_Items.m_ItemCount-1; a>=0; a--)
			if (m_Items[a].Selectable && m_Items[a].Enabled)
			{
				PressItem(a);
				m_Keyboard = TRUE;

				break;
			}

		return;

	case VK_UP:
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (--Index<0)
				Index = m_Items.m_ItemCount-1;

			if (m_Items[Index].Selectable && m_Items[Index].Enabled)
			{
				PressItem(Index);
				m_Keyboard = TRUE;

				break;
			}
		}

		return;

	case VK_DOWN:
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (++Index>=(INT)m_Items.m_ItemCount)
				Index = 0;

			if (m_Items[Index].Selectable && m_Items[Index].Enabled)
			{
				PressItem(Index);
				m_Keyboard = TRUE;

				break;
			}
		}

		return;
	}

	CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBackstageSidebar::OnIdleUpdateCmdUI()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items[a].Selectable)
		{
			CSidebarCmdUI cmdUI;
			cmdUI.m_nID = m_Items[a].CmdID;
			cmdUI.DoUpdate(GetOwner(), TRUE);

			if (m_Items[a].Enabled!=cmdUI.m_Enabled)
			{
				m_Items[a].Enabled = cmdUI.m_Enabled;
				InvalidateItem(a);
			}
		}
}

void CBackstageSidebar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
}

void CBackstageSidebar::OnKillFocus(CWnd* /*pNewWnd*/)
{
	if (m_Keyboard)
	{
		PressItem(-1);
		m_Keyboard = FALSE;
	}
}