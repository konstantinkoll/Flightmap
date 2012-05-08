
// CDataGrid.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "CDataGrid.h"
#include "ChooseDetailsDlg.h"
#include "Flightmap.h"
#include "Resource.h"


// CDataGrid
//

#define MINWIDTH     75
#define MAXWIDTH     750

#define BORDER       3
#define MARGIN       4
#define GUTTER       13

CDataGrid::CDataGrid()
{
	p_Edit = NULL;
	m_Rows = m_Cols = 0;
	hThemeList = hThemeButton = NULL;
	m_SelectedItem.x = m_SelectedItem.y = m_HotItem.x = m_HotItem.y = m_EditLabel.x = m_EditLabel.y = -1;
	m_Hover = m_SpacePressed = m_IgnoreHeaderItemChange = FALSE;
}

CDataGrid::~CDataGrid()
{
	DestroyEdit();
}

BOOL CDataGrid::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CDataGrid::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (p_Edit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
				return TRUE;
			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;
			}
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (p_Edit)
			return TRUE;
		break;
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

void CDataGrid::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	m_HeaderHeight = wp.cy + (wp.cy ? 4 : 0);

	AdjustScrollbars();
	Invalidate();

	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDataGrid::AutosizeColumns()
{
	m_wndHeader.SetRedraw(FALSE);

	for (UINT col=0; col<m_Cols; col++)
		AutosizeColumn(col);

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	AdjustScrollbars();
	Invalidate();
}

/*void CDataGrid::EditLabel(CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
	{
		m_EditLabel.x = m_EditLabel.y = -1;
		return;
	}
	if (!(m_Tree[MAKEPOSI(item)].Flags & CF_CANRENAME))
	{
		m_EditLabel.x = m_EditLabel.y = -1;
		return;
	}

	EnsureVisible(item);

	INT y = m_HeaderHeight+item.y*m_RowHeight;
	INT x = 0;
	for (INT a=0; a<item.x; a++)
		x += m_ViewParameters.ColumnWidth[a];

	WCHAR Name[MAX_PATH];
	wcscpy_s(Name, MAX_PATH, m_Tree[MAKEPOSI(item)].pItem->Name);

	IShellFolder* pParentFolder = NULL;
	if (SUCCEEDED(SHBindToParent(m_Tree[MAKEPOSI(item)].pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
	{
		STRRET strret;
		if (SUCCEEDED(pParentFolder->GetDisplayNameOf(m_Tree[MAKEPOSI(item)].pItem->pidlRel, SHGDN_FOREDITING, &strret)))
			if (strret.uType==STRRET_WSTR)
				wcscpy_s(Name, MAX_PATH, strret.pOleStr);

		pParentFolder->Release();
	}

	CRect rect(x+m_CheckboxSize.cx+m_IconSize.cx+GUTTER+BORDER+2*MARGIN-5, y, x+m_ViewParameters.ColumnWidth[item.x], y+m_RowHeight);

	p_Edit = new CEdit();
	p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
	p_Edit->SetWindowText(Name);
	p_Edit->SetSel(0, (INT)wcslen(Name));
	p_Edit->SetFont(&theApp.m_DefaultFont);
	p_Edit->SetFocus();
}*/

void CDataGrid::EnsureVisible(CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if ((INT)((item.y+1)*m_RowHeight)>m_VScrollPos+rect.Height()-(INT)m_HeaderHeight)
		nInc = (item.y+1)*m_RowHeight-rect.Height()+(INT)m_HeaderHeight-m_VScrollPos;
	if ((INT)(item.y*m_RowHeight)<m_VScrollPos+nInc)
		nInc = item.y*m_RowHeight-m_VScrollPos;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	// Horizontal
	INT x = 0;
	for (INT a=0; a<item.x; a++)
		x += m_ViewParameters.ColumnWidth[a];

	nInc = 0;
	if (x+m_ViewParameters.ColumnWidth[item.x]>m_HScrollPos+rect.Width())
		nInc = x+m_ViewParameters.ColumnWidth[item.x]-rect.Width()-m_HScrollPos;
	if (x<m_HScrollPos+nInc)
		nInc = x-m_HScrollPos;

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
}

