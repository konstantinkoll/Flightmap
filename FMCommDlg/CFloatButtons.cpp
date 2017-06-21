
// CFloatButtons.cpp: Implementierung der Klasse CFloatButtons
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CFloatButton
//

#define PADDING     8

BOOL CFloatButton::Create(CWnd* pParentWnd, UINT nID, CIcons* pButtonIcons, CIcons* pTooltipIcons, INT IconID, BOOL Small)
{
	p_ButtonIcons = pButtonIcons;
	p_TooltipIcons = pTooltipIcons;
	m_IconID = IconID;
	m_Small = Small;

	ENSURE(m_Caption.LoadString(nID));

	INT Pos = m_Caption.Find(L'\n');
	if (Pos!=-1)
	{
		m_Hint = m_Caption.Left(Pos);
		m_Caption.Delete(0, Pos+1);

		if (m_Hint.GetLength()>40)
		{
			Pos = m_Hint.Find(L' ', m_Hint.GetLength()/2);
			if (Pos!=-1)
				m_Hint.SetAt(Pos, L'\n');
		}
	}

	return CHoverButton::Create(m_Caption, pParentWnd, nID);
}

void CFloatButton::GetPreferredSize(LPSIZE lpSize) const
{
	ASSERT(lpSize);

	if (m_Small)
	{
		CString tmpStr;
		GetWindowText(tmpStr);

		lpSize->cx = FMGetApp()->m_DialogFont.GetTextExtent(tmpStr).cx+3*PADDING/2;
		if (m_IconID>=0)
			lpSize->cx += p_ButtonIcons->GetIconSize()+PADDING/2;

		lpSize->cx += 19;
		lpSize->cx -= lpSize->cx%20;

		lpSize->cy = max(p_ButtonIcons->GetIconSize(), FMGetApp()->m_DialogFont.GetFontHeight())+3*PADDING/2;
	}
	else
	{
		lpSize->cy = p_ButtonIcons->GetIconSize()+FMGetApp()->m_DefaultFont.GetFontHeight()+3*PADDING;
		lpSize->cx = lpSize->cy*5/3;
	}
}


BEGIN_MESSAGE_MAP(CFloatButton, CHoverButton)
	ON_NOTIFY_REFLECT(REQUEST_DRAWBUTTONFOREGROUND, OnDrawButtonForeground)
	ON_NOTIFY_REFLECT(REQUEST_TOOLTIP_DATA, OnRequestTooltipData)
END_MESSAGE_MAP()

void CFloatButton::OnDrawButtonForeground(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_DRAWBUTTONFOREGROUND* pDrawButtonForeground = (NM_DRAWBUTTONFOREGROUND*)pNMHDR;
	LPDRAWITEMSTRUCT lpDrawItemStruct = pDrawButtonForeground->lpDrawItemStruct;

	// State
	const BOOL Disabled = (lpDrawItemStruct->itemState & ODS_DISABLED);

	// Content
	CRect rect(lpDrawItemStruct->rcItem);

	CFont* pOldFont = pDrawButtonForeground->pDC->SelectObject(m_Small ? &FMGetApp()->m_DialogFont : &FMGetApp()->m_DefaultFont);

	UINT nFormat = DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
	if (GetParent()->SendMessage(WM_QUERYUISTATE) & UISF_HIDEACCEL)
		nFormat |= DT_HIDEPREFIX;

	if (m_Small)
	{
		rect.DeflateRect(3*PADDING/4, 0);

		// Icon
		if ((m_IconID>=0) && (rect.Width()>=p_ButtonIcons->GetIconSize()+PADDING/2+rect.Height()))
		{
			p_ButtonIcons->Draw(*pDrawButtonForeground->pDC, rect.left, rect.top+(rect.Height()-p_ButtonIcons->GetIconSize())/2, m_IconID, m_Hover, Disabled);

			rect.left += p_ButtonIcons->GetIconSize()+PADDING/2;
		}

		// Text
		pDrawButtonForeground->pDC->DrawText(m_Caption, rect, nFormat | DT_VCENTER);
	}
	else
	{
		// Icon
		p_ButtonIcons->Draw(*pDrawButtonForeground->pDC, rect.left+(rect.Width()-p_ButtonIcons->GetIconSize())/2, rect.top+PADDING, m_IconID, m_Hover, Disabled);

		rect.DeflateRect(PADDING, PADDING);
		rect.top += p_ButtonIcons->GetIconSize()+PADDING-2;

		// Text
		pDrawButtonForeground->pDC->DrawText(m_Caption, rect, nFormat | DT_TOP);
	}

	pDrawButtonForeground->pDC->SelectObject(pOldFont);

	*pResult = TRUE;
}

