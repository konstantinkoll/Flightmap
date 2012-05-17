
// CDataGrid.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "CDataGrid.h"
#include "ChooseDetailsDlg.h"
#include "Flightmap.h"
#include "Resource.h"


// CDataGrid
//

#define MINWIDTH     50
#define MAXWIDTH     750
#define MARGIN       3
#define PrepareBlend()                      INT w = min(rect.Width(), RatingBitmapWidth); \
                                            INT h = min(rect.Height(), RatingBitmapHeight);
#define Blend(dc, rect, level, bitmaps)     { HDC hdcMem = CreateCompatibleDC(dc); \
                                            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmaps[level>MaxRating ? 0 : level]); \
                                            AlphaBlend(dc, rect.left, rect.top+1, w, h, hdcMem, 0, 0, w, h, BF); \
                                            SelectObject(hdcMem, hbmOld); \
                                            DeleteDC(hdcMem); }

static const BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

CDataGrid::CDataGrid()
{
	p_Itinerary = NULL;
	p_Edit = NULL;
	hThemeList = hThemeButton = NULL;
	m_HeaderItemClicked = m_SelectedItem.x = m_SelectedItem.y = m_HotItem.x = m_HotItem.y = m_EditLabel.x = m_EditLabel.y = -1;
	m_Hover = m_SpacePressed = m_IgnoreHeaderItemChange = FALSE;
	m_ViewParameters = theApp.m_ViewParameters;
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

void CDataGrid::SetItinerary(CItinerary* pItinerary)
{
	p_Itinerary = pItinerary;

	AdjustLayout();
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

	m_HeaderHeight = wp.cy + (wp.cy ? 1 : 0);

	AdjustScrollbars();
	Invalidate();

	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDataGrid::AdjustHeader()
{
	m_wndHeader.SetRedraw(FALSE);
	m_IgnoreHeaderItemChange = TRUE;

	VERIFY(m_wndHeader.SetOrderArray(FMAttributeCount, m_ViewParameters.ColumnOrder));

	// Width
	for (UINT a=0; a<FMAttributeCount; a++)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_WIDTH;
		HdItem.cxy = m_ViewParameters.ColumnWidth[a];

		if (HdItem.cxy)
			if ((FMAttributes[a].Type==FMTypeRating) || (FMAttributes[a].Type==FMTypeColor) || (FMAttributes[a].Type==FMTypeFlags))
			{
				HdItem.cxy = m_ViewParameters.ColumnWidth[a] = theApp.m_ViewParameters.ColumnWidth[a] = FMAttributes[a].RecommendedWidth;
			}
			else
				if (HdItem.cxy<MINWIDTH)
					m_ViewParameters.ColumnWidth[a] = theApp.m_ViewParameters.ColumnWidth[a] = HdItem.cxy = MINWIDTH;

		m_wndHeader.SetItem(a, &HdItem);
	}

/*	// Sort indicator
	HDITEM hdi;
	hdi.mask = HDI_FORMAT;

	if ((m_HeaderItemSort!=(INT)p_ViewParameters->SortBy) && (m_HeaderItemSort!=-1))
	{
		hdi.fmt = 0;
		m_wndHeader.SetItem(m_HeaderItemSort, &hdi);
	}

	hdi.fmt = p_ViewParameters->Descending ? HDF_SORTDOWN : HDF_SORTUP;
	m_wndHeader.SetItem(p_ViewParameters->SortBy, &hdi);

	m_HeaderItemSort = p_ViewParameters->SortBy;*/

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	m_IgnoreHeaderItemChange = FALSE;
}

/*void CDataGrid::EditLabel(CPoint item)
{
	if (!p_Itinerary)
		return;

	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=FMAttributeCount) || (item.y>=(INT)(p_Itinerary->m_Flights.m_ItemCount+1)))
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
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

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

	CRect rect(x+m_CheckboxSize.cx+m_IconSize.cx+BORDER+2*MARGIN-5, y, x+m_ViewParameters.ColumnWidth[m_ViewParemeters.ColumnOrder[item.x]], y+m_RowHeight);

	p_Edit = new CEdit();
	p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
	p_Edit->SetWindowText(Name);
	p_Edit->SetSel(0, (INT)wcslen(Name));
	p_Edit->SetFont(&theApp.m_DefaultFont);
	p_Edit->SetFocus();
}*/

void CDataGrid::EnsureVisible(CPoint item)
{
	if (!p_Itinerary)
		return;

	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=FMAttributeCount) || (item.y>=(INT)(p_Itinerary->m_Flights.m_ItemCount+1)))
		return;

	CRect rect;
	GetClientRect(rect);

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
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	nInc = 0;
	if (x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[item.x]]>m_HScrollPos+rect.Width())
		nInc = x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[item.x]]-rect.Width()-m_HScrollPos;
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

	INT ScrollHeight = p_Itinerary ? (p_Itinerary->m_Flights.m_ItemCount+1)*m_RowHeight : 0;
	INT ScrollWidth = 0;
	for (UINT a=0; a<FMAttributeCount; a++)
		ScrollWidth += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

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

	if (!p_Itinerary)
		return FALSE;

	point.y -= m_HeaderHeight-m_VScrollPos;

	INT row = (point.y>=0) ? point.y/m_RowHeight : -1;
	if ((row!=-1) && (row<=p_Itinerary->m_Flights.m_ItemCount))
	{
		INT x = -m_HScrollPos;

		for (UINT a=0; a<FMAttributeCount; a++)
		{
			if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]]))
			{
				item->x = a;
				item->y = row;
				return TRUE;
			}

			x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];
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
			x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

		CRect rect(x, m_HeaderHeight+Item.y*m_RowHeight-m_VScrollPos, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]], m_HeaderHeight-m_VScrollPos+(Item.y+1)*m_RowHeight);
		InvalidateRect(rect);
	}
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