void CDataGrid::ResetScrollbars()
{
	ScrollWindowEx(0, m_VScrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
	ScrollWindowEx(m_HScrollPos, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
	m_VScrollPos = m_HScrollPos = 0;
	SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
	SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);
}

void CDataGrid::AdjustScrollbars()
{
	CRect rect;
	GetWindowRect(&rect);

	INT ScrollHeight = m_Rows*m_RowHeight;
	INT ScrollWidth = (m_Cols<FMAttributeCount) ? GUTTER : 0;
	for (UINT col=0; col<m_Cols; col++)
		ScrollWidth += m_ViewParameters.ColumnWidth[col];

	BOOL HScroll = FALSE;
	if (ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}
	if (ScrollHeight>rect.Height()-(INT)m_HeaderHeight)
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);
	if ((ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
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

BOOL CDataGrid::HitTest(CPoint point, CPoint* item)
{
	ASSERT(item);

	point.y -= m_HeaderHeight-m_VScrollPos;

	INT row = (point.y>=0) ? point.y/m_RowHeight : -1;
	if (row!=-1)
	{
		INT x = -m_HScrollPos;

		for (UINT a=0; a<min(m_Cols+1, FMAttributeCount); a++)
		{
			if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[a]))
			{
				item->x = a;
				item->y = row;
				return TRUE;
			}

			x += m_ViewParameters.ColumnWidth[a];
		}
	}

	return FALSE;
}

void CDataGrid::InvalidateItem(CPoint Item)
{
	if ((Item.x!=-1) && (Item.y!=-1))
	{
		INT x = -m_HScrollPos;
		for (UINT a=0; a<(UINT)Item.x; a++)
			x += m_ViewParameters.ColumnWidth[a];

		CRect rect(x, m_HeaderHeight+Item.y*m_RowHeight-m_VScrollPos, x+m_ViewParameters.ColumnWidth[Item.x], m_HeaderHeight-m_VScrollPos+(Item.y+1)*m_RowHeight);
		InvalidateRect(rect);
	}
}

void CDataGrid::TrackMenu(UINT nID, CPoint point, INT col)
{
/*	CMenu menu;
	ENSURE(menu.LoadMenu(nID));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	if (!col)
		pPopup->EnableMenuItem(IDD_CHOOSEPROPERTY, MF_GRAYED | MF_DISABLED);

	if ((!col) || (m_ColumnMapping[col]==-1))
		pPopup->EnableMenuItem(IDM_TREE_RESETPROPERTY, MF_GRAYED | MF_DISABLED);

	BOOL Enable = FALSE;
	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].Flags & CF_CANEXPAND)
		{
			Enable = TRUE;
			break;
		}

	if (!Enable)
		pPopup->EnableMenuItem(IDM_TREE_EXPANDCOLUMN, MF_GRAYED | MF_DISABLED);

	switch (pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this))
	{
	case IDM_TREE_AUTOSIZE:
		AutosizeColumn(col);
		AdjustScrollbars();
		Invalidate();
		break;
	case IDM_VIEW_AUTOSIZEALL:
		AutosizeColumns();
		break;
	case IDM_TREE_EXPANDCOLUMN:
		ExpandColumn(col);
		break;
	case IDD_CHOOSEPROPERTY:
		PostMessage(IDD_CHOOSEPROPERTY, (WPARAM)col);
		break;
	case IDM_TREE_RESETPROPERTY:
		m_ColumnMapping[col] = -1;
		UpdateColumnCaption(col);
		break;
	}*/
}

void CDataGrid::SelectItem(CPoint Item)
{
	if (Item==m_SelectedItem)
		return;

	InvalidateItem(m_SelectedItem);
	m_SelectedItem = Item;
	EnsureVisible(Item);
	InvalidateItem(Item);
	m_EditLabel = CPoint(-1, -1);

	ReleaseCapture();
}

/*void CDataGrid::UpdateColumnCaption(UINT col)
{
	ASSERT(col<m_Cols);

	CString caption = GetColumnCaption(col);

	HDITEM HdItem;
	HdItem.mask = HDI_TEXT;
	HdItem.pszText = caption.GetBuffer();
	m_wndHeader.SetItem(col, &HdItem);
}*/

void CDataGrid::AutosizeColumn(UINT col, BOOL OnlyEnlarge)
{
	DestroyEdit();

	INT Width = 0;
/*	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].pItem)
			Width = max(Width, m_Tree[MAKEPOS(row, col)].pItem->Width);*/

	//Width += GUTTER+2*BORDER+m_CheckboxSize.cx+m_IconSize.cx+3*MARGIN;
	m_ViewParameters.ColumnWidth[col] = min(OnlyEnlarge ? max(Width, m_ViewParameters.ColumnWidth[col]) : Width, MAXWIDTH);

	HDITEM HdItem;
	HdItem.mask = HDI_WIDTH;
	HdItem.cxy = m_ViewParameters.ColumnWidth[col];
	m_wndHeader.SetItem(col, &HdItem);
}

void CDataGrid::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		CPoint item = m_EditLabel;
		if ((item.x==-1) || (item.y==-1))
			item = m_SelectedItem;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		CString tmpStr;
		victim->GetWindowText(tmpStr);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (item.x!=-1) && (item.y!=-1))
		{
			/*Cell* cell = &m_Tree[MAKEPOSI(item)];
			if (cell->pItem)
				if (Name!=cell->pItem->Name)
				{
					IShellFolder* pParentFolder = NULL;
					if (SUCCEEDED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
					{
						LPITEMIDLIST pidlRel = NULL;
						if (SUCCEEDED(pParentFolder->SetNameOf(m_hWnd, cell->pItem->pidlRel, Name, SHGDN_NORMAL, &pidlRel)))
						{
							LPITEMIDLIST pidlParent = NULL;
							theApp.GetShellManager()->GetParentItem(cell->pItem->pidlFQ, pidlParent);

							SetItem(item.y, item.x, pidlRel, theApp.GetShellManager()->ConcatenateItem(pidlParent, pidlRel), cell->Flags);
							UpdateChildPIDLs(item.y, item.x);
							InvalidateItem(item);

							theApp.GetShellManager()->FreeItem(pidlParent);
						}

						pParentFolder->Release();
					}
				}*/
		}
	}

	m_EditLabel.x = m_EditLabel.y = -1;
}