void CFloatButton::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	CString Caption(m_Caption);
	
	INT Pos = Caption.Find(L'&');
	if (Pos!=-1)
		Caption.Delete(Pos);

	wcscpy_s(pTooltipData->Caption, 256, Caption);
	wcscpy_s(pTooltipData->Hint, 4096, m_Hint);
	pTooltipData->hIcon = p_TooltipIcons->ExtractIcon(m_IconID, IsCtrlThemed());

	*pResult = TRUE;
}


// CButtonGroup
//

#define INNERBORDER     6

INT CButtonGroup::m_Top;

CButtonGroup::CButtonGroup(CFloatButtons* pFloatButtons, LPCWSTR Caption, BOOL Alert)
{
	ASSERT(pFloatButtons);

	ZeroMemory(&m_Rect, sizeof(m_Rect));

	wcscpy_s(m_Caption, 256, Caption);
	p_FloatButtons = pFloatButtons;
	m_Alert = Alert;
	m_BeginNewColumn = FALSE;

	m_Top = FMGetApp()->m_LargeFont.GetFontHeight()+2*INNERBORDER;
}

CButtonGroup::~CButtonGroup()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CButton* pFloatButton = m_Items[a].pFloatButton;

		if (pFloatButton)
		{
			pFloatButton->DestroyWindow();
			delete pFloatButton;
		}
	}
}

void CButtonGroup::CalcSize(ButtonGroupItem& Item) const
{
	if (Item.pFloatButton)
	{
		Item.pFloatButton->GetPreferredSize(&Item.Size);
	}
	else
	{
		Item.Size = FMGetApp()->m_DefaultFont.GetTextExtent(Item.Text);

		if (Item.Bullet && (Item.Size.cx>0))
			Item.Size.cx += p_FloatButtons->m_Indent;
	}
}

void CButtonGroup::BeginNewColumn()
{
	m_BeginNewColumn = TRUE;
}

void CButtonGroup::AddButton(UINT nID, INT IconID, BOOL Small)
{
	ButtonGroupItem Item;
	ZeroMemory(&Item, sizeof(Item));

	Item.pFloatButton = new CFloatButton();
	Item.pFloatButton->Create(p_FloatButtons, nID, Small ? p_FloatButtons->p_SmallIcons : p_FloatButtons->p_LargeIcons, p_FloatButtons->p_LargeIcons, IconID, Small);
	CalcSize(Item);
	Item.BeginNewColumn = m_BeginNewColumn ? 2 : !Small ? 1 : 0;

	m_BeginNewColumn = FALSE;

	m_Items.AddItem(Item);
}

void CButtonGroup::AddText(LPCWSTR Text, BOOL Bullet)
{
	ButtonGroupItem Item;
	ZeroMemory(&Item, sizeof(Item));

	wcscpy_s(Item.Text, 256, Text);
	Item.Bullet = Bullet;
	CalcSize(Item);

	if (m_BeginNewColumn)
	{
		Item.BeginNewColumn = 2;
		m_BeginNewColumn = FALSE;
	}
	else
	{
		if (m_Items.m_ItemCount)
			if (m_Items[m_Items.m_ItemCount-1].pFloatButton)
				if (!m_Items[m_Items.m_ItemCount-1].pFloatButton->IsSmall())
					Item.BeginNewColumn = 1;
	}

	m_Items.AddItem(Item);
}

