
// CDataGrid.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AddRouteDlg.h"
#include "CDataGrid.h"
#include "ChooseDetailsDlg.h"
#include "CMainWnd.h"
#include "EditFlightDlg.h"
#include "FilterDlg.h"
#include "FindReplaceDlg.h"
#include "Flightmap.h"


// CDataGrid
//

#define MINWIDTH      50
#define MAXWIDTH      750
#define MARGIN        3
#define FLAGCOUNT     7
#define PrepareBlend()                      INT w = min(rect.Width(), RatingBitmapWidth); \
                                            INT h = min(rect.Height(), RatingBitmapHeight);
#define Blend(dc, rect, level, bitmaps)     { HDC hdcMem = CreateCompatibleDC(dc); \
                                            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, bitmaps[level>MaxRating ? 0 : level]); \
                                            AlphaBlend(dc, rect.left, rect.top+1, w, h, hdcMem, 0, 0, w, h, BF); \
                                            SelectObject(hdcMem, hOldBitmap); \
                                            DeleteDC(hdcMem); }

static const BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
static const UINT DisplayFlags[] = { 0, AIRX_AwardFlight, AIRX_GroundTransportation, AIRX_BusinessTrip, AIRX_LeisureTrip, AIRX_Upgrade, AIRX_Cancelled };

CDataGrid::CDataGrid()
	: CWnd()
{
	p_Itinerary = NULL;
	p_Edit = NULL;
	m_HeaderItemClicked = m_FocusItem.x = m_FocusItem.y = m_HotItem.x = m_HotItem.y = m_HotSubitem = m_SelectionAnchor = -1;
	m_Hover = m_IgnoreHeaderItemChange = FALSE;
	m_ViewParameters = theApp.m_ViewParameters;

	m_FindReplaceSettings = theApp.m_FindReplaceSettings;
	m_FindReplaceSettings.FirstAction = FALSE;
}

CDataGrid::~CDataGrid()
{
	DestroyEdit();
}

BOOL CDataGrid::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

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

				for (INT col=m_FocusItem.x+1; col<FMAttributeCount; col++)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					{
						SetFocusItem(CPoint(col, m_FocusItem.y), FALSE);
						break;
					}

				return TRUE;

			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;

			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				if (!m_EditAllowCursor)
				{
					DestroyEdit(TRUE);
					PostMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
					return TRUE;
				}
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

void CDataGrid::SetItinerary(CItinerary* pItinerary, UINT Row)
{
	if (p_Itinerary!=pItinerary)
	{
		p_Itinerary = pItinerary;

		m_FocusItem.x = p_Itinerary ? 0 : -1;
		m_FocusItem.y = p_Itinerary ? Row : -1;
		m_SelectionAnchor = -1;
		AdjustLayout();
		EnsureVisible();
	}
}