BEGIN_MESSAGE_MAP(CDataGrid, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
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
	ON_WM_MOUSEHWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)
	ON_NOTIFY(HDN_ENDDRAG, 1, OnEndDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
	ON_NOTIFY(HDN_ITEMCLICK, 1, OnItemClick)
	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

INT CDataGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | HDS_FLAT | HDS_HORZ | HDS_FULLDRAG | HDS_BUTTONS | CCS_TOP | CCS_NOMOVEY | CCS_NODIVIDER;
	CRect rect;
	rect.SetRectEmpty();
	if (!m_wndHeader.Create(dwStyle, rect, this, 1))
		return -1;

	m_IgnoreHeaderItemChange = TRUE;

	for (UINT a=0; a<FMAttributeCount; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(FMAttributes[a].nNameID));

		HDITEM HdItem;
		HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
		HdItem.fmt = HDF_STRING | HDF_CENTER;
		HdItem.cxy = MINWIDTH;//m_ColumnWidth[idx];
		HdItem.pszText = tmpStr.GetBuffer();
		m_wndHeader.InsertItem(a, &HdItem);
	}

	m_IgnoreHeaderItemChange = FALSE;

	m_TooltipCtrl.Create(this);

	if (theApp.m_ThemeLibLoaded)
	{
		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
		{
			theApp.zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
	}

	IMAGEINFO ii;
	theApp.m_SystemImageListSmall.GetImageInfo(0, &ii);
	m_IconSize.cx = ii.rcImage.right-ii.rcImage.left;
	m_IconSize.cy = ii.rcImage.bottom-ii.rcImage.top;

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (4+max(abs(lf.lfHeight), m_IconSize.cy)) & ~1;

	for (UINT a=0; a<FMAttributeCount; a++)
		m_ViewParameters.ColumnWidth[a] = MINWIDTH;

	ResetScrollbars();

	return 0;
}

void CDataGrid::OnDestroy()
{
	if (hThemeButton)
		theApp.zCloseThemeData(hThemeButton);
	if (hThemeList)
		theApp.zCloseThemeData(hThemeList);

	CWnd::OnDestroy();
}

LRESULT CDataGrid::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hThemeButton)
			theApp.zCloseThemeData(hThemeButton);
		if (hThemeList)
			theApp.zCloseThemeData(hThemeList);

		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}