void CButtonGroup::SetGroupAlert(BOOL Alert)
{
	m_Alert = Alert;
}

void CButtonGroup::SetText(UINT Index, LPCWSTR Text, BOOL Bullet)
{
	ASSERT(Index<m_Items.m_ItemCount);

	wcscpy_s(m_Items[Index].Text, 256, Text);
	m_Items[Index].Bullet = Bullet;
	CalcSize(m_Items[Index]);
}

void CButtonGroup::OnIdleUpdateCmdUI()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CFloatButton* pFloatButton = m_Items[a].pFloatButton;
		
		if (pFloatButton)
		{
			CCmdUI cmdUI;
			cmdUI.m_nID = pFloatButton->GetDlgCtrlID();
			cmdUI.m_pOther = pFloatButton;
			cmdUI.DoUpdate(p_FloatButtons, TRUE);
		}
	}
}

void CButtonGroup::GetPreferredSize(LPSIZE lpSize, INT MaxWidth)
{
	ASSERT(lpSize);

	MaxWidth -= INNERBORDER;

	// Caption size
	lpSize->cx = FMGetApp()->m_LargeFont.GetTextExtent(m_Caption).cx+2*INNERBORDER;
	lpSize->cy = m_Top;

	// Determine column width
	FMDynArray<INT, 8, 8> ColumnWidths;
	INT ColumnWidth = 0;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		if ((a>0) && (m_Items[a].BeginNewColumn))
		{
			ColumnWidths.AddItem(ColumnWidth);
			ColumnWidth = 0;
		}

		if (m_Items[a].Size.cx>ColumnWidth)
			ColumnWidth = m_Items[a].Size.cx;
	}

	ColumnWidths.AddItem(ColumnWidth);

	// Calc cumulative column widths
	INT* CumulativeColumnWidths = new INT[ColumnWidths.m_ItemCount];
	INT CumulativeColumnWidth = 0;

	for (INT a=(INT)ColumnWidths.m_ItemCount-1; a>=0; a--)
	{
		if (ColumnWidths[a]>CumulativeColumnWidth)
			CumulativeColumnWidth = ColumnWidths[a];

		CumulativeColumnWidths[a] = CumulativeColumnWidth;
	}

	// Place items
	INT CurrentColumn = -1;
	INT x = INNERBORDER;
	INT y = m_Top;
	BOOL AllowBreaks = TRUE;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		if (m_Items[a].BeginNewColumn)
			if (++CurrentColumn)
			{
				INT NewLeft = x+ColumnWidths[CurrentColumn-1];
				if (ColumnWidths[CurrentColumn])
					NewLeft += m_Items[a].BeginNewColumn*INNERBORDER;

				if (AllowBreaks&=(NewLeft+CumulativeColumnWidths[CurrentColumn]<=MaxWidth))
				{
					// Align top
					x = NewLeft;
					y = m_Top;
				}
				else
					if (a>0)
						if ((!m_Items[a].pFloatButton) && (m_Items[a-1].pFloatButton) && (!m_Items[a-1].pFloatButton->IsSmall()))
						{
							// Align left
							x = INNERBORDER;
							y = lpSize->cy-INNERBORDER;
						}
			}

		// Extra whitespace after button
		if ((a>0) && (y>m_Top) && (!m_Items[a].pFloatButton))
			if (m_Items[a-1].pFloatButton)
				y += INNERBORDER*3/2;

		if ((m_Items[a].Size.cy) && m_Items[a].Bullet && (y>m_Top))
			y -= INNERBORDER;

		m_Items[a].Rect.left = x;
		m_Items[a].Rect.top = y;
		m_Items[a].Rect.right = x+m_Items[a].Size.cx;
		m_Items[a].Rect.bottom = y+m_Items[a].Size.cy;

		if (m_Items[a].Size.cy)
		{
			if (m_Items[a].Rect.right+INNERBORDER>lpSize->cx)
				lpSize->cx = m_Items[a].Rect.right+INNERBORDER;

			if ((y=m_Items[a].Rect.bottom+INNERBORDER)>lpSize->cy)
					lpSize->cy = y;
		}
	}

	delete[] CumulativeColumnWidths;
}