BOOL CDataGrid::HasSelection(BOOL CurrentLineIfNoneSelected)
{
	if (!p_Itinerary)
		return FALSE;

	if (CurrentLineIfNoneSelected && (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
		return TRUE;

	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
		if (p_Itinerary->m_Flights.m_Items[a].Flags & AIRX_Selected)
			return TRUE;

	return FALSE;
}

BOOL CDataGrid::IsSelected(UINT Index)
{
	ASSERT(p_Itinerary);

	return (Index<p_Itinerary->m_Flights.m_ItemCount) ? (p_Itinerary->m_Flights.m_Items[Index].Flags & AIRX_Selected ) : FALSE;
}

UINT CDataGrid::GetCurrentRow()
{
	if (p_Itinerary)
		if (m_FocusItem.y!=-1)
			return (UINT)m_FocusItem.y;

	return 0;
}

void CDataGrid::DoCopy(BOOL Cut)
{
	if (OpenClipboard())
	{
		UINT Count = 0;

		// Text erstellen
		CString MemBitmap;
		if (HasSelection())
		{
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				if (IsSelected(a))
				{
					MemBitmap += p_Itinerary->Flight2Text(a);
					Count++;
				}
		}
		else
			if (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				WCHAR tmpBuf[256];
				AttributeToString(p_Itinerary->m_Flights.m_Items[m_FocusItem.y], m_ViewParameters.ColumnOrder[m_FocusItem.x], tmpBuf, 256);

				MemBitmap = tmpBuf;
			}

		EmptyClipboard();

		// CF_UNICODETEXT
		SIZE_T sz = (MemBitmap.GetLength()+1)*sizeof(WCHAR);
		HGLOBAL ClipBuffer = GlobalAlloc(GMEM_DDESHARE, sz);
		LPVOID pBuffer = GlobalLock(ClipBuffer);
		wcscpy_s((WCHAR*)pBuffer, sz, MemBitmap.GetBuffer());
		GlobalUnlock(ClipBuffer);
		SetClipboardData(CF_UNICODETEXT, ClipBuffer);

		// CF_FLIGHTS
		if (Count)
		{
			sz = Count*sizeof(AIRX_Flight);
			ClipBuffer = GlobalAlloc(GMEM_DDESHARE, sz);
			pBuffer = GlobalLock(ClipBuffer);

			AIRX_Flight* pFlight = (AIRX_Flight*)pBuffer;
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				if (IsSelected(a))
				{
					*pFlight = p_Itinerary->m_Flights.m_Items[a];
					pFlight++;
				}

			GlobalUnlock(ClipBuffer);
			SetClipboardData(theApp.CF_FLIGHTS, ClipBuffer);
		}

		CloseClipboard();

		if (Cut)
			DoDelete();
	}
}

void CDataGrid::DoDelete()
{
	if (HasSelection())
	{
		p_Itinerary->DeleteSelectedFlights();

		m_SelectionAnchor = -1;
		AdjustLayout();
	}
	else
	{
		FinishEdit(L"", m_FocusItem);
	}
}

void CDataGrid::AdjustLayout()
{
	if (p_Itinerary)
		if (m_FocusItem.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
			m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount;

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
				{
					m_ViewParameters.ColumnWidth[a] = theApp.m_ViewParameters.ColumnWidth[a] = HdItem.cxy = MINWIDTH;
				}

		m_wndHeader.SetItem(a, &HdItem);
	}

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	m_IgnoreHeaderItemChange = FALSE;
}

void CDataGrid::EditCell(BOOL AllowCursor, BOOL Delete, WCHAR PushChar, CPoint Item)
{
	if (!p_Itinerary)
		return;

	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;
	if ((Item.x==-1) || (Item.y==-1) || (Item.x>=FMAttributeCount) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	if (!FMAttributes[Attr].Editable)
		return;

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);
	const LPVOID pData = NewLine ? NULL : (((BYTE*)&p_Itinerary->m_Flights.m_Items[Item.y])+FMAttributes[Attr].Offset);
	Delete |= NewLine;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeColor:
		if (!PushChar)
		{
			COLORREF col = pData ? *((COLORREF*)pData) : (COLORREF)-1;
			if (theApp.ChooseColor(&col, this))
			{
				if (NewLine)
				{
					p_Itinerary->AddFlight();
					Item.y = p_Itinerary->m_Flights.m_ItemCount-1;

					AdjustLayout();
				}

				*((COLORREF*)(((BYTE*)&p_Itinerary->m_Flights.m_Items[Item.y])+FMAttributes[Attr].Offset)) = col;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(Item);
			}
		}

		return;

	case FMTypeFlags:
		if (pData)
			switch (PushChar)
			{
			case L'A':
			case L'a':
				*((DWORD*)pData) ^= AIRX_AwardFlight;
				break;

			case L'G':
			case L'g':
				*((DWORD*)pData) ^= AIRX_GroundTransportation;
				break;

			case L'B':
			case L'b':
				*((DWORD*)pData) ^= AIRX_BusinessTrip;
				break;

			case L'L':
			case L'l':
				*((DWORD*)pData) ^= AIRX_LeisureTrip;
				break;

			case L'U':
			case L'u':
				*((DWORD*)pData) ^= AIRX_Upgrade;
				break;

			case L'C':
			case L'c':
				*((DWORD*)pData) ^= AIRX_Cancelled;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateRow(Item.y);

			default:
				return;
			}

		p_Itinerary->m_IsModified = TRUE;
		InvalidateItem(Item);

		return;

	case FMTypeRating:
		if (pData && (PushChar>=L'0') && (PushChar<=L'5'))
		{
			*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
			*((DWORD*)pData) |= (((PushChar-'0')*2)<<FMAttributes[Attr].DataParameter);

			p_Itinerary->m_IsModified = TRUE;
			InvalidateItem(Item);
		}

		return;
	}

	INT y = Item.y*m_RowHeight+m_HeaderHeight-m_VScrollPos;
	INT x = -m_HScrollPos;
	for (INT a=0; a<Item.x; a++)
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	WCHAR tmpBuf[256] = L"";
	if (PushChar)
	{
		tmpBuf[0] = PushChar;
		tmpBuf[1] = L'\0';
	}
	else
		if (!Delete)
		{
			AttributeToString(p_Itinerary->m_Flights.m_Items[Item.y], Attr, tmpBuf, 256);
		}

	CRect rect(x-2, y-2, x+m_ViewParameters.ColumnWidth[Attr]+1, y+m_RowHeight+1);

	p_Edit = new CMFCMaskedEdit();
	p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);

	PrepareEditCtrl(p_Edit, Attr);
	m_EditAllowCursor = AllowCursor;

	p_Edit->SetWindowText(tmpBuf);
	p_Edit->SetFont(&theApp.m_DefaultFont);
	p_Edit->SetFocus();
	p_Edit->SetSel(PushChar ? -1 : 0, PushChar ? 0 : -1);
}

void CDataGrid::EditFlight(CPoint Item, INT iSelectPage)
{
	if (!p_Itinerary)
		return;

	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;
	if ((Item.x==-1) || (Item.y==-1) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);

	EditFlightDlg dlg(NewLine ? NULL : &p_Itinerary->m_Flights.m_Items[Item.y], this, p_Itinerary, iSelectPage);
	if (dlg.DoModal()==IDOK)
	{
		if (NewLine)
		{
			p_Itinerary->AddFlight();
			Item.y = p_Itinerary->m_Flights.m_ItemCount-1;

			AdjustLayout();
		}
		else
		{
			Invalidate();
		}

		p_Itinerary->m_Flights.m_Items[Item.y] = dlg.m_Flight;
		p_Itinerary->m_IsModified = TRUE;
	}
	else
		if (NewLine)
		{
			p_Itinerary->DeleteAttachments(&dlg.m_Flight);
		}
		else
		{
			p_Itinerary->m_Flights.m_Items[Item.y].AttachmentCount = dlg.m_Flight.AttachmentCount;
			memcpy_s(p_Itinerary->m_Flights.m_Items[Item.y].Attachments, AIRX_MaxAttachmentCount*sizeof(UINT), dlg.m_Flight.Attachments, AIRX_MaxAttachmentCount*sizeof(UINT));
		}
}

void CDataGrid::EnsureVisible(CPoint Item)
{
	if (!p_Itinerary)
		return;

	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;
	if ((Item.x==-1) || (Item.y==-1) || (Item.x>=FMAttributeCount) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if ((INT)((Item.y+1)*m_RowHeight)>m_VScrollPos+rect.Height()-(INT)m_HeaderHeight)
		nInc = (Item.y+1)*m_RowHeight-rect.Height()+(INT)m_HeaderHeight-m_VScrollPos;
	if ((INT)(Item.y*m_RowHeight)<m_VScrollPos+nInc)
		nInc = Item.y*m_RowHeight-m_VScrollPos;

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
	for (INT a=0; a<Item.x; a++)
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	nInc = 0;
	if (x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]>m_HScrollPos+rect.Width())
		nInc = x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]-rect.Width()-m_HScrollPos;
	if (x<m_HScrollPos+nInc)
		nInc = x-m_HScrollPos;

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