BOOL CDataGrid::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CDataGrid::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

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
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	LOGBRUSH brsh;
	brsh.lbColor = 0x808080;
	brsh.lbStyle = PS_SOLID;
	CPen pen(PS_COSMETIC | PS_ALTERNATE, 1, &brsh);
	CPen* pOldPen = dc.SelectObject(&pen);

	INT start = m_VScrollPos/m_RowHeight;
/*	INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
	Cell* curCell = &m_Tree[MAKEPOS(start, 0)];
	for (UINT row=start; row<m_Rows; row++)
	{
		INT x = -m_HScrollPos;
		for (UINT col=0; col<FMAttributeCount; col++)
		{
			CRect rectItem(x, y, x+m_ViewParameters.ColumnWidth[col], y+m_RowHeight);
			CRect rectIntersect;
			if (rectIntersect.IntersectRect(rectItem, rectUpdate))
			{
				rectItem.left += GUTTER;

				BOOL Hot = (m_HotItem.x==(INT)col) && (m_HotItem.y==(INT)row);
				BOOL Selected = (m_SelectedItem.x==(INT)col) && (m_SelectedItem.y==(INT)row);

				if (curCell->pItem)
				{
					if (hThemeList)
					{
						if (Hot | Selected)
						{
							const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
							UINT State = 0;
							if (Hot)
								State |= 1;
							if (Selected)
								State |= 2;
								theApp.zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], rectItem, rectItem);
						}

						dc.SetTextColor(curCell->pItem->Path[0] ? 0x000000 : 0x808080);
					}
					else
						if (Selected)
						{
							dc.FillSolidRect(rectItem, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
							dc.SetTextColor(GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

							if (GetFocus()==this)
							{
								dc.SetBkColor(0x000000);
								dc.DrawFocusRect(rectItem);
							}
						}
						else
						{
							dc.SetTextColor(curCell->pItem->Path[0] ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : Themed ? 0x808080 : GetSysColor(COLOR_GRAYTEXT));
						}

					rectItem.left += m_CheckboxSize.cx+BORDER+MARGIN;
					theApp.m_SystemImageListSmall.Draw(&dc, Selected ? curCell->pItem->IconIDSelected : curCell->pItem->IconIDNormal, CPoint(rectItem.left, y+(m_RowHeight-m_IconSize.cy)/2), ILD_TRANSPARENT);
					rectItem.left += m_IconSize.cx+MARGIN;
					rectItem.right -= BORDER;

					CRect rectButton(x+GUTTER+BORDER, y+(m_RowHeight-m_CheckboxSize.cy)/2, x+GUTTER+BORDER+m_CheckboxSize.cx, y+(m_RowHeight-m_CheckboxSize.cy)/2+m_CheckboxSize.cy);
					if (hThemeButton)
					{
						INT uiStyle;
						if (curCell->pItem->Path[0])
						{
							uiStyle = (Selected && (m_SpacePressed || m_CheckboxPressed)) ? CBS_UNCHECKEDPRESSED : (Hot && m_CheckboxHot) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
							if (curCell->Flags & CF_CHECKED)
								uiStyle += 4;
						}
						else
						{
							uiStyle = CBS_UNCHECKEDDISABLED;
						}
						theApp.zDrawThemeBackground(hThemeButton, dc, BP_CHECKBOX, uiStyle, rectButton, rectButton);
					}
					else
					{
						UINT uiStyle = DFCS_BUTTONCHECK;
						if (curCell->pItem->Path[0])
						{
							uiStyle |= (curCell->Flags & CF_CHECKED ? DFCS_CHECKED : 0) | ((Selected && (m_SpacePressed || m_CheckboxPressed)) ? DFCS_PUSHED : 0);
						}
						else
						{
							uiStyle |= DFCS_INACTIVE;
						}
						dc.DrawFrameControl(rectButton, DFC_BUTTON, uiStyle);
					}

					dc.DrawText(curCell->pItem->Name, -1, rectItem, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

					dc.MoveTo(x+((curCell->Flags & CF_ISSIBLING) ? GUTTER/2 : 0), y+m_RowHeight/2);
					dc.LineTo(x+GUTTER+BORDER-1, y+m_RowHeight/2);

					if (curCell->Flags & (CF_HASCHILDREN | CF_CANEXPAND))
					{
						INT right = x+GUTTER+BORDER+m_CheckboxSize.cx+m_IconSize.cx+2*MARGIN+curCell->pItem->Width+1;
						if (right<x+m_ViewParameters.ColumnWidth[col])
						{
							dc.MoveTo(x+m_ViewParameters.ColumnWidth[col], y+m_RowHeight/2);
							dc.LineTo(right, y+m_RowHeight/2);
						}
					}
				}

				if (curCell->Flags & CF_HASSIBLINGS)
				{
					dc.MoveTo(x+GUTTER/2, y+m_RowHeight/2);
					dc.LineTo(x+GUTTER/2, y+m_RowHeight);
				}

				if (curCell->Flags & CF_ISSIBLING)
				{
					dc.MoveTo(x+GUTTER/2, y);
					dc.LineTo(x+GUTTER/2, y+m_RowHeight/2);
				}

				if (col)
					if ((curCell-1)->Flags & (CF_CANEXPAND | CF_CANCOLLAPSE))
					{
						dc.MoveTo(x, y+m_RowHeight/2);
						dc.LineTo(x+2, y+m_RowHeight/2);

						CRect rectGlyph(x, y+(m_RowHeight-m_GlyphSize.cy)/2, x+m_GlyphSize.cx, y+(m_RowHeight-m_GlyphSize.cy)/2+m_GlyphSize.cy);
						if (hThemeTree)
						{
							if (theApp.OSVersion==OS_XP)
							{
								rectGlyph.OffsetRect(2, 1);
							}
							else
							{
								rectGlyph.OffsetRect(1-m_GlyphSize.cx/4, 0);
							}

							BOOL Hot = (m_HotExpando.x==(INT)col) && (m_HotExpando.y==(INT)row) && (theApp.OSVersion>OS_XP);
							theApp.zDrawThemeBackground(hThemeTree, dc, Hot ? TVP_HOTGLYPH : TVP_GLYPH, (curCell-1)->Flags & CF_CANEXPAND ? GLPS_CLOSED : GLPS_OPENED, rectGlyph, rectGlyph);
						}
						else
						{
							rectGlyph.OffsetRect(1, 0);
							m_DefaultGlyphs.Draw(&dc, (curCell-1)->Flags & CF_CANEXPAND ? 0 : 1, rectGlyph.TopLeft(), 0);
						}
					}
			}

			x += m_ViewParameters.ColumnWidth[col];
			curCell++;
		}

		y += m_RowHeight;
		if (y>rect.Height())
			break;
	}

*/
	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CDataGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDataGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		nInc = -((INT)m_RowHeight);
		break;
	case SB_LINEDOWN:
		nInc = m_RowHeight;
		break;
	case SB_PAGEUP:
		nInc = min(-1, -(rect.Height()-(INT)m_HeaderHeight));
		break;
	case SB_PAGEDOWN:
		nInc = max(1, rect.Height()-(INT)m_HeaderHeight);
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
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(&rect);
			ScreenToClient(&rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDataGrid::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);

		UpdateWindow();
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CDataGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