void CButtonGroup::AdjustLayout(INT VScrollPos)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items[a].pFloatButton)
		{
			CRect rectItem(m_Items[a].Rect);
			rectItem.OffsetRect(m_Rect.left, m_Rect.top-VScrollPos);

			if (rectItem.right>m_Rect.right-INNERBORDER)
				rectItem.right = m_Rect.right-INNERBORDER;

			m_Items[a].pFloatButton->SetWindowPos(NULL, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
}

void CButtonGroup::Draw(CDC& dc, Graphics& g, INT VScrollPos, BOOL Themed) const
{
	CRect rect(m_Rect);
	rect.OffsetRect(0, -VScrollPos);

	// Background
	if (m_Alert)
		if (Themed)
		{
			rect.DeflateRect(1, 1);

			GraphicsPath Path;
			CreateRoundRectangle(&rect, 4, Path);

			LinearGradientBrush brush(Point(0, rect.top-2), Point(0, rect.bottom+2), 0xFFFFF8D4, 0xFFFFFFFF);
			g.FillPath(&brush, &Path);

			Pen pen(Color(0x80FFFFFF));
			g.DrawPath(&pen, &Path);

			rect.InflateRect(1, 1);
			CreateRoundRectangle(&rect, 5, Path);

			brush.SetLinearColors(Color(0xFFF8C834), Color(0xFFFFFFFF));
			pen.SetBrush(&brush);

			g.DrawPath(&pen, &Path);
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_INFOBK));
			dc.Draw3dRect(rect, GetSysColor(COLOR_INFOTEXT), GetSysColor(COLOR_INFOTEXT));
		}

	// Items
	CFont* pOldFont = dc.SelectObject(&FMGetApp()->m_DefaultFont);

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CRect rectItem(m_Items[a].Rect);
		rectItem.OffsetRect(m_Rect.left, m_Rect.top-VScrollPos);

		if (rectItem.right>rect.right-INNERBORDER)
			rectItem.right = rect.right-INNERBORDER;

		if (!m_Items[a].pFloatButton)
		{
			if (m_Items[a].Bullet)
			{
				static const WCHAR Bullet = 0x25BA;

				dc.SetTextColor(Themed ? 0xC0C0C0 : GetSysColor(m_Alert ? COLOR_INFOTEXT : COLOR_WINDOWTEXT));
				dc.DrawText(&Bullet, 1, rectItem, DT_SINGLELINE | DT_LEFT);

				rectItem.left += p_FloatButtons->m_Indent;
			}

			dc.SetTextColor(Themed ? 0x000000 : GetSysColor(m_Alert ? COLOR_INFOTEXT : COLOR_WINDOWTEXT));
			dc.DrawText(m_Items[a].Text, -1, rectItem, DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
		}
		else
		{
			if (Themed)
				DrawWhiteButtonBorder(g, rectItem);
		}
	}

	dc.SelectObject(pOldFont);

	// Caption
	rect.DeflateRect(INNERBORDER, INNERBORDER-FMCATEGORYPADDING);
	DrawCategory(dc, rect, m_Caption, NULL, Themed);
}

BOOL CButtonGroup::SetFocus()
{
	// Set focus on first enabled button
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CFloatButton* pFloatButton = m_Items[a].pFloatButton;

		if (pFloatButton)
			if (pFloatButton->IsWindowEnabled())
			{
				pFloatButton->SetFocus();
				return TRUE;
			}
	}

	return FALSE;
}