BOOL CDataGrid::HitTest(CPoint point, CPoint* Item, INT* pSubitem)
{
	ASSERT(Item);

	if (!p_Itinerary)
		return FALSE;

	point.y -= m_HeaderHeight-m_VScrollPos;

	INT row = (point.y>=0) ? point.y/m_RowHeight : -1;
	if ((row!=-1) && (row<=(INT)p_Itinerary->m_Flights.m_ItemCount))
	{
		INT x = -m_HScrollPos;

		for (UINT a=0; a<FMAttributeCount; a++)
		{
			const UINT Attr = m_ViewParameters.ColumnOrder[a];
			if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[Attr]))
			{
				Item->x = a;
				Item->y = row;

				if (pSubitem)
				{
					*pSubitem = -1;

					if (row<(INT)p_Itinerary->m_Flights.m_ItemCount)
					{
						x = point.x-x-MARGIN;

						if (x>0)
							switch (FMAttributes[Attr].Type)
							{
							case FMTypeFlags:
								if (x<18*FLAGCOUNT)
									if (x%18<16)
										*pSubitem = x/18;

								break;

							case FMTypeRating:
								if (x<RatingBitmapWidth+6)
									if ((x<6) || (x%18<16))
										*pSubitem = (x<6) ? 0 : 2*(x/18)+(x%18>8)+1;

								break;
							}
					}
				}

				return TRUE;
			}

			x += m_ViewParameters.ColumnWidth[Attr];
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

		CRect rect(x-2, m_HeaderHeight+Item.y*m_RowHeight-m_VScrollPos-2, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]+1, m_HeaderHeight-m_VScrollPos+(Item.y+1)*m_RowHeight+1);
		InvalidateRect(rect);
	}
}

void CDataGrid::InvalidateItem(UINT Row, UINT Attr)
{
	for (UINT a=0; a<FMAttributeCount; a++)
		if (m_ViewParameters.ColumnOrder[a]==(INT)Attr)
			InvalidateItem(CPoint(a, Row));
}

void CDataGrid::InvalidateRow(UINT Row)
{
	CRect rectClient;
	GetClientRect(rectClient);

	CRect rect(rectClient.left, m_HeaderHeight+Row*m_RowHeight-m_VScrollPos, rectClient.right, m_HeaderHeight-m_VScrollPos+(Row+1)*m_RowHeight);
	InvalidateRect(rect);
}

void CDataGrid::SetFocusItem(CPoint FocusItem, BOOL ShiftSelect)
{
	ASSERT(p_Itinerary);

	if (FocusItem==m_FocusItem)
		return;

	if (ShiftSelect)
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_FocusItem.y;

		for (INT a=0; a<(INT)p_Itinerary->m_Flights.m_ItemCount; a++)
			SelectItem(a, ((a>=FocusItem.y) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=FocusItem.y)));
	}
	else
	{
		m_SelectionAnchor = -1;

		for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
			SelectItem(a, FALSE);
	}

	InvalidateItem(m_FocusItem);
	m_FocusItem = FocusItem;
	EnsureVisible();
	InvalidateItem(m_FocusItem);
}

void CDataGrid::SelectItem(UINT Index, BOOL Select, BOOL InternalCall)
{
	ASSERT(p_Itinerary);
	
	if (Index<p_Itinerary->m_Flights.m_ItemCount)
		if (Select!=((p_Itinerary->m_Flights.m_Items[Index].Flags & AIRX_Selected)!=0))
		{
			if (Select)
			{
				p_Itinerary->m_Flights.m_Items[Index].Flags |= AIRX_Selected;
			}
			else
			{
				p_Itinerary->m_Flights.m_Items[Index].Flags &= ~AIRX_Selected;
			}

			if (!InternalCall)
				InvalidateRow(Index);
		}
}

void CDataGrid::DrawCell(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect rect, BOOL Selected)
{
	ASSERT(Attr<FMAttributeCount);

	// Background
	if (!Selected)
		if (FMAttributes[Attr].Type==FMTypeClass)
			switch (Flight.Class)
			{
			case AIRX_Economy:
			case AIRX_PremiumEconomy:
			case AIRX_Charter:
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

	case FMTypeFlags:
		{
			HDC hdcMem = CreateCompatibleDC(dc);
			CPoint pt(rect.left, rect.top+(rect.Height()-16)/2);

			for (UINT a=0; a<FLAGCOUNT; a++)
			{
				HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, theApp.m_FlagIcons16[a ? Flight.Flags & DisplayFlags[a] ? 1 : 0 : Flight.AttachmentCount>0 ? 1 : 0]);
				AlphaBlend(dc, pt.x, pt.y, 16, 16, hdcMem, a*16, 0, 16, 16, BF);
				SelectObject(hdcMem, hOldBitmap);

				pt.x += 18;
			}

			DeleteDC(hdcMem);
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
			CSize szText = dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr));

			Width = max(Width, szText.cx);
		}

		dc.SelectObject(pOldFont);
	}

	Width += 2*MARGIN+1;
	m_ViewParameters.ColumnWidth[Attr] = theApp.m_ViewParameters.ColumnWidth[Attr] = min(Width, MAXWIDTH);
}

void CDataGrid::FinishEdit(WCHAR* pStr, CPoint Item)
{
	ASSERT(p_Itinerary);

	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	StringToAttribute(pStr, p_Itinerary->m_Flights.m_Items[Item.y], Attr);

	p_Itinerary->m_IsModified = TRUE;
	InvalidateItem(Item);

	switch (Attr)
	{
	case 0:
	case 3:
		CalcDistance(p_Itinerary->m_Flights.m_Items[Item.y], TRUE);
		InvalidateItem(Item.y, 6);
		break;

	case 1:
		CalcFuture(p_Itinerary->m_Flights.m_Items[Item.y]);

	case 20:
		InvalidateRow(Item.y);
		break;
	}
}

void CDataGrid::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		CPoint Item = m_FocusItem;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		WCHAR tmpBuf[256];
		victim->GetWindowText(tmpBuf, 256);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (p_Itinerary) && (Item.x!=-1) && (Item.y!=-1))
		{
			if (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				if (tmpBuf[0]==L'\0')
					return;

				p_Itinerary->AddFlight();
				Item.y = p_Itinerary->m_Flights.m_ItemCount-1;

				AdjustLayout();
			}

			FinishEdit(tmpBuf, Item);
		}
	}
}