void CDataGrid::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL Dragging = (GetCapture()==this);
	BOOL Pressed = FALSE;
//	BOOL CheckboxHot = m_CheckboxHot;
	CPoint Item(-1, -1);
	BOOL OnItem = HitTest(point, &Item/*, Dragging ? &Pressed : &m_CheckboxHot, &Expando*/);

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
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
			m_TooltipCtrl.Deactivate();

	if (!Dragging)
	{
		if (m_HotItem!=Item)
		{
			InvalidateItem(m_HotItem);
			m_HotItem = Item;
			InvalidateItem(m_HotItem);
		}

		if ((OnItem) && (nFlags & MK_RBUTTON))
		{
			SetFocus();
			m_SelectedItem = m_HotItem;
		}
	}

//	m_CheckboxPressed = (Item==m_SelectedItem) && Pressed && Dragging;
}

void CDataGrid::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem.x = m_HotItem.y = -1;
}

void CDataGrid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem.x!=-1) && (m_HotItem.y!=-1) && /*(!m_CheckboxHot) &&*/ (!p_Edit))
			if (m_HotItem==m_EditLabel)
			{
				m_TooltipCtrl.Deactivate();
				//EditLabel(m_EditLabel);
			}
			else
				if (!m_TooltipCtrl.IsWindowVisible())
				{
					/*HICON hIcon = NULL;
					CSize size(0, 0);
					CString caption;
					CString hint;
					TooltipDataFromPIDL(m_Tree[MAKEPOSI(m_HotItem)].pItem->pidlFQ, &theApp.m_SystemImageListExtraLarge, hIcon, size, caption, hint);

					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, hIcon, size, caption, hint);*/
				}
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