// CFloatButtons
//

#define GROUPMARGIN     2*BACKSTAGEBORDER

CFloatButtons::CFloatButtons()
	: CFrontstageWnd()
{
	m_Indent = FMGetApp()->m_DefaultFont.GetFontHeight()*9/8;

	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = m_VScrollMax = m_VScrollPos = 0;
	m_Destroying = FALSE;
}

BOOL CFloatButtons::Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID)
{
	// Load icons
	p_SmallIcons = &SmallIcons;
	SmallIcons.Load(ResID);

	p_LargeIcons = &LargeIcons;
	LargeIcons.Load(ResID, LI_FORTOOLTIPS);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CFloatButtons::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route commands to owner
	if (!CFrontstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return GetOwner()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

	return TRUE;
}

void CFloatButtons::BeginGroup(LPCWSTR Caption, BOOL Alert)
{
	m_ButtonGroups.AddItem(new CButtonGroup(this, Caption, Alert));
}

void CFloatButtons::BeginGroup(UINT ResID, BOOL Alert)
{
	BeginGroup(CString((LPCSTR)ResID), Alert);
}

void CFloatButtons::BeginNewColumn()
{
	ASSERT(m_ButtonGroups.m_ItemCount);

	m_ButtonGroups[m_ButtonGroups.m_ItemCount-1]->BeginNewColumn();
}

void CFloatButtons::AddButton(UINT nID, INT IconID, BOOL AddRight)
{
	ASSERT(m_ButtonGroups.m_ItemCount);

	m_ButtonGroups[m_ButtonGroups.m_ItemCount-1]->AddButton(nID, IconID, AddRight);
}

void CFloatButtons::AddText(LPCWSTR Text, BOOL Bullet)
{
	ASSERT(m_ButtonGroups.m_ItemCount);

	m_ButtonGroups[m_ButtonGroups.m_ItemCount-1]->AddText(Text, Bullet);
}

void CFloatButtons::AddText(UINT ResID, BOOL Bullet)
{
	ASSERT(m_ButtonGroups.m_ItemCount);

	AddText(ResID ? CString((LPCSTR)ResID) : _T(""), Bullet);
}

void CFloatButtons::SetGroupAlert(UINT nGroup, BOOL Alert)
{
	ASSERT(nGroup<m_ButtonGroups.m_ItemCount);

	m_ButtonGroups[nGroup]->SetGroupAlert(Alert);
}

void CFloatButtons::SetText(UINT nGroup, UINT nID, LPCWSTR Text, BOOL Bullet)
{
	ASSERT(nGroup<m_ButtonGroups.m_ItemCount);

	m_ButtonGroups[nGroup]->SetText(nID, Text, Bullet);
}

void CFloatButtons::SetText(UINT nGroup, UINT nID, UINT ResID, BOOL Bullet)
{
	ASSERT(nGroup<m_ButtonGroups.m_ItemCount);

	SetText(nGroup, nID, ResID ? CString((LPCSTR)ResID) : _T(""), Bullet);
}

void CFloatButtons::AdjustLayout()
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	if (!rectWindow.Width())
		return;

	rectWindow.right -= BACKSTAGEBORDER-INNERBORDER;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollHeight = 0;

	INT Left = BACKSTAGEBORDER-INNERBORDER;
	INT Row = BACKSTAGEBORDER-INNERBORDER;

	UINT FirstInRow = 0;
	INT MaxRowHeight = 0;
	BOOL AllMaxWidth = TRUE;

	for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
	{
		SIZE Size;
		m_ButtonGroups[a]->GetPreferredSize(&Size, rectWindow.Width()-(BACKSTAGEBORDER-INNERBORDER));

		if (FirstInRow!=a)
			if (Left+Size.cx>rectWindow.Width())
			{
				// Extend single button group to full width
				if (FirstInRow!=a-1)
					AllMaxWidth = FALSE;

				// All button groups in row must have equal height
				for (UINT b=FirstInRow; b<a; b++)
					m_ButtonGroups[b]->m_Rect.bottom = Row+MaxRowHeight;

				// Next row
				Left = BACKSTAGEBORDER-INNERBORDER;
				Row += MaxRowHeight+GROUPMARGIN;
				FirstInRow = a;
				MaxRowHeight = 0;
			}

		if (Size.cy>MaxRowHeight)
			MaxRowHeight = Size.cy;

		m_ButtonGroups[a]->m_Rect = CRect(Left, Row, Left+Size.cx, Row+Size.cy);
		Left += Size.cx+GROUPMARGIN;
	}

	m_ScrollHeight = Row+MaxRowHeight+(BACKSTAGEBORDER-INNERBORDER);
	if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars))
	{
		HasScrollbars = TRUE;
		rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
		goto Restart;
	}

	// Adjust group layout
	for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
	{
		// All button groups in row must have equal height
		if (a>=FirstInRow)
			m_ButtonGroups[a]->m_Rect.bottom = Row+MaxRowHeight;

		// Extend single button group to full width
		if (AllMaxWidth)
			m_ButtonGroups[a]->m_Rect.right = rectWindow.Width();
	}

	// Adjust scrollbars
	m_VScrollMax = max(0, m_ScrollHeight-rectWindow.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rectWindow.Height();
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	AdjustScrollbars();
}