void CDataGrid::FindReplace(INT iSelectPage)
{
	FindReplaceDlg dlg(iSelectPage, m_ViewParameters.ColumnOrder[m_FocusItem.x], this);
	if (dlg.DoModal()==IDOK)
	{
		theApp.m_FindReplaceSettings = m_FindReplaceSettings = dlg.m_FindReplaceSettings;

		// Something to search?
		if (m_FindReplaceSettings.SearchTerm[0]!=L'\0')
		{
			// Append search term to "recent" list
			for (POSITION p=theApp.m_RecentSearchTerms.GetHeadPosition(); p; )
			{
				POSITION pl = p;
				if (theApp.m_RecentSearchTerms.GetNext(p)==m_FindReplaceSettings.SearchTerm)
					theApp.m_RecentSearchTerms.RemoveAt(pl);
			}

			theApp.m_RecentSearchTerms.AddHead(m_FindReplaceSettings.SearchTerm);

			if (m_FindReplaceSettings.DoReplace && (m_FindReplaceSettings.ReplaceTerm[0]!=L'\0'))
			{
				// Append replace term to "recent" list
				for (POSITION p=theApp.m_RecentReplaceTerms.GetHeadPosition(); p; )
				{
					POSITION pl = p;
					if (theApp.m_RecentReplaceTerms.GetNext(p)==m_FindReplaceSettings.ReplaceTerm)
						theApp.m_RecentReplaceTerms.RemoveAt(pl);
				}

				theApp.m_RecentReplaceTerms.AddHead(m_FindReplaceSettings.ReplaceTerm);
			}

			OnFindReplaceAgain();
		}
	}
}


BEGIN_MESSAGE_MAP(CDataGrid, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
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
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	ON_COMMAND(IDM_EDIT_CUT, OnCut)
	ON_COMMAND(IDM_EDIT_COPY, OnCopy)
	ON_COMMAND(IDM_EDIT_PASTE, OnPaste)
	ON_COMMAND(IDM_EDIT_INSERTROW, OnInsertRow)
	ON_COMMAND(IDM_EDIT_DELETE, OnDelete)
	ON_COMMAND(IDM_EDIT_EDITFLIGHT, OnEditFlight)
	ON_COMMAND(IDM_EDIT_ADDROUTE, OnAddRoute)
	ON_COMMAND(IDM_EDIT_FIND, OnFind)
	ON_COMMAND(IDM_EDIT_REPLACE, OnReplace)
	ON_COMMAND(IDM_EDIT_FINDREPLACEAGAIN, OnFindReplaceAgain)
	ON_COMMAND(IDM_EDIT_FILTER, OnFilter)
	ON_COMMAND(IDM_EDIT_SELECTALL, OnSelectAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_EDIT_CUT, IDM_EDIT_SELECTALL, OnUpdateEditCommands)

	ON_COMMAND(IDM_DETAILS_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_DETAILS_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_DETAILS_CHOOSE, OnChooseDetails)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DETAILS_AUTOSIZEALL, IDM_DETAILS_CHOOSE, OnUpdateDetailsCommands)

	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)
	ON_NOTIFY(HDN_ENDDRAG, 1, OnEndDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
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
		CString tmpStr((LPCSTR)FMAttributes[a].nNameID);

		HDITEM HdItem;
		HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
		HdItem.fmt = HDF_STRING | HDF_CENTER;
		HdItem.cxy = m_ViewParameters.ColumnWidth[a];
		HdItem.pszText = tmpStr.GetBuffer();
		m_wndHeader.InsertItem(a, &HdItem);
	}

	VERIFY(m_wndHeader.SetOrderArray(FMAttributeCount, m_ViewParameters.ColumnOrder));
	m_IgnoreHeaderItemChange = FALSE;

	SYSTEMTIME st;
	GetLocalTime(&st);
	m_wDay = st.wDay;

	SetTimer(1, 1000, NULL);

	m_TooltipCtrl.Create(this);

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (2*(MARGIN-1)+max(abs(lf.lfHeight), 16)) & ~1;

	ResetScrollbars();

	return 0;
}

void CDataGrid::OnDestroy()
{
	KillTimer(1);

	CWnd::OnDestroy();
}

void CDataGrid::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		if (st.wDay!=m_wDay)
		{
			m_wDay = st.wDay;

			if (p_Itinerary)
			{
				for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
					CalcFuture(p_Itinerary->m_Flights.m_Items[a], &st);

				Invalidate();
			}
		}
	}

	CWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();
	const COLORREF colBackground = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	const COLORREF colLines = Themed ? theApp.OSVersion==OS_Eight ? 0xEAE9E8 : 0xDDDCDA : GetSysColor(COLOR_SCROLLBAR);
	const COLORREF colText = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);
	const COLORREF colDisabled = Themed ? 0xA0A0A0 : GetSysColor(COLOR_3DFACE);
	dc.FillSolidRect(rect, colBackground);

	if (p_Itinerary)
	{
		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		INT start = m_VScrollPos/m_RowHeight;
		INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
		for (UINT row=start; row<=p_Itinerary->m_Flights.m_ItemCount; row++)
		{
			const BOOL Selected = (row<p_Itinerary->m_Flights.m_ItemCount) && (p_Itinerary->m_Flights.m_Items[row].Flags & AIRX_Selected);

			INT x = -m_HScrollPos;
			for (UINT col=0; col<FMAttributeCount; col++)
			{
				const UINT Attr = m_ViewParameters.ColumnOrder[col];
				const INT l = m_ViewParameters.ColumnWidth[Attr];
				if (l)
				{
					CRect rectItem(x, y, x+l-1, y+m_RowHeight-1);
					CRect rectIntersect;
					if (rectIntersect.IntersectRect(rectItem, rectUpdate))
					{
						if (Selected)
						{
							dc.FillSolidRect(rectItem, Themed && (theApp.OSVersion!=OS_XP) && (theApp.OSVersion!=OS_Eight) ? 0xFFDBB7 : GetSysColor(COLOR_HIGHLIGHT));
						}
						else
							if (!FMAttributes[Attr].Editable)
							{
								dc.FillSolidRect(rectItem, theApp.OSVersion==OS_Eight ? 0xF7F6F5 : 0xF5F5F5);
							}

						if (row<p_Itinerary->m_Flights.m_ItemCount)
						{
							const DWORD Flags = p_Itinerary->m_Flights.m_Items[row].Flags;
							dc.SetTextColor((Selected && (!Themed || (theApp.OSVersion==OS_XP) || (theApp.OSVersion==OS_Eight))) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : ((Attr==0) && (Flags & AIRX_UnknownFrom)) || ((Attr==3) && (Flags & AIRX_UnknownTo)) ? 0x0000FF : (Flags & AIRX_Cancelled) ? colDisabled : (Flags & AIRX_FutureFlight) ? 0x008000 : colText);
							DrawCell(dc, p_Itinerary->m_Flights.m_Items[row], Attr, rectItem, Selected);
						}
					}

					x += l;
					dc.FillSolidRect(x-1, y, 1, m_RowHeight-1, colLines);
				}
			}

			y += m_RowHeight;
			dc.FillSolidRect(0, y-1, x, 1, colLines);

			if (y>rect.Height())
				break;
			}

		if ((m_FocusItem.x!=-1) && (m_FocusItem.y!=-1))
		{
			INT x = -m_HScrollPos;
			for (INT col=0; col<m_FocusItem.x; col++)
				x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]];

			CRect rect(x-2, m_FocusItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos-2, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[m_FocusItem.x]]+1, m_FocusItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos+m_RowHeight+1);

			for (UINT a=0; a<3; a++)
			{
				dc.Draw3dRect(rect, colText, colText);
				rect.DeflateRect(1, 1);
			}
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