void CDataGrid::DrawItem(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect rect)
{
	ASSERT(Attr<FMAttributeCount);

	// Background
	if (FMAttributes[Attr].Type==FMTypeClass)
		switch (Flight.Class)
		{
		case AIRX_Economy:
		case AIRX_EconomyPlus:
			dc.FillSolidRect(rect, 0xE0FFE0);
			break;
		case AIRX_Business:
			dc.FillSolidRect(rect, 0xFFF0E0);
			break;
		case AIRX_First:
			dc.FillSolidRect(rect, 0xE0E0FF);
			break;
		case AIRX_Crew:
			dc.FillSolidRect(rect, 0xD8FFFF);
			break;
		}

	// Foreground
	rect.DeflateRect(MARGIN, 0);

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeColor:
		if (*((COLORREF*)(((BYTE*)&Flight)+FMAttributes[Attr].Offset))!=(COLORREF)-1)
		{
			CRect rectColor(rect.left, rect.top+MARGIN, rect.right, rect.bottom-MARGIN);
			dc.Draw3dRect(rectColor, 0x000000, 0x000000);
			rectColor.DeflateRect(1, 1);
			dc.FillSolidRect(rectColor, Flight.Color);
		}
		break;
	case FMTypeRating:
		{
			rect.top += (rect.Height()-RatingBitmapHeight-1)/2;

			UCHAR Rating = (UCHAR)(*((UINT*)(((BYTE*)&Flight)+FMAttributes[Attr].Offset))>>FMAttributes[Attr].DataParameter);
			PrepareBlend();
			Blend(dc, rect, Rating, theApp.m_RatingBitmaps);
		}
		break;
	default:
		{
			WCHAR tmpStr[256];
			AttributeToString(Flight, Attr, tmpStr, 256);

			dc.DrawText(tmpStr, -1, rect, DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | (((FMAttributes[Attr].Type==FMTypeDistance) || (FMAttributes[Attr].Type==FMTypeUINT) || (Attr==14)) ? DT_RIGHT : DT_LEFT));
		}
	}
}