BOOL CDataGrid::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CDataGrid::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CDataGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_Rows)
	{
		CRect rect;
		GetClientRect(&rect);

		CPoint item(m_SelectedItem);

/*		switch (nChar)
		{
		case VK_F2:
			EditLabel();
			break;
		case VK_EXECUTE:
		case VK_RETURN:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANEXPAND)
			{
				ExpandFolder();
			}
			else
			{
				OpenFolder();
			}

			break;
		case VK_DELETE:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANDELETE)
				DeleteFolder();

			break;
		case VK_SPACE:
			if (m_Tree[MAKEPOSI(item)].pItem->Path[0])
			{
				m_SpacePressed = TRUE;
				InvalidateItem(item);
			}

			break;
		case VK_LEFT:
			if (GetKeyState(VK_CONTROL)<0)
			{
				if (m_Tree[MAKEPOSI(item)].Flags & CF_CANCOLLAPSE)
					Collapse(item.y, item.x);
			}
			else
				if (item.x)
				{
					item.x--;
					for (INT row=item.y; row>=0; row--)
						if (m_Tree[MAKEPOS(row, item.x)].pItem)
						{
							item.y = row;
							break;
						}
				}

			break;
		case VK_RIGHT:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANEXPAND)
				Expand(item.y, item.x, FALSE);

			if ((item.x<(INT)m_Cols-1) && (GetKeyState(VK_CONTROL)>=0))
				for (INT row=item.y; row<(INT)m_Rows; row++)
					if (m_Tree[MAKEPOS(row, item.x+1)].pItem)
					{
						item.x++;
						item.y = row;
						break;
					}

			break;
		case VK_UP:
			for (INT row=item.y-1; row>=0; row--)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					break;
				}

			break;
		case VK_PRIOR:
			for (INT row=item.y-1; row>=0; row--)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					if (row<=m_SelectedItem.y-rect.Height()/(INT)m_RowHeight)
						break;
				}

			break;
		case VK_DOWN:
			for (INT row=item.y+1; row<(INT)m_Rows; row++)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					break;
				}

			break;
		case VK_NEXT:
			for (INT row=item.y+1; row<(INT)m_Rows; row++)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					if (row>=m_SelectedItem.y+rect.Height()/(INT)m_RowHeight)
						break;
				}

			break;
		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.x = item.y = 0;
			}
			else
				for (INT col=item.x; col>=0; col--)
					if (m_Tree[MAKEPOS(item.y, col)].pItem)
					{
						item.x = col;
					}
					else
						break;

			break;
		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.x = m_Cols-1;
				item.y = m_Rows-1;
				while ((item.x>0) || (item.y>0))
				{
					if (m_Tree[MAKEPOS(item.y, item.x)].pItem)
						break;

					item.y--;
					if (item.y<0)
					{
						item.y = m_Rows-1;
						item.x--;
					}
				}
			}
			else
				for (INT col=item.x; col<(INT)m_Cols; col++)
					if (m_Tree[MAKEPOS(item.y, col)].pItem)
					{
						item.x = col;
					}
					else
						break;

			break;
		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			return;
		}*/

		SelectItem(item);
	}
}

void CDataGrid::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_Rows)
		switch (nChar)
		{
		case VK_SPACE:
			if (m_SpacePressed)
			{
/*				if (m_Tree[MAKEPOSI(m_SelectedItem)].pItem->Path[0])
				{
					m_Tree[MAKEPOSI(m_SelectedItem)].Flags ^= CF_CHECKED;
					InvalidateItem(m_SelectedItem);
				}*/

				m_SpacePressed = FALSE;
			}

			break;
		default:
			CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}
}

void CDataGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item))
	{
		if (Item==m_SelectedItem)
		{
			m_EditLabel = m_SelectedItem;
		}
		else
		{
			SelectItem(Item);
		}
	}
}

void CDataGrid::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture()==this)
	{
		CPoint Item;
		if (HitTest(point, &Item))
			if (Item==m_SelectedItem) ;
				/*if (nFlags & MK_CONTROL)
				{
					m_Tree[MAKEPOSI(Item)].Flags ^= CF_CHECKED;
					InvalidateItem(Item);
				}
				else
				{
					SetBranchCheck(!(m_Tree[MAKEPOSI(Item)].Flags & CF_CHECKED), Item);
				}

		m_CheckboxPressed = FALSE;*/
		
		ReleaseCapture();
	}
}

void CDataGrid::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
//	m_CheckboxPressed = FALSE;
	ReleaseCapture();

	CPoint Item;
	if (HitTest(point, &Item))
		if (Item==m_SelectedItem) ;
}

void CDataGrid::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item))
		SelectItem(Item);
}

void CDataGrid::OnContextMenu(CWnd* pWnd, CPoint point)
{
/*	CPoint item(-1, -1);
	if ((point.x<0) || (point.y<0))
	{
		if ((m_SelectedItem.x==-1) || (m_SelectedItem.y==-1))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}

		item = m_SelectedItem;

		point.x = GUTTER;
		for (INT a=0; a<item.x; a++)
			point.x += m_ViewParameters.ColumnWidth[a];
		point.y = (item.y+1)*m_RowHeight+m_HeaderHeight+1;
		ClientToScreen(&point);
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		if (pWnd->GetSafeHwnd()==m_wndHeader)
		{
			HDHITTESTINFO htt;
			htt.pt = ptClient;
			TrackMenu(IDM_HEADER, point, m_wndHeader.HitTest(&htt));
			return;
		}

		if (!HitTest(ptClient, &item))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}
	}

	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	Cell* cell = &m_Tree[MAKEPOSI(item)];
	if (!cell->pItem)
		return;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetSafeHwnd(), 1, (LPCITEMIDLIST*)&cell->pItem->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_CANRENAME;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				InsertMenu(hPopup, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

				CString tmpStr;
				ENSURE(tmpStr.LoadString(IDS_INCLUDEBRANCH));
				InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7001, tmpStr);

				ENSURE(tmpStr.LoadString(IDS_EXCLUDEBRANCH));
				InsertMenu(hPopup, 1, MF_BYPOSITION, 0x7002, tmpStr);

				if (cell->Flags & CF_CANEXPAND)
				{
					ENSURE(tmpStr.LoadString(IDS_EXPAND));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7003, tmpStr);

					ENSURE(tmpStr.LoadString(IDS_EXPANDBRANCH));
					InsertMenu(hPopup, 1, MF_BYPOSITION, 0x7004, tmpStr);

					SetMenuDefaultItem(hPopup, 0x7003, FALSE);
				}

				if (cell->Flags & CF_CANCOLLAPSE)
				{
					ENSURE(tmpStr.LoadString(IDS_COLLAPSE));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7005, tmpStr);
				}

				if (item.x)
				{
					CString tmpStr;
					ENSURE(tmpStr.LoadString(IDS_CHOOSEPROPERTY));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7000, tmpStr);

					InsertMenu(hPopup, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				}

				pcm->QueryInterface(IID_IContextMenu2, (void**)&m_pContextMenu2);
				UINT idCmd = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, GetSafeHwnd(), NULL);
				if (m_pContextMenu2)
				{
					m_pContextMenu2->Release();
					m_pContextMenu2 = NULL;
				}

				switch (idCmd)
				{
				case 0x7000:
					OnChooseProperty((WPARAM)item.x, NULL);
					break;
				case 0x7001:
					SetBranchCheck(TRUE, item);
					break;
				case 0x7002:
					SetBranchCheck(FALSE, item);
					break;
				case 0x7003:
					ExpandFolder(item, FALSE);
					break;
				case 0x7004:
					ExpandFolder(item, TRUE);
					break;
				case 0x7005:
					Collapse(item.y, item.x);
					break;
				case 0:
					break;
				default:
					{
						CHAR Verb[256] = "";
						pcm->GetCommandString(idCmd-1, GCS_VERBA, NULL, Verb, 256);

						if (strcmp(Verb, "rename")==0)
						{
							EditLabel(item);
						}
						else
						{
							CWaitCursor wait;

							CMINVOKECOMMANDINFO cmi;
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask = 0;
							cmi.hwnd = GetSafeHwnd();
							cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd-1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory = NULL;
							cmi.nShow = SW_SHOWNORMAL;
							cmi.dwHotKey = 0;
							cmi.hIcon = NULL;

							pcm->InvokeCommand(&cmi);

							SetFocus();
						}
					}
				}
			}
		}

		pcm->Release();
	}

	pParentFolder->Release();*/
}