void CDataGrid::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	CPoint Item(-1, -1);
	INT Subitem = -1;
	BOOL OnItem = HitTest(point, &Item, &Subitem);

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
		if ((m_TooltipCtrl.IsWindowVisible()) && ((Item!=m_HotItem) || (Subitem!=m_HotSubitem)))
			m_TooltipCtrl.Deactivate();

	if (!OnItem)
	{
		m_HotItem.x = m_HotItem.y = m_HotSubitem = -1;
	}
	else
		if ((Item!=m_HotItem) || (Subitem!=m_HotSubitem))
		{
			InvalidateItem(m_HotItem);
			m_HotItem = Item;
			m_HotSubitem = Subitem;
			InvalidateItem(m_HotItem);

			SetCursor(theApp.LoadStandardCursor(m_HotSubitem==-1 ? IDC_ARROW : IDC_HAND));
		}
}

void CDataGrid::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem.x = m_HotItem.y = m_HotSubitem = -1;
}

void CDataGrid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem.x!=-1) && (m_HotItem.y!=-1) && (p_Itinerary) && (!p_Edit))
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
					switch (FMAttributes[Attr].Type)
					{
					case FMTypeFlags:
						if (m_HotSubitem!=-1)
						{
							CString Caption;
							CString Message((LPCSTR)IDS_ATTACHMENTS+m_HotSubitem);

							INT Pos = Message.Find(L'\n');
							if (Pos!=-1)
							{
								Caption = Message.Left(Pos);
								Message = Message.Mid(Pos+1);
							}

							m_TooltipCtrl.Track(point, theApp.m_FlagIcons32.ExtractIcon(m_HotSubitem), NULL, Caption, Message);
						}

					case FMTypeColor:
						break;

					case FMTypeUINT:
					case FMTypeDistance:
					case FMTypeDateTime:
					case FMTypeTime:
						if (IsSelected(m_HotItem.y))
						{
							UINT U;
							UINT UMin = 0xFFFFFFFF;
							UINT UMax = 0;
							ULONGLONG USum = 0;
							FILETIME F;
							FILETIME FMin = { 0xFFFFFFFF, 0xFFFFFFFF };
							FILETIME FMax = { 0, 0 };
							DOUBLE D;
							DOUBLE DMin = 100000.0;
							DOUBLE DMax = 0.0;
							DOUBLE DSum = 0.0;
							UINT Count = 0;

							for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
								if (IsSelected(a))
								{
									const LPVOID pData = (((BYTE*)&p_Itinerary->m_Flights.m_Items[a])+FMAttributes[Attr].Offset);

									switch (FMAttributes[Attr].Type)
									{
									case FMTypeUINT:
									case FMTypeTime:
										U = *((UINT*)pData);
										if (U)
										{
											if (U<UMin)
												UMin = U;
											if (U>UMax)
												UMax = U;
											USum += U;

											Count++;
										}

										break;

									case FMTypeDistance:
										D = *((DOUBLE*)pData);
										if (D)
										{
											if (D<DMin)
												DMin = D;
											if (D>DMax)
												DMax = D;
											DSum += D;

											Count++;
										}

										break;

									case FMTypeDateTime:
										F = *((FILETIME*)pData);
										if ((F.dwHighDateTime!=0) || (F.dwLowDateTime!=0))
										{
											if ((F.dwHighDateTime<FMin.dwHighDateTime) || ((F.dwHighDateTime==FMin.dwHighDateTime) && (F.dwLowDateTime<FMin.dwLowDateTime)))
												FMin = F;
											if ((F.dwHighDateTime>FMax.dwHighDateTime) || ((F.dwHighDateTime==FMax.dwHighDateTime) && (F.dwLowDateTime>FMax.dwLowDateTime)))
												FMax = F;

											Count++;
										}

										break;
									}
								}

							if (Count)
							{
								CString msg;
								WCHAR tmpMin[256];
								WCHAR tmpMax[256];
								WCHAR tmpAvg[256];

								switch (FMAttributes[Attr].Type)
								{
								case FMTypeUINT:
									msg.Format(IDS_TOOLTIP_UINT, UMin, UMax, (DOUBLE)USum/(DOUBLE)Count);
									break;

								case FMTypeTime:
									TimeToString(tmpMin, 256, UMin);
									TimeToString(tmpMax, 256, UMax);
									TimeToString(tmpAvg, 256, (UINT)((DOUBLE)USum/(DOUBLE)Count));

									msg.Format(IDS_TOOLTIP_TIME, tmpMin, tmpMax, tmpAvg);
									break;

								case FMTypeDistance:
									DistanceToString(tmpMin, 256, DMin);
									DistanceToString(tmpMax, 256, DMax);
									DistanceToString(tmpAvg, 256, DSum/(DOUBLE)Count);

									msg.Format(IDS_TOOLTIP_DISTANCE, tmpMin, tmpMax, tmpAvg);
									break;

								case FMTypeDateTime:
									DateTimeToString(tmpMin, 256, FMin);
									DateTimeToString(tmpMax, 256, FMax);

									msg.Format(IDS_TOOLTIP_DATETIME, tmpMin, tmpMax);
									break;
								}

								CString cpt((LPCSTR)IDS_COLUMN0+Attr);

								m_TooltipCtrl.Track(point, NULL, NULL, cpt, msg);
							}
							break;
						}
					default:
						AttributeToString(p_Itinerary->m_Flights.m_Items[m_HotItem.y], Attr, tmpStr, 256);

						if (tmpStr[0]!=L'\0')
						{
							CClientDC dc(this);

							CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);
							CSize szText = dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr));
							dc.SelectObject(pOldFont);

							if ((szText.cx>m_ViewParameters.ColumnWidth[Attr]-2*MARGIN-1) || (FMAttributes[Attr].Type==FMTypeColor))
								m_TooltipCtrl.Track(point, NULL, NULL, _T(""), tmpStr);
						}
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