void CDataGrid::AutosizeColumn(UINT Attr)
{
	DestroyEdit();

	INT Width = MINWIDTH;

	if (p_Itinerary)
	{
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		for (UINT row=0; row<p_Itinerary->m_Flights.m_ItemCount; row++)
		{
			WCHAR tmpStr[256];
			AttributeToString(p_Itinerary->m_Flights.m_Items[row], Attr, tmpStr, 256);
			CSize szText = dc.GetTextExtent(tmpStr, wcslen(tmpStr));

			Width = max(Width, szText.cx);
		}

		dc.SelectObject(pOldFont);
	}

	Width += 2*MARGIN+1;
	m_ViewParameters.ColumnWidth[Attr] = theApp.m_ViewParameters.ColumnWidth[Attr] = min(Width, MAXWIDTH);
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
	ON_COMMAND(IDM_DETAILS_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_DETAILS_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_DETAILS_CHOOSE, OnChooseDetails)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DETAILS_AUTOSIZEALL, IDM_DETAILS_CHOOSE, OnUpdateDetailsCommands)
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
		HdItem.cxy = m_ViewParameters.ColumnWidth[a];
		HdItem.pszText = tmpStr.GetBuffer();
		m_wndHeader.InsertItem(a, &HdItem);
	}

	VERIFY(m_wndHeader.SetOrderArray(FMAttributeCount, m_ViewParameters.ColumnOrder));
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

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (2*(MARGIN-1)+max(abs(lf.lfHeight), 16)) & ~1;

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
	COLORREF lineCol = Themed ? 0xDDDCDA : GetSysColor(COLOR_3DFACE);
	dc.FillSolidRect(rect, bkCol);

	if (p_Itinerary)
	{
		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		INT start = m_VScrollPos/m_RowHeight;
		INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
		for (UINT row=start; row<=p_Itinerary->m_Flights.m_ItemCount; row++)
		{
			INT x = -m_HScrollPos;
			for (UINT col=0; col<FMAttributeCount; col++)
			{
				CRect rectItem(x, y, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]]-1, y+m_RowHeight-1);
				CRect rectIntersect;
				if (rectIntersect.IntersectRect(rectItem, rectUpdate))
				{
					if (!FMAttributes[m_ViewParameters.ColumnOrder[col]].Editable)
						dc.FillSolidRect(rectItem, 0xF5F5F5);

					BOOL Selected = (m_SelectedItem.x==(INT)col) && (m_SelectedItem.y==(INT)row);

					/*if (Selected && (!p_Edit))
						if (hThemeList)
						{
							theApp.zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, rectItem, rectItem);
							//dc.SetTextColor(curCell->pItem->Path[0] ? 0x000000 : 0x808080);
						}
						else
						{
							dc.FillSolidRect(rectItem, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
							//dc.SetTextColor(GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

							if (GetFocus()==this)
							{
								dc.SetBkColor(0x000000);
								dc.DrawFocusRect(rectItem);
							}
						}*/

					if (row<p_Itinerary->m_Flights.m_ItemCount)
						DrawItem(dc, p_Itinerary->m_Flights.m_Items[row], m_ViewParameters.ColumnOrder[col], rectItem);
				}

				x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]];
				dc.FillSolidRect(x-1, y, 1, m_RowHeight-1, lineCol);
			}

			y += m_RowHeight;
			dc.FillSolidRect(0, y-1, x, 1, lineCol);

			if (y>rect.Height())
				break;
			}

		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
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
				if (!m_TooltipCtrl.IsWindowVisible() && (m_HotItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
				{
					ClientToScreen(&point);
					const AIRX_Flight* pFlight = &p_Itinerary->m_Flights.m_Items[m_HotItem.y];
					UINT Attr = m_ViewParameters.ColumnOrder[m_HotItem.x];
					WCHAR tmpStr[256];

					switch (Attr)
					{
					case 0:
						if (strlen(pFlight->From.Code)==3)
							m_TooltipCtrl.Track(point, (CHAR*)&pFlight->From.Code, _T(""));
						break;
					case 3:
						if (strlen(pFlight->To.Code)==3)
							m_TooltipCtrl.Track(point, (CHAR*)&pFlight->To.Code, _T(""));
						break;
					default:
						AttributeToString(p_Itinerary->m_Flights.m_Items[m_HotItem.y], Attr, tmpStr, 256);

						if (tmpStr[0]!=L'\0')
							if (FMAttributes[Attr].Type==FMTypeColor)
							{
								m_TooltipCtrl.Track(point, NULL, NULL, CSize(0, 0), _T(""), tmpStr);
							}
							else
							{
								CClientDC dc(this);

								CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);
								CSize szText = dc.GetTextExtent(tmpStr, wcslen(tmpStr));
								dc.SelectObject(pOldFont);

								if (szText.cx>m_ViewParameters.ColumnWidth[Attr]-2*MARGIN-1)
									m_TooltipCtrl.Track(point, NULL, NULL, CSize(0, 0), _T(""), tmpStr);
							}
					}
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
	if (p_Itinerary)
	{
		CRect rect;
		GetClientRect(rect);

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
	if (p_Itinerary)
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
	if (pWnd->GetSafeHwnd()==m_wndHeader)
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		HDHITTESTINFO htt;
		htt.pt = ptClient;
		m_HeaderItemClicked = m_wndHeader.HitTest(&htt);

		CDialogMenuPopup* pPopup = new CDialogMenuPopup();
		pPopup->Create(this, IDB_MENUDETAILS_32, IDB_MENUDETAILS_16);
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZEALL, 0);
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_DETAILS_CHOOSE, 1);

		pPopup->Track(point);

		return;
	}


	/*	CPoint item(-1, -1);
	if ((point.x<0) || (point.y<0))
	{
		if ((m_SelectedItem.x==-1) || (m_SelectedItem.y==-1))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}

		item = m_SelectedItem;

		point.x = 0;
		for (INT a=0; a<item.x; a++)
			point.x += m_ViewParameters.ColumnWidth[a];
		point.y = (item.y+1)*m_RowHeight+m_HeaderHeight+1;
		ClientToScreen(&point);
	}

	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;
*/
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