void CDataGrid::OnSetFocus(CWnd* /*pOldWnd*/)
{
	InvalidateItem(m_SelectedItem);
}

void CDataGrid::OnKillFocus(CWnd* /*pNewWnd*/)
{
	m_SpacePressed = FALSE;
	InvalidateItem(m_SelectedItem);
}

/*void CDataGrid::OnAutosizeAll()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		if (m_ViewParameters.ColumnWidth[a])
			AutosizeColumn(a);

	AdjustHeader(TRUE);
	AdjustLayout();
}*/

/*void CDataGrid::OnAutosize()
{
	if (m_HeaderItemClicked!=-1)
	{
		AutosizeColumn(m_HeaderItemClicked);
		AdjustHeader(TRUE);
		AdjustLayout();
	}
}*/

/*void CDataGrid::OnChooseDetails()
{
	ChooseDetailsDlg dlg(this, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(m_Context);
}*/

/*void CDataGrid::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_ViewParameters.Mode==LFViewDetails);

	if (pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE)
		b &= (m_HeaderItemClicked!=-1);

	pCmdUI->Enable(b);
}*/

void CDataGrid::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	*pResult = (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
}

void CDataGrid::OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_ORDER)
	{
		// GetColumnOrderArray() enth�lt noch die alte Reihenfolge, daher:
		// 1. Spalte an der alten Stelle l�schen
		for (UINT a=0; a<FMAttributeCount; a++)
			if (m_ViewParameters.ColumnOrder[a]==pHdr->iItem)
			{
				for (UINT b=a; b<FMAttributeCount-1; b++)
					m_ViewParameters.ColumnOrder[b] = m_ViewParameters.ColumnOrder[b+1];
				break;
			}

		// 2. Spalte an der neuen Stelle einf�gen
		for (INT a=FMAttributeCount-1; a>pHdr->pitem->iOrder; a--)
			m_ViewParameters.ColumnOrder[a] = m_ViewParameters.ColumnOrder[a-1];

		m_ViewParameters.ColumnOrder[pHdr->pitem->iOrder] = pHdr->iItem;
		//UpdateViewOptions(m_Context);

		*pResult = FALSE;
	}
}

void CDataGrid::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
		*pResult = /*(theApp.m_Attributes[pHdr->iItem]->Type==LFTypeRating) ||*/ (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
}

void CDataGrid::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if ((pHdr->pitem->mask & HDI_WIDTH) && (!m_IgnoreHeaderItemChange))
	{
//		if (pHdr->pitem->cxy<MINWIDTH)
//			pHdr->pitem->cxy = (pHdr->iItem==LFAttrFileName) ? MINWIDTH : 0;

		m_ViewParameters.ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
		AdjustLayout();

		*pResult = FALSE;
	}
}

void CDataGrid::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;
	UINT attr = pHdr->iItem;

/*	if (!AttributeSortableInView(attr, m_ViewParameters.Mode))
	{
		CString msg;
		ENSURE(msg.LoadString(IDS_ATTRIBUTENOTSORTABLE));
		MessageBox(msg, theApp.m_Attributes[attr]->Name, MB_OK | MB_ICONWARNING);
	
		return;
	}*/

	/*if (p_ViewParameters->SortBy==attr)
	{
		p_ViewParameters->Descending = !p_ViewParameters->Descending;
	}
	else
	{
		p_ViewParameters->SortBy = attr;
		p_ViewParameters->Descending = theApp.m_Attributes[attr]->PreferDescendingSort;
	}
	theApp.UpdateSortOptions(m_Context);*/

	*pResult = NULL;
}

void CDataGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