BOOL CDataGrid::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
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

void CDataGrid::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
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
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
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

		CPoint Item(m_FocusItem);

		switch (nChar)
		{
		case VK_F2:
			EditCell(TRUE);
			return;

		case VK_BACK:
			if (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				EditCell(FALSE, TRUE);

			return;

		case VK_DELETE:
			if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			{
				if (HasSelection(TRUE))
					OnDelete();
			}
			else
				if (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				{
					FinishEdit(L"", m_FocusItem);
				}

			return;

		case VK_LEFT:
			for (INT col=Item.x-1; col>=0; col--)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
				{
					Item.x = col;
					break;
				}

			break;

		case VK_RIGHT:
		case VK_EXECUTE:
		case VK_RETURN:
			for (INT col=Item.x+1; col<FMAttributeCount; col++)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
				{
					Item.x = col;
					break;
				}

			break;

		case VK_UP:
			if (Item.y>0)
				Item.y--;

			break;

		case VK_PRIOR:
			Item.y -= (rect.Height()-m_HeaderHeight)/(INT)m_RowHeight;
			if (Item.y<0)
				Item.y = 0;

			break;

		case VK_DOWN:
			if (Item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				Item.y++;

			break;

		case VK_NEXT:
			Item.y += (rect.Height()-m_HeaderHeight)/(INT)m_RowHeight;
			if (Item.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
				Item.y = p_Itinerary->m_Flights.m_ItemCount;

			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				Item.y = 0;
			}
			else
			{
				for (INT col=0; col<FMAttributeCount; col++)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					{
						Item.x = col;
						break;
					}
			}

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				Item.y = p_Itinerary->m_Flights.m_ItemCount;
			}
			else
			{
				for (INT col=FMAttributeCount-1; col>=0; col--)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					{
						Item.x = col;
						break;
					}
			}

			break;

		case 'A':
			if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
				OnSelectAll();

			break;

		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			return;
		}

		SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);
	}
}

void CDataGrid::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (p_Itinerary && !p_Edit)
		if (nChar>=L' ')
			EditCell(FALSE, FALSE, (WCHAR)nChar);
}

void CDataGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	DestroyEdit();

	CPoint Item;
	INT Subitem = -1;
	if (HitTest(point, &Item, &Subitem))
	{
		if (nFlags & MK_CONTROL)
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Item;
			EnsureVisible();
			InvalidateItem(m_FocusItem);

			SelectItem(Item.y, !IsSelected(Item.y));
		}
		else
			if (p_Itinerary && (Item==m_FocusItem))
			{
				if (Item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				{
					const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
					const LPVOID pData = (((BYTE*)&p_Itinerary->m_Flights.m_Items[Item.y])+FMAttributes[Attr].Offset);

					switch (FMAttributes[Attr].Type)
					{
					case FMTypeFlags:
						if (Subitem==0)
						{
							EditFlight(Item, 2);
						}
						else
							if (Subitem!=-1)
							{
								*((DWORD*)pData) ^= DisplayFlags[Subitem];
								p_Itinerary->m_IsModified = TRUE;

								if (DisplayFlags[Subitem]==AIRX_Cancelled)
								{
									InvalidateRow(Item.y);
								}
								else
								{
									InvalidateItem(Item);
								}
							}

						break;

					case FMTypeRating:
						if (Subitem!=-1)
						{
							*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
							*((DWORD*)pData) |= (Subitem<<FMAttributes[Attr].DataParameter);

							p_Itinerary->m_IsModified = TRUE;
							InvalidateItem(Item);
						}

						break;

					default:
						EditCell();
					}
				}
			}
			else
			{
				SetFocusItem(Item, nFlags & MK_SHIFT);
			}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CDataGrid::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPoint Item;
	if (HitTest(point, &Item))
	{
		if (GetFocus()!=this)
			SetFocus();
	}
	else
		if (!(nFlags & MK_CONTROL) && p_Itinerary)
		{
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				SelectItem(a, FALSE);
		}
}

void CDataGrid::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	CPoint Item;
	if (HitTest(point, &Item))
		EditFlight(Item);
}

void CDataGrid::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint Item;
	if (HitTest(point, &Item))
	{
		if (!(nFlags & (MK_SHIFT | MK_CONTROL)))
			if (!IsSelected(Item.y))
				for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
					SelectItem(a, FALSE);

		InvalidateItem(m_FocusItem);
		m_FocusItem = Item;
		EnsureVisible();
		InvalidateItem(m_FocusItem);
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CDataGrid::OnRButtonUp(UINT nFlags, CPoint point)
{
	CPoint Item;
	if (HitTest(point, &Item))
	{
		if (GetFocus()!=this)
			SetFocus();

		if (!IsSelected(Item.y))
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Item;
			EnsureVisible();
			InvalidateItem(m_FocusItem);

			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				SelectItem(a, FALSE);
		}
	}
	else
	{
		if (!(nFlags & MK_CONTROL))
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				SelectItem(a, FALSE);
	}

	GetParent()->UpdateWindow();
	CWnd::OnRButtonUp(nFlags, point);
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
		pPopup->Create((CMainWindow*)GetTopLevelParent(), IDB_MENUDETAILS_32, IDB_MENUDETAILS_16);
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZEALL);
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_DETAILS_CHOOSE, 0);

		pPopup->Track(point);
		return;
	}

	if ((point.x<0) || (point.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		point.x = (rect.left+rect.right)/2;
		point.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&point);
	}

	CDialogMenuPopup* pPopup = (CDialogMenuPopup*)GetParent()->SendMessage(WM_REQUESTSUBMENU, IDM_EDIT);

	pPopup->Track(point);
}