void CDataGrid::OnAutosizeAll()
{
	for (UINT a=0; a<FMAttributeCount; a++)
		if (m_ViewParameters.ColumnWidth[a])
			AutosizeColumn(a);

	AdjustHeader();
	AdjustLayout();
}

void CDataGrid::OnAutosize()
{
	if (m_HeaderItemClicked!=-1)
	{
		AutosizeColumn(m_HeaderItemClicked);

		AdjustHeader();
		AdjustLayout();
	}
}

void CDataGrid::OnChooseDetails()
{
	ChooseDetailsDlg dlg(&m_ViewParameters, this);
	if (dlg.DoModal()==IDOK)
	{
		theApp.m_ViewParameters = m_ViewParameters;

		AdjustHeader();
		AdjustLayout();
	}
}

void CDataGrid::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL b = (pCmdUI->m_nID!=IDM_DETAILS_AUTOSIZE);

	if (pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE)
		if (m_HeaderItemClicked!=-1)
			b = (FMAttributes[m_HeaderItemClicked].Type!=FMTypeRating) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeColor) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags);

	pCmdUI->Enable(b);
}

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
		// GetColumnOrderArray() enthält noch die alte Reihenfolge, daher:
		// 1. Spalte an der alten Stelle löschen
		for (UINT a=0; a<FMAttributeCount; a++)
			if (m_ViewParameters.ColumnOrder[a]==pHdr->iItem)
			{
				for (UINT b=a; b<FMAttributeCount-1; b++)
					m_ViewParameters.ColumnOrder[b] = m_ViewParameters.ColumnOrder[b+1];
				break;
			}

		// 2. Spalte an der neuen Stelle einfügen
		for (INT a=FMAttributeCount-1; a>pHdr->pitem->iOrder; a--)
			m_ViewParameters.ColumnOrder[a] = m_ViewParameters.ColumnOrder[a-1];

		m_ViewParameters.ColumnOrder[pHdr->pitem->iOrder] = pHdr->iItem;
		memcpy_s(theApp.m_ViewParameters.ColumnOrder, sizeof(theApp.m_ViewParameters.ColumnOrder), m_ViewParameters.ColumnOrder, sizeof(theApp.m_ViewParameters.ColumnOrder));
		AdjustLayout();

		*pResult = FALSE;
	}
}

void CDataGrid::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
		*pResult = (FMAttributes[pHdr->iItem].Type==FMTypeRating) || (FMAttributes[pHdr->iItem].Type==FMTypeColor) || (FMAttributes[pHdr->iItem].Type==FMTypeFlags) || (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
}

void CDataGrid::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if ((pHdr->pitem->mask & HDI_WIDTH) && (!m_IgnoreHeaderItemChange))
	{
		if (pHdr->pitem->cxy<MINWIDTH)
			pHdr->pitem->cxy = (pHdr->iItem==0) || (pHdr->iItem==3) ? MINWIDTH : pHdr->pitem->cxy<15 ? 0 : MINWIDTH;

		m_ViewParameters.ColumnWidth[pHdr->iItem] = theApp.m_ViewParameters.ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
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