void CFloatButtons::AdjustScrollbars()
{
	// Adjust group layout
	for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
		m_ButtonGroups[a]->AdjustLayout(m_VScrollPos);

	// Redraw
	m_BackBufferL = m_BackBufferH = 0;
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


BEGIN_MESSAGE_MAP(CFloatButtons, CFrontstageWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void CFloatButtons::OnDestroy()
{
	// Destorying the button groups may cause focus events to fire!
	m_Destroying = TRUE;

	for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
		delete m_ButtonGroups[a];

	DeleteObject(hBackgroundBrush);

	CFrontstageWnd::OnDestroy();
}

BOOL CFloatButtons::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFloatButtons::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		DeleteObject(hBackgroundBrush);

		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		const BOOL Themed = IsCtrlThemed();

		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
			m_ButtonGroups[a]->Draw(dc, g, m_VScrollPos, Themed);

		CFrontstageWnd::DrawWindowEdge(g, Themed);

		dc.SelectObject(pOldBitmap);

		hBackgroundBrush = CreatePatternBrush(MemBitmap);
	}

	FillRect(pDC, rect, hBackgroundBrush);
}

LRESULT CFloatButtons::OnThemeChanged()
{
	AdjustLayout();

	return NULL;
}

HBRUSH CFloatButtons::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hBrush = CFrontstageWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (hBackgroundBrush)
		if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
		{
			CRect rect;
			pWnd->GetWindowRect(rect);
			ScreenToClient(rect);

			pDC->SetBrushOrg(-rect.left, -rect.top);

			hBrush = hBackgroundBrush;
		}

	return hBrush;
}

void CFloatButtons::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	OnIdleUpdateCmdUI();
	AdjustLayout();
}

void CFloatButtons::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		nInc = -rect.Height()/8;
		break;

	case SB_LINEDOWN:
		nInc = rect.Height()/8;
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
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		AdjustScrollbars();
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CFloatButtons::OnMouseWheel(UINT /*nFlags*/, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return FALSE;

	INT nInc = max(-m_VScrollPos, min(-zDelta*rect.Height()/8, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		AdjustScrollbars();
		FMGetApp()->HideTooltip();
	}

	return TRUE;
}

void CFloatButtons::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (!m_Destroying)
		for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
			if (m_ButtonGroups[a]->SetFocus())
				break;
}

void CFloatButtons::OnIdleUpdateCmdUI()
{
	for (UINT a=0; a<m_ButtonGroups.m_ItemCount; a++)
		m_ButtonGroups[a]->OnIdleUpdateCmdUI();
}