BOOL CDataGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(theApp.LoadStandardCursor(m_HotSubitem==-1 ? IDC_ARROW : IDC_HAND));

	return TRUE;
}

void CDataGrid::OnSetFocus(CWnd* /*pOldWnd*/)
{
	InvalidateItem(m_FocusItem);
}

void CDataGrid::OnKillFocus(CWnd* /*pNewWnd*/)
{
	InvalidateItem(m_FocusItem);
}


// Edit

void CDataGrid::OnCut()
{
	ASSERT(p_Itinerary);
	ASSERT(HasSelection(TRUE));

	DoCopy(TRUE);
}

void CDataGrid::OnCopy()
{
	ASSERT(p_Itinerary);
	ASSERT(HasSelection(TRUE));

	DoCopy(FALSE);
}

void CDataGrid::OnPaste()
{
	ASSERT(p_Itinerary);

	if (OpenClipboard())
	{
		// CF_FLIGHTS
		HGLOBAL ClipBuffer = GetClipboardData(theApp.CF_FLIGHTS);
		if (ClipBuffer)
		{
			LPVOID pBuffer = GlobalLock(ClipBuffer);

			p_Itinerary->InsertFlights(m_FocusItem.y, (UINT)(GlobalSize(ClipBuffer)/sizeof(AIRX_Flight)), (AIRX_Flight*)pBuffer);
			p_Itinerary->m_IsModified = TRUE;
			m_SelectionAnchor = -1;
			AdjustLayout();

			goto Finish;
		}

		// CF_UNICODETEXT
		ClipBuffer = GetClipboardData(CF_UNICODETEXT);
		if (ClipBuffer)
		{
			LPVOID pBuffer = GlobalLock(ClipBuffer);

			if (m_FocusItem.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				p_Itinerary->AddFlight();
				m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount-1;

				AdjustLayout();
			}

			FinishEdit((WCHAR*)pBuffer, m_FocusItem);
		}

Finish:
		GlobalUnlock(ClipBuffer);
		CloseClipboard();
	}
}

void CDataGrid::OnInsertRow()
{
	ASSERT(p_Itinerary);

	if (m_FocusItem.y!=-1)
	{
		p_Itinerary->InsertFlights(m_FocusItem.y);
		p_Itinerary->m_IsModified = TRUE;

		m_SelectionAnchor = -1;
		AdjustLayout();
	}
}

void CDataGrid::OnDelete()
{
	ASSERT(p_Itinerary);
	ASSERT(HasSelection(TRUE));

	DoDelete();
}

void CDataGrid::OnEditFlight()
{
	EditFlight();
}

void CDataGrid::OnAddRoute()
{
	ASSERT(p_Itinerary);

	AddRouteDlg dlg(p_Itinerary, this);
	if (dlg.DoModal()==IDOK)
	{
		CString From;
		CString To;
		CString resToken;
		INT Pos = 0;
		WCHAR DelimiterFound;
		BOOL Added = FALSE;

		while (Tokenize(dlg.m_Route, resToken, Pos, _T("-/,"), &DelimiterFound))
		{
			From = To;
			To = resToken;

			if ((!From.IsEmpty()) && (!To.IsEmpty()))
			{
				StringToAttribute(From.GetBuffer(), dlg.m_FlightTemplate, 0);
				StringToAttribute(To.GetBuffer(), dlg.m_FlightTemplate, 3);

				CalcDistance(dlg.m_FlightTemplate, TRUE);
				CalcFuture(dlg.m_FlightTemplate);

				p_Itinerary->m_Flights.AddItem(dlg.m_FlightTemplate);

				Added = TRUE;
			}

			if (DelimiterFound==L',')
				To.Empty();
		}

		if (Added)
		{
			AdjustLayout();
			SetFocusItem(CPoint(m_FocusItem.x, p_Itinerary->m_Flights.m_ItemCount-1), FALSE);
			p_Itinerary->m_IsModified = TRUE;
		}
	}
}

void CDataGrid::OnFind()
{
	FindReplace(0);
}

void CDataGrid::OnReplace()
{
	FindReplace(1);
}

void CDataGrid::OnFindReplaceAgain()
{
	ASSERT(m_FindReplaceSettings.SearchTerm[0]);

	#define ColumnValid(Attr) (FMAttributes[Attr].Searchable && (m_FindReplaceSettings.DoReplace ? FMAttributes[Attr].Editable : TRUE))
	#define ToUpper(Str) \
		{ WCHAR* pChar = Str; \
		  while (*pChar) \
			*(pChar++) = (WCHAR)toupper(*pChar); }

	// Prepare search term
	WCHAR SearchTerm[256];
	wcscpy_s(SearchTerm, 256, m_FindReplaceSettings.SearchTerm);

	if ((m_FindReplaceSettings.Flags & FRS_MATCHCASE)==0)
		ToUpper(SearchTerm);

	// Check for valid column
	if (!m_FindReplaceSettings.FirstAction && (m_FindReplaceSettings.Flags & FRS_MATCHCOLUMNONLY))
		if (!ColumnValid(m_ViewParameters.ColumnOrder[m_FocusItem.x]))
		{
			CString Caption((LPCSTR)IDS_FINDREPLACE);
			CString Message((LPCSTR)(m_FindReplaceSettings.DoReplace ? IDS_ILLEGALCOLUMN_REPLACE : IDS_ILLEGALCOLUMN_FIND));

			MessageBox(Message, Caption, MB_ICONERROR | MB_OK);

			return;
		}

	BOOL StartOver = FALSE;
	BOOL Replaced = FALSE;
	CPoint Item = m_FocusItem;
Again:
	// If not called from dialog, goto next cell
	if (!m_FindReplaceSettings.FirstAction)
	{
		if ((m_FindReplaceSettings.Flags & FRS_MATCHCOLUMNONLY)==0)
		{
			for (INT col=Item.x+1; col<FMAttributeCount; col++)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					if (ColumnValid(m_ViewParameters.ColumnOrder[col]))
					{
						Item.x = col;
						goto FoundPosition;
					}

			Item.x = 0;
		}

		Item.y++;
		if (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
		{
			Item.y = 0;

			if (StartOver)
			{
				CString Caption((LPCSTR)IDS_FINDREPLACE);
				CString Message((LPCSTR)((m_FindReplaceSettings.DoReplace && Replaced) ? IDS_ALLREPLACED : IDS_SEARCHTERMNOTFOUND));

				MessageBox(Message, Caption, (m_FindReplaceSettings.DoReplace && Replaced) ? MB_OK : MB_ICONEXCLAMATION | MB_OK);

				return;
			}

			StartOver = TRUE;
		}
	}
FoundPosition:
	m_FindReplaceSettings.FirstAction = FALSE;

	// Match
	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];

	WCHAR tmpStr[256];
	AttributeToString(p_Itinerary->m_Flights.m_Items[Item.y], Attr, tmpStr, 256);

	if ((m_FindReplaceSettings.Flags & FRS_MATCHCASE)==0)
		ToUpper(tmpStr);

	const BOOL Match = m_FindReplaceSettings.Flags & FRS_MATCHENTIRECELL ? wcscmp(SearchTerm, tmpStr)==0 : wcsstr(tmpStr, SearchTerm)!=NULL;
	if (!Match)
		goto Again;

	// Process
	if (m_FindReplaceSettings.DoReplace)
	{
		Replaced = TRUE;

		if ((m_FindReplaceSettings.Flags & FRS_REPLACEALL)==0)
		{
			SetFocusItem(Item, FALSE);

			CString Caption((LPCSTR)IDS_FINDREPLACE);
			CString Message((LPCSTR)IDS_REPLACEQUESTION);

			switch (MessageBox(Message, Caption, MB_ICONQUESTION | MB_YESNOCANCEL))
			{
			case IDNO:
				goto Again;

			case IDCANCEL:
				return;
			}
		}

		StringToAttribute(m_FindReplaceSettings.ReplaceTerm, p_Itinerary->m_Flights.m_Items[Item.y], Attr);
		p_Itinerary->m_IsModified = TRUE;

		InvalidateItem(Item);
		goto Again;
	}
	else
	{
		SetFocusItem(Item, FALSE);
	}
}

void CDataGrid::OnFilter()
{
	FilterDlg dlg(p_Itinerary, this);
	if (dlg.DoModal()==IDOK)
	{
		CItinerary* pItinerary = new CItinerary(p_Itinerary);

		for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
		{
			// Filter
			const AIRX_Flight* pFlight = &p_Itinerary->m_Flights.m_Items[a];

			if (strlen(dlg.m_Filter.Airport)==3)
				if ((strcmp(dlg.m_Filter.Airport, pFlight->From.Code)!=0) && (strcmp(dlg.m_Filter.Airport, pFlight->To.Code)!=0))
					continue;
			if (dlg.m_Filter.Carrier[0])
				if (wcscmp(dlg.m_Filter.Carrier, pFlight->Carrier)!=0)
					continue;
			if (dlg.m_Filter.Equipment[0])
				if (wcscmp(dlg.m_Filter.Equipment, pFlight->Equipment)!=0)
					continue;
			if (dlg.m_Filter.Business)
				if ((pFlight->Flags & AIRX_BusinessTrip)==0)
					continue;
			if (dlg.m_Filter.Leisure)
				if ((pFlight->Flags & AIRX_LeisureTrip)==0)
					continue;
			if (pFlight->Flags>>28<dlg.m_Filter.Rating)
				continue;

			if (dlg.m_Filter.DepartureMonth | dlg.m_Filter.DepartureYear)
			{
				SYSTEMTIME st;
				FileTimeToSystemTime(&pFlight->From.Time, &st);

				if (dlg.m_Filter.DepartureMonth)
					if (dlg.m_Filter.DepartureMonth!=st.wMonth)
						continue;
				if (dlg.m_Filter.DepartureYear)
					if (dlg.m_Filter.DepartureYear!=st.wYear)
						continue;
			}

			pItinerary->AddFlight(p_Itinerary, a);
		}

		if (dlg.m_Filter.SortBy>=0)
			pItinerary->Sort((UINT)dlg.m_Filter.SortBy, dlg.m_Filter.Descending);

		CMainWnd* pFrame = new CMainWnd();
		pFrame->Create(pItinerary);
		pFrame->ShowWindow(SW_SHOW);
		pFrame->UpdateWindow();
	}
}

void CDataGrid::OnSelectAll()
{
	ASSERT(p_Itinerary);

	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
		SelectItem(a, TRUE, TRUE);

	m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount;
	EnsureVisible();
	Invalidate();
}

void CDataGrid::OnUpdateEditCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_Itinerary!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_EDIT_CUT:
	case IDM_EDIT_COPY:
	case IDM_EDIT_DELETE:
		b = HasSelection(TRUE);
		break;

	case IDM_EDIT_PASTE:
		{
			COleDataObject dobj;
			if (dobj.AttachClipboard())
				b &= dobj.IsDataAvailable(theApp.CF_FLIGHTS) || dobj.IsDataAvailable(CF_UNICODETEXT);
		}

		break;

	case IDM_EDIT_FINDREPLACEAGAIN:
		b &= (m_FindReplaceSettings.SearchTerm[0]!=L'\0');

	case IDM_EDIT_FIND:
	case IDM_EDIT_REPLACE:
	case IDM_EDIT_FILTER:
	case IDM_EDIT_SELECTALL:
		if (p_Itinerary)
			b &= (p_Itinerary->m_Flights.m_ItemCount!=0);

		break;
	}

	pCmdUI->Enable(b);
}


// Details

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

		for (INT col=m_FocusItem.x; col>=0; col--)
			if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
			{
				m_FocusItem.x = col;
				break;
			}

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


// Header

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
		if (pHdr->pitem->iOrder==-1)
		{
			theApp.m_ViewParameters.ColumnWidth[pHdr->iItem] = m_ViewParameters.ColumnWidth[pHdr->iItem] = 0;
		}
		else
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
		}

		AdjustHeader();
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

void CDataGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
