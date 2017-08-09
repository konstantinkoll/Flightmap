
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

#define FLAGCOUNT          7
#define ITEMPADDING        2
#define LEFTMARGIN         (BACKSTAGEBORDER-1)
#define MINCOLUMNWIDTH     50
#define MAXAUTOWIDTH       400
#define SPACER             (4*ITEMPADDING+1)

static const UINT DisplayFlags[] = { 0, AIRX_AwardFlight, AIRX_GroundTransportation, AIRX_BusinessTrip, AIRX_LeisureTrip, AIRX_Upgrade, AIRX_Cancelled };

CIcons CDataGrid::m_LargeIcons;
CIcons CDataGrid::m_SmallIcons;

CDataGrid::CDataGrid()
	: CFrontstageWnd()
{
	p_Itinerary = NULL;
	p_Edit = NULL;
	m_HeaderItemClicked = m_HotItem.x = m_HotItem.y = m_HotSubitem = m_SelectionAnchor = -1;
	m_Hover = m_IgnoreHeaderItemChange = FALSE;
	m_ViewParameters = theApp.m_ViewParameters;

	m_FindReplaceSettings = theApp.m_FindReplaceSettings;
	m_FindReplaceSettings.FirstAction = FALSE;
}

CDataGrid::~CDataGrid()
{
	DestroyEdit();
}

BOOL CDataGrid::Create(CItinerary* pItinerary, CWnd* pParentWnd, UINT nID)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;
	m_FocusItem.x = 0;
	m_FocusItem.y = pItinerary->m_Metadata.CurrentRow;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
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

				for (INT Col=m_FocusItem.x+1; Col<FMAttributeCount; Col++)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
					{
						SetFocusItem(CPoint(Col, m_FocusItem.y), FALSE);
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
		FMGetApp()->HideTooltip();
		break;
	}

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

void CDataGrid::SetItinerary(CItinerary* pItinerary)
{
	if (p_Itinerary!=pItinerary)
	{
		p_Itinerary = pItinerary;

		m_FocusItem.x = 0;
		m_FocusItem.y = pItinerary->m_Metadata.CurrentRow;
		m_SelectionAnchor = -1;

		AdjustLayout();
		EnsureVisible();
	}
}

BOOL CDataGrid::HasSelection(BOOL CurrentLineIfNoneSelected) const
{
	if (CurrentLineIfNoneSelected && (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
		return TRUE;

	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
		if (p_Itinerary->m_Flights[a].Flags & AIRX_Selected)
			return TRUE;

	return FALSE;
}

BOOL CDataGrid::IsSelected(UINT Index) const
{
	return (Index<p_Itinerary->m_Flights.m_ItemCount) ? (p_Itinerary->m_Flights[Index].Flags & AIRX_Selected ) : FALSE;
}

UINT CDataGrid::GetCurrentRow() const
{
	return (m_FocusItem.y!=-1) ? (UINT)m_FocusItem.y : 0;
}

void CDataGrid::DoCopy(BOOL Cut)
{
	if (OpenClipboard())
	{
		UINT Count = 0;

		// Text erstellen
		CString Text;
		if (HasSelection())
		{
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				if (IsSelected(a))
				{
					Text += p_Itinerary->Flight2Text(a);
					Count++;
				}
		}
		else
			if ((m_FocusItem.y>=0) && (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
			{
				WCHAR tmpBuf[256];
				AttributeToString(p_Itinerary->m_Flights[m_FocusItem.y], m_ViewParameters.ColumnOrder[m_FocusItem.x], tmpBuf, 256);

				Text = tmpBuf;
			}

		EmptyClipboard();

		// CF_UNICODETEXT
		SIZE_T Size = (Text.GetLength()+1)*sizeof(WCHAR);
		HGLOBAL hClipBuffer = GlobalAlloc(GMEM_DDESHARE, Size);
		if (hClipBuffer)
		{
			LPWSTR pBuffer = (LPWSTR)GlobalLock(hClipBuffer);
			wcscpy_s((LPWSTR)pBuffer, Size, Text);

			GlobalUnlock(hClipBuffer);

			SetClipboardData(CF_UNICODETEXT, hClipBuffer);
		}

		// CF_FLIGHTS
		if (Count)
		{
			Size = Count*sizeof(AIRX_Flight);
			hClipBuffer = GlobalAlloc(GMEM_DDESHARE, Size);
			if (hClipBuffer)
			{
				AIRX_Flight* pFlight = (AIRX_Flight*)GlobalLock(hClipBuffer);

				for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
					if (IsSelected(a))
					{
						*pFlight = p_Itinerary->m_Flights[a];
						pFlight++;
					}

				GlobalUnlock(hClipBuffer);

				SetClipboardData(theApp.CF_FLIGHTS, hClipBuffer);
			}
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
	if (m_FocusItem.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
		m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount;

	// Header
	CRect rect;
	GetWindowRect(rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	wp.x = LEFTMARGIN;
	wp.y = 0;
	m_HeaderHeight = wp.cy;

	AdjustScrollbars();
	Invalidate();

	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

void CDataGrid::AdjustHeader()
{
	m_IgnoreHeaderItemChange = TRUE;
	SetRedraw(FALSE);

	// Set column order
	VERIFY(m_wndHeader.SetOrderArray(FMAttributeCount, m_ViewParameters.ColumnOrder));

	// Set column properties
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
				if (HdItem.cxy<MINCOLUMNWIDTH)
				{
					m_ViewParameters.ColumnWidth[a] = theApp.m_ViewParameters.ColumnWidth[a] = HdItem.cxy = MINCOLUMNWIDTH;
				}

		m_wndHeader.SetItem(a, &HdItem);
	}

	SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	m_IgnoreHeaderItemChange = FALSE;
}

void CDataGrid::EditCell(BOOL AllowCursor, BOOL Delete, WCHAR PushChar, CPoint Item)
{
	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;

	if ((Item.x==-1) || (Item.y==-1) || (Item.x>=FMAttributeCount) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	if (!FMAttributes[Attr].Editable)
		return;

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);
	const LPVOID pData = NewLine ? NULL : (((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset);
	Delete |= NewLine;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeColor:
		if (!PushChar)
		{
			COLORREF clr = pData ? *((COLORREF*)pData) : (COLORREF)-1;
			if (theApp.ChooseColor(&clr, this))
			{
				if (NewLine)
				{
					p_Itinerary->AddFlight();
					Item.y = p_Itinerary->m_Flights.m_ItemCount-1;

					AdjustLayout();
				}

				*((COLORREF*)(((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset)) = clr;

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

	INT x = -m_HScrollPos+LEFTMARGIN;

	for (INT a=0; a<Item.x; a++)
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	INT y = Item.y*m_RowHeight+m_HeaderHeight-m_VScrollPos;

	WCHAR tmpBuf[256] = L"";
	if (PushChar)
	{
		tmpBuf[0] = PushChar;
		tmpBuf[1] = L'\0';
	}
	else
		if (!Delete)
		{
			AttributeToString(p_Itinerary->m_Flights[Item.y], Attr, tmpBuf, 256);
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

void CDataGrid::EditFlight(CPoint Item, INT SelectTab)
{
	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;

	if ((Item.x==-1) || (Item.y==-1) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);

	EditFlightDlg dlg(NewLine ? NULL : &p_Itinerary->m_Flights[Item.y], this, p_Itinerary, SelectTab);
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

		p_Itinerary->m_Flights[Item.y] = dlg.m_Flight;
		p_Itinerary->m_IsModified = TRUE;
	}
	else
		if (NewLine)
		{
			p_Itinerary->DeleteAttachments(&dlg.m_Flight);
		}
		else
		{
			p_Itinerary->m_Flights[Item.y].AttachmentCount = dlg.m_Flight.AttachmentCount;
			memcpy_s(p_Itinerary->m_Flights[Item.y].Attachments, AIRX_MaxAttachmentCount*sizeof(UINT), dlg.m_Flight.Attachments, AIRX_MaxAttachmentCount*sizeof(UINT));
		}
}

void CDataGrid::EnsureVisible(CPoint Item)
{
	if ((Item.x==-1) || (Item.y==-1))
		Item = m_FocusItem;

	if ((Item.x==-1) || (Item.y==-1) || (Item.x>=FMAttributeCount) || (Item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	CRect rect;
	GetClientRect(rect);

	// Vertikal
	INT nInc = 0;

	if ((INT)((Item.y+1)*m_RowHeight)>m_VScrollPos+rect.Height()-(INT)m_HeaderHeight)
		nInc = (Item.y+1)*m_RowHeight-rect.Height()+(INT)m_HeaderHeight-m_VScrollPos;

	if ((INT)(Item.y*m_RowHeight)<m_VScrollPos+nInc)
		nInc = Item.y*m_RowHeight-m_VScrollPos;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
	}

	// Horizontal
	INT x = LEFTMARGIN;
	for (INT a=0; a<Item.x; a++)
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	nInc = 0;

	if (x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]>m_HScrollPos+rect.Width())
		nInc = x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]-rect.Width()-m_HScrollPos+LEFTMARGIN;

	if (x<m_HScrollPos+nInc)
		nInc = x-m_HScrollPos;

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);
	}
}

void CDataGrid::ResetScrollbars()
{
	ScrollWindow(m_HScrollPos, m_VScrollPos);

	SetScrollPos(SB_VERT, m_VScrollPos=0);
	SetScrollPos(SB_HORZ, m_HScrollPos=0);
}

void CDataGrid::AdjustScrollbars()
{
	INT ScrollHeight = (p_Itinerary->m_Flights.m_ItemCount+1)*m_RowHeight-1;
	INT ScrollWidth = LEFTMARGIN-1;

	for (UINT a=0; a<FMAttributeCount; a++)
		ScrollWidth += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	// Dimensions
	CRect rect;
	GetWindowRect(rect);

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

	// Set vertical bars
	m_VScrollMax = max(0, ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMax = ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	// Set horizontal bars
	m_HScrollMax = max(0, ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	si.nPage = rect.Width();
	si.nMax = ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);
}

BOOL CDataGrid::HitTest(CPoint point, CPoint* Item, INT* pSubitem)
{
	ASSERT(Item);

	point.y -= m_HeaderHeight-m_VScrollPos;

	INT y = (point.y>=0) ? point.y/m_RowHeight : -1;
	if ((y!=-1) && (y<=(INT)p_Itinerary->m_Flights.m_ItemCount))
	{
		INT x = -m_HScrollPos+LEFTMARGIN;

		for (UINT a=0; a<FMAttributeCount; a++)
		{
			const UINT Attr = m_ViewParameters.ColumnOrder[a];
			if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[Attr]))
			{
				Item->x = a;
				Item->y = y;

				if (pSubitem)
				{
					*pSubitem = -1;

					if (y<(INT)p_Itinerary->m_Flights.m_ItemCount)
					{
						x = point.x-x-2*ITEMPADDING;

						if (x>0)
							switch (FMAttributes[Attr].Type)
							{
							case FMTypeFlags:
								if (x<18*FLAGCOUNT)
									if (x%18<16)
										*pSubitem = x/18;

								break;

							case FMTypeRating:
								if (x<RATINGBITMAPWIDTH+6)
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

void CDataGrid::InvalidateItem(const CPoint& Item)
{
	if ((Item.x!=-1) && (Item.y!=-1))
	{
		INT x = -m_HScrollPos+LEFTMARGIN;
		for (UINT a=0; a<(UINT)Item.x; a++)
			x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

		InvalidateRect(CRect(x-2, m_HeaderHeight+Item.y*m_RowHeight-m_VScrollPos-2, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]+1, m_HeaderHeight-m_VScrollPos+(Item.y+1)*m_RowHeight+1));
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
	CRect rect;
	GetClientRect(rect);

	InvalidateRect(CRect(rect.left, m_HeaderHeight+Row*m_RowHeight-m_VScrollPos-2, rect.right, m_HeaderHeight-m_VScrollPos+(Row+1)*m_RowHeight+1));
}

void CDataGrid::SetFocusItem(const CPoint& FocusItem, BOOL ShiftSelect)
{
	if (FocusItem==m_FocusItem)
		return;

	if (ShiftSelect)
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_FocusItem.y;

		for (INT a=0; a<(INT)p_Itinerary->m_Flights.m_ItemCount; a++)
			SelectItem(a, ((a>=FocusItem.y) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=FocusItem.y)), TRUE);
	}
	else
	{
		m_SelectionAnchor = -1;

		for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
			SelectItem(a, FALSE, TRUE);
	}

	m_FocusItem = FocusItem;
	Invalidate();
	EnsureVisible();
}

void CDataGrid::SelectItem(UINT Index, BOOL Select, BOOL InternalCall)
{
	if (Index<p_Itinerary->m_Flights.m_ItemCount)
		if (Select!=((p_Itinerary->m_Flights[Index].Flags & AIRX_Selected)!=0))
		{
			if (Select)
			{
				p_Itinerary->m_Flights[Index].Flags |= AIRX_Selected;
			}
			else
			{
				p_Itinerary->m_Flights[Index].Flags &= ~AIRX_Selected;
			}

			if (!InternalCall)
				InvalidateRow(Index);
		}
}

__forceinline void CDataGrid::DrawCell(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect& rectItem, BOOL Selected)
{
	ASSERT(Attr<FMAttributeCount);

	// Background
	if (!Selected && (FMAttributes[Attr].Type==FMTypeClass))
		switch (Flight.Class)
		{
		case AIRX_Economy:
		case AIRX_PremiumEconomy:
		case AIRX_Charter:
			dc.FillSolidRect(rectItem, 0xE0FFE0);
			break;

		case AIRX_Business:
			dc.FillSolidRect(rectItem, 0xFFF0E0);
			break;

		case AIRX_First:
			dc.FillSolidRect(rectItem, 0xE0E0FF);
			break;

		case AIRX_Crew:
			dc.FillSolidRect(rectItem, 0xD8FFFF);
			break;
		}

	// Foreground
	rectItem.left += 2*ITEMPADDING;
	rectItem.right -= 2*ITEMPADDING+1;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeColor:
		if (*((COLORREF*)(((LPBYTE)&Flight)+FMAttributes[Attr].Offset))!=(COLORREF)-1)
		{
			rectItem.InflateRect(ITEMPADDING, 0);
			rectItem.DeflateRect(0, ITEMPADDING);
			dc.Draw3dRect(rectItem, 0x000000, 0x000000);

			rectItem.DeflateRect(1, 1);
			dc.FillSolidRect(rectItem, Flight.Color);
		}

		break;

	case FMTypeFlags:
		rectItem.top += (rectItem.Height()-m_SmallIcons.GetIconSize())/2;

		for (UINT a=0; a<FLAGCOUNT; a++)
		{
			const BOOL Enabled = a ? Flight.Flags & DisplayFlags[a] : Flight.AttachmentCount;
			m_SmallIcons.Draw(dc, rectItem.left, rectItem.top, a, FALSE, !Enabled);

			rectItem.left += 18;
		}

		break;

	case FMTypeRating:
		{
			// Rating bitmap
			const UCHAR Rating = (UCHAR)(*((UINT*)(((LPBYTE)&Flight)+FMAttributes[Attr].Offset))>>FMAttributes[Attr].DataParameter);
			ASSERT(Rating<=MAXRATING);

			CDC dcMem;
			dcMem.CreateCompatibleDC(&dc);

			HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(theApp.hRatingBitmaps[Rating]);

			dc.AlphaBlend(rectItem.left, (rectItem.top+rectItem.bottom-RATINGBITMAPHEIGHT)/2-1, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);

			SelectObject(dcMem, hOldBitmap);
		}

		break;

	default:
		{
			WCHAR tmpStr[256];
			AttributeToString(Flight, Attr, tmpStr, 256);

			dc.DrawText(tmpStr, -1, rectItem, DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | (((FMAttributes[Attr].Type==FMTypeDistance) || (FMAttributes[Attr].Type==FMTypeUINT) || (Attr==14)) ? DT_RIGHT : DT_LEFT));
		}
	}
}

INT CDataGrid::GetMaxAttributeWidth(UINT Attr) const
{
	ASSERT(Attr<FMAttributeCount);

	// Calculate width
	INT Width = 0;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	for (UINT Row=0; Row<p_Itinerary->m_Flights.m_ItemCount; Row++)
	{
		INT cx = SPACER;

		WCHAR tmpStr[256];
		AttributeToString(p_Itinerary->m_Flights[Row], Attr, tmpStr, 256);

		cx += dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr)).cx;

		if (cx>Width)
		{
			Width = cx;

			// Abort loop when MAXAUTOWIDTH has been reached or exceeded
			if (Width>=MAXAUTOWIDTH)
			{
				Width = MAXAUTOWIDTH;
				break;
			}
		}
	}
	
	dc.SelectObject(pOldFont);

	return max(Width, MINCOLUMNWIDTH);
}

void CDataGrid::AutosizeColumn(UINT Attr)
{
	ASSERT(Attr<FMAttributeCount);

	theApp.m_ViewParameters.ColumnWidth[Attr] = m_ViewParameters.ColumnWidth[Attr] = GetMaxAttributeWidth(Attr);
}

void CDataGrid::FinishEdit(LPWSTR pStr, const CPoint& Item)
{
	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	StringToAttribute(pStr, p_Itinerary->m_Flights[Item.y], Attr);

	p_Itinerary->m_IsModified = TRUE;
	InvalidateItem(Item);

	switch (Attr)
	{
	case 0:
	case 3:
		CalcDistance(p_Itinerary->m_Flights[Item.y], TRUE);
		InvalidateItem(Item.y, 6);
		break;

	case 1:
		CalcFuture(p_Itinerary->m_Flights[Item.y]);

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

		WCHAR tmpBuf[256];
		p_Edit->GetWindowText(tmpBuf, 256);

		// Destroying the edit control will release its focus, in turn causing DestroyEdit()
		// to be called. To prevent an infinite recursion, we set p_Edit to NULL first.
		CEdit* pVictim = p_Edit;
		p_Edit = NULL;

		pVictim->DestroyWindow();
		delete pVictim;

		if ((Accept) && (Item.x!=-1) && (Item.y!=-1))
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

void CDataGrid::FindReplace(INT SelectTab)
{
	FindReplaceDlg dlg(m_ViewParameters.ColumnOrder[m_FocusItem.x], this, SelectTab);
	if (dlg.DoModal()==IDOK)
	{
		theApp.m_FindReplaceSettings = m_FindReplaceSettings = dlg.m_FindReplaceSettings;

		// Something to search?
		if (m_FindReplaceSettings.SearchTerm[0]!=L'\0')
		{
			theApp.AddToRecentSearchTerms(m_FindReplaceSettings.SearchTerm);

			if (m_FindReplaceSettings.DoReplace && (m_FindReplaceSettings.ReplaceTerm[0]!=L'\0'))
				theApp.AddToRecentReplaceTerms(m_FindReplaceSettings.ReplaceTerm);

			OnFindReplaceAgain();
		}
	}
}

void CDataGrid::ScrollWindow(INT dx, INT dy, LPCRECT /*lpRect*/, LPCRECT /*lpClipRect*/)
{
	CRect rect;
	GetClientRect(rect);

	rect.top = m_HeaderHeight;

	if (dx!=0)
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		WINDOWPOS wp;
		HDLAYOUT HdLayout;
		HdLayout.prc = &rectWindow;
		HdLayout.pwpos = &wp;
		m_wndHeader.Layout(&HdLayout);

		wp.x = BACKSTAGEBORDER-1;
		wp.y = 0;

		m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (IsCtrlThemed() && (dy!=0))
	{
		rect.bottom -= BACKSTAGERADIUS;

		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
		RedrawWindow(CRect(rect.left, rect.bottom, rect.right, rect.bottom+BACKSTAGERADIUS), NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
	}
}


BEGIN_MESSAGE_MAP(CDataGrid, CFrontstageWnd)
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

	ON_COMMAND(IDM_DATAGRID_CUT, OnCut)
	ON_COMMAND(IDM_DATAGRID_COPY, OnCopy)
	ON_COMMAND(IDM_DATAGRID_PASTE, OnPaste)
	ON_COMMAND(IDM_DATAGRID_INSERTROW, OnInsertRow)
	ON_COMMAND(IDM_DATAGRID_DELETE, OnDelete)
	ON_COMMAND(IDM_DATAGRID_EDITFLIGHT, OnEditFlight)
	ON_COMMAND(IDM_DATAGRID_ADDROUTE, OnAddRoute)
	ON_COMMAND(IDM_DATAGRID_FIND, OnFind)
	ON_COMMAND(IDM_DATAGRID_REPLACE, OnReplace)
	ON_COMMAND(IDM_DATAGRID_FINDREPLACE, OnFindReplace)
	ON_COMMAND(IDM_DATAGRID_FINDREPLACEAGAIN, OnFindReplaceAgain)
	ON_COMMAND(IDM_DATAGRID_FILTER, OnFilter)
	ON_COMMAND(IDM_DATAGRID_SELECTALL, OnSelectAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DATAGRID_CUT, IDM_DATAGRID_SELECTALL, OnUpdateEditCommands)

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
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Header
	if (!m_wndHeader.Create(this, 1, TRUE))
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

	// Time
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_wDay = st.wDay;

	// Items
	m_LargeIcons.Load(IDB_FLAGS_16, LI_FORTOOLTIPS);
	m_SmallIcons.Load(IDB_FLAGS_16, LI_NORMAL, &theApp.m_DefaultFont);

	m_RowHeight = (max(m_SmallIcons.GetIconSize(), theApp.m_DefaultFont.GetFontHeight())+2*ITEMPADDING+1) & ~1;

	ResetScrollbars();

	SetTimer(1, 1000, NULL);

	return 0;
}

void CDataGrid::OnDestroy()
{
	KillTimer(1);

	CFrontstageWnd::OnDestroy();
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

			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				CalcFuture(p_Itinerary->m_Flights[a], &st);

			Invalidate();
		}
	}

	CFrontstageWnd::OnTimer(nIDEvent);

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

	const BOOL Themed = IsCtrlThemed();

	const COLORREF clrBackground = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	const COLORREF clrLines = Themed ? 0xEAE9E8 : GetSysColor(COLOR_3DFACE);
	const COLORREF clrText = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);
	const COLORREF clrDisabled = Themed ? 0xA0A0A0 : GetSysColor(COLOR_GRAYTEXT);

	dc.FillSolidRect(CRect(0, m_HeaderHeight, rect.right, rect.bottom), clrBackground);

	// Cells
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	INT Start = m_VScrollPos/m_RowHeight;
	INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
	for (UINT Row=Start; Row<=p_Itinerary->m_Flights.m_ItemCount; Row++)
	{
		const BOOL Selected = (Row<p_Itinerary->m_Flights.m_ItemCount) && (p_Itinerary->m_Flights[Row].Flags & AIRX_Selected);

		INT x = -m_HScrollPos+LEFTMARGIN;
		for (UINT Col=0; Col<FMAttributeCount; Col++)
		{
			const UINT Attr = m_ViewParameters.ColumnOrder[Col];
			const INT Width = m_ViewParameters.ColumnWidth[Attr];
			if (Width)
			{
				CRect rectItem(x, y, x+Width, y+m_RowHeight-1);
				CRect rectIntersect;
				if (rectIntersect.IntersectRect(rectItem, rectUpdate))
				{
					if (Selected)
					{
						dc.FillSolidRect(rectItem, Themed ? 0xFF9018 : GetSysColor(COLOR_HIGHLIGHT));
					}
					else
						if (!FMAttributes[Attr].Editable)
						{
							dc.FillSolidRect(rectItem, Themed ? 0xF7F6F5 : clrLines);
						}

					if (Row<p_Itinerary->m_Flights.m_ItemCount)
					{
						const DWORD Flags = p_Itinerary->m_Flights[Row].Flags;
						dc.SetTextColor(Selected ? Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT) : ((Attr==0) && (Flags & AIRX_UnknownFrom)) || ((Attr==3) && (Flags & AIRX_UnknownTo)) ? 0x0000FF : (Flags & AIRX_Cancelled) ? clrDisabled : (Flags & AIRX_FutureFlight) ? 0x008000 : clrText);

						DrawCell(dc, p_Itinerary->m_Flights[Row], Attr, rectItem, Selected);
					}
				}

				x += Width;
			}
		}

		// Horizontal lines
		y += m_RowHeight;
		dc.FillSolidRect(-m_HScrollPos+LEFTMARGIN, y-1, x-(-m_HScrollPos+LEFTMARGIN), 1, clrLines);

		if (y>rect.Height())
			break;
	}

	// Vertical lines
	INT x = -m_HScrollPos+LEFTMARGIN;
	dc.FillSolidRect(x, 0, 1, y-1, clrLines);

	for (UINT Col=0; Col<FMAttributeCount; Col++)
	{
		const UINT Attr = m_ViewParameters.ColumnOrder[Col];
		const INT Width = m_ViewParameters.ColumnWidth[Attr];
		if (Width)
		{
			x += Width;
			dc.FillSolidRect(x-1, 0, 1, y-1, clrLines);
		}
	}

	// Focus cell
	if ((m_FocusItem.x!=-1) && (m_FocusItem.y!=-1))
	{
		INT x = -m_HScrollPos+LEFTMARGIN;
		for (INT Col=0; Col<m_FocusItem.x; Col++)
			x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]];

		CRect rect(x-2, m_FocusItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos-2, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[m_FocusItem.x]]+1, m_FocusItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos+m_RowHeight+1);

		for (UINT a=0; a<3; a++)
		{
			dc.Draw3dRect(rect, clrText, clrText);
			rect.DeflateRect(1, 1);
		}
	}

	// Header
	dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, Themed ? clrBackground : GetSysColor(COLOR_3DFACE));

	if (Themed)
	{
		Bitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP);

		Graphics g(dc);
		g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+GetScrollPos(SB_HORZ)+BACKSTAGEBORDER-1, m_HeaderHeight-(INT)pDivider->GetHeight());

		CTaskbar::DrawTaskbarShadow(g, rect);
	}

	dc.SelectObject(pOldFont);

	CFrontstageWnd::DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CDataGrid::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

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
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(rect);
			ScreenToClient(rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
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
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		UpdateWindow();
	}

	CFrontstageWnd::OnHScroll(nSBCode, nPos, pScrollBar);
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
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((FMGetApp()->IsTooltipVisible()) && ((Item!=m_HotItem) || (Subitem!=m_HotSubitem)))
			FMGetApp()->HideTooltip();

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
	FMGetApp()->HideTooltip();
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem.x = m_HotItem.y = m_HotSubitem = -1;
}

void CDataGrid::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem.x!=-1) && (m_HotItem.y!=-1) && (!p_Edit))
			if (!FMGetApp()->IsTooltipVisible() && (m_HotItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
			{
				const AIRX_Flight* pFlight = &p_Itinerary->m_Flights[m_HotItem.y];
				const UINT Attr = m_ViewParameters.ColumnOrder[m_HotItem.x];
				WCHAR tmpStr[256];

				switch (Attr)
				{
				case 0:
					if (strlen(pFlight->From.Code)==3)
						FMGetApp()->ShowTooltip(this, point, pFlight->From.Code, _T(""));

					break;

				case 3:
					if (strlen(pFlight->To.Code)==3)
						FMGetApp()->ShowTooltip(this, point, pFlight->To.Code, _T(""));

					break;

				default:
					switch (FMAttributes[Attr].Type)
					{
					case FMTypeFlags:
						if (m_HotSubitem!=-1)
						{
							CString Caption((LPCSTR)IDS_ATTACHMENTS+m_HotSubitem);
							CString Hint;

							INT Pos = Caption.Find(L'\n');
							if (Pos!=-1)
							{
								Hint = Caption.Mid(Pos+1);
								Caption = Caption.Left(Pos);
							}

							FMGetApp()->ShowTooltip(this, point, Caption, Hint, m_LargeIcons.ExtractIcon(m_HotSubitem));
						}

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
									const LPVOID pData = (((LPBYTE)&p_Itinerary->m_Flights[a])+FMAttributes[Attr].Offset);

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
								CString Hint;
								WCHAR tmpMin[256];
								WCHAR tmpMax[256];
								WCHAR tmpAvg[256];

								switch (FMAttributes[Attr].Type)
								{
								case FMTypeUINT:
									Hint.Format(IDS_TOOLTIP_UINT, UMin, UMax, (DOUBLE)USum/(DOUBLE)Count);
									break;

								case FMTypeTime:
									TimeToString(tmpMin, 256, UMin);
									TimeToString(tmpMax, 256, UMax);
									TimeToString(tmpAvg, 256, (UINT)((DOUBLE)USum/(DOUBLE)Count));

									Hint.Format(IDS_TOOLTIP_TIME, tmpMin, tmpMax, tmpAvg);
									break;

								case FMTypeDistance:
									DistanceToString(tmpMin, 256, DMin);
									DistanceToString(tmpMax, 256, DMax);
									DistanceToString(tmpAvg, 256, DSum/(DOUBLE)Count);

									Hint.Format(IDS_TOOLTIP_DISTANCE, tmpMin, tmpMax, tmpAvg);
									break;

								case FMTypeDateTime:
									DateTimeToString(tmpMin, 256, FMin);
									DateTimeToString(tmpMax, 256, FMax);

									Hint.Format(IDS_TOOLTIP_DATETIME, tmpMin, tmpMax);
									break;
								}

								theApp.ShowTooltip(this, point, CString((LPCSTR)IDS_COLUMN0+Attr), Hint);
							}
							break;
						}

					default:
						AttributeToString(p_Itinerary->m_Flights[m_HotItem.y], Attr, tmpStr, 256);

						if (tmpStr[0]!=L'\0')
						{
							CSize szText = theApp.m_DefaultFont.GetTextExtent(tmpStr);

							if ((szText.cx>m_ViewParameters.ColumnWidth[Attr]-SPACER) || (FMAttributes[Attr].Type==FMTypeColor))
								theApp.ShowTooltip(this, point, _T(""), tmpStr);
						}
					}
				}
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

BOOL CDataGrid::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		FMGetApp()->HideTooltip();

		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CDataGrid::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);
	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		FMGetApp()->HideTooltip();

		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CDataGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
		for (INT Col=Item.x-1; Col>=0; Col--)
			if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
			{
				Item.x = Col;
				break;
			}

		break;

	case VK_RIGHT:
	case VK_EXECUTE:
	case VK_RETURN:
		for (INT Col=Item.x+1; Col<FMAttributeCount; Col++)
			if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
			{
				Item.x = Col;
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
			for (INT Col=0; Col<FMAttributeCount; Col++)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
				{
					Item.x = Col;
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
			for (INT Col=FMAttributeCount-1; Col>=0; Col--)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
				{
					Item.x = Col;
					break;
				}
		}

		break;

	case 'A':
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnSelectAll();

		break;

	default:
		CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);
}

void CDataGrid::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (!p_Edit)
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
			SelectItem(Item.y, !IsSelected(Item.y));
		}
		else
			if (Item==m_FocusItem)
			{
				if (Item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				{
					const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
					const LPVOID pData = (((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset);

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
		if (!(nFlags & MK_CONTROL))
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
			{
				for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
					SelectItem(a, FALSE, TRUE);

				Invalidate();
			}
			else
			{
				InvalidateItem(m_FocusItem);
				m_FocusItem = Item;
				EnsureVisible();
				InvalidateItem(m_FocusItem);
			}
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
			m_FocusItem = Item;
			EnsureVisible();

			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				SelectItem(a, FALSE, TRUE);

			Invalidate();
		}
	}
	else
		if (!(nFlags & MK_CONTROL))
		{
			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				SelectItem(a, FALSE, TRUE);

			Invalidate();
		}

	GetParent()->UpdateWindow();

	CFrontstageWnd::OnRButtonUp(nFlags, point);
}

void CDataGrid::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd->GetSafeHwnd()==m_wndHeader)
	{
		CMenu Menu;
		Menu.LoadMenu(IDM_DETAILS);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		CPoint pt(point);
		ScreenToClient(&pt);

		HDHITTESTINFO htt;
		htt.pt = pt;
		m_HeaderItemClicked = m_wndHeader.HitTest(&htt);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);

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

	CMenu Menu;
	Menu.LoadMenu(IDM_DATAGRID);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
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
	ASSERT(HasSelection(TRUE));

	DoCopy(TRUE);
}

void CDataGrid::OnCopy()
{
	ASSERT(HasSelection(TRUE));

	DoCopy(FALSE);
}

void CDataGrid::OnPaste()
{
	if (OpenClipboard())
	{
		// CF_FLIGHTS
		HGLOBAL hClipBuffer = GetClipboardData(theApp.CF_FLIGHTS);
		if (hClipBuffer)
		{
			LPVOID pBuffer = GlobalLock(hClipBuffer);

			p_Itinerary->InsertFlights(m_FocusItem.y, (UINT)(GlobalSize(hClipBuffer)/sizeof(AIRX_Flight)), (AIRX_Flight*)pBuffer);
			p_Itinerary->m_IsModified = TRUE;
			m_SelectionAnchor = -1;
			AdjustLayout();

			GlobalUnlock(hClipBuffer);

			goto Finish;
		}

		// CF_UNICODETEXT
		hClipBuffer = GetClipboardData(CF_UNICODETEXT);
		if (hClipBuffer)
		{
			LPVOID pBuffer = GlobalLock(hClipBuffer);

			if (m_FocusItem.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				p_Itinerary->AddFlight();
				m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount-1;

				AdjustLayout();
			}

			FinishEdit((WCHAR*)pBuffer, m_FocusItem);

			GlobalUnlock(hClipBuffer);
		}

Finish:
		CloseClipboard();
	}
}

void CDataGrid::OnInsertRow()
{
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
	ASSERT(HasSelection(TRUE));

	DoDelete();
}

void CDataGrid::OnEditFlight()
{
	EditFlight();
}

void CDataGrid::OnAddRoute()
{
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
			p_Itinerary->m_IsModified = TRUE;

			AdjustLayout();
			SetFocusItem(CPoint(m_FocusItem.x, p_Itinerary->m_Flights.m_ItemCount-1), FALSE);
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

void CDataGrid::OnFindReplace()
{
	FindReplace();
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
			FMMessageBox(this, CString((LPCSTR)(m_FindReplaceSettings.DoReplace ? IDS_ILLEGALCOLUMN_REPLACE : IDS_ILLEGALCOLUMN_FIND)), CString((LPCSTR)IDS_FINDREPLACE), MB_ICONERROR | MB_OK);

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
			for (INT Col=Item.x+1; Col<FMAttributeCount; Col++)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
					if (ColumnValid(m_ViewParameters.ColumnOrder[Col]))
					{
						Item.x = Col;
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
				FMMessageBox(this, CString((LPCSTR)((m_FindReplaceSettings.DoReplace && Replaced) ? IDS_ALLREPLACED : IDS_SEARCHTERMNOTFOUND)), CString((LPCSTR)IDS_FINDREPLACE), (m_FindReplaceSettings.DoReplace && Replaced) ? MB_OK : MB_ICONEXCLAMATION | MB_OK);

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
	AttributeToString(p_Itinerary->m_Flights[Item.y], Attr, tmpStr, 256);

	if ((m_FindReplaceSettings.Flags & FRS_MATCHCASE)==0)
		ToUpper(tmpStr);

	const BOOL Match = (m_FindReplaceSettings.Flags & FRS_MATCHENTIRECELL) ? wcscmp(tmpStr, SearchTerm)==0 : wcsstr(tmpStr, SearchTerm)!=NULL;
	if (!Match)
		goto Again;

	// Process
	if (m_FindReplaceSettings.DoReplace)
	{
		Replaced = TRUE;

		if ((m_FindReplaceSettings.Flags & FRS_REPLACEALL)==0)
		{
			SetFocusItem(Item, FALSE);

			switch (FMMessageBox(this, CString((LPCSTR)IDS_REPLACEQUESTION), CString((LPCSTR)IDS_FINDREPLACE), MB_ICONQUESTION | MB_YESNOCANCEL))
			{
			case IDNO:
				goto Again;

			case IDCANCEL:
				return;
			}
		}

		StringToAttribute(m_FindReplaceSettings.ReplaceTerm, p_Itinerary->m_Flights[Item.y], Attr);
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
			const AIRX_Flight* pFlight = &p_Itinerary->m_Flights[a];

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
	}
}

void CDataGrid::OnSelectAll()
{
	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
		SelectItem(a, TRUE, TRUE);

	m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount;
	EnsureVisible();
	Invalidate();
}

void CDataGrid::OnUpdateEditCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_DATAGRID_CUT:
	case IDM_DATAGRID_COPY:
	case IDM_DATAGRID_DELETE:
		bEnable = HasSelection(TRUE);
		break;

	case IDM_DATAGRID_PASTE:
		{
			COleDataObject dobj;
			if (dobj.AttachClipboard())
				bEnable &= dobj.IsDataAvailable(theApp.CF_FLIGHTS) || dobj.IsDataAvailable(CF_UNICODETEXT);
		}

		break;

	case IDM_DATAGRID_FINDREPLACEAGAIN:
		bEnable &= (m_FindReplaceSettings.SearchTerm[0]!=L'\0');

	case IDM_DATAGRID_FIND:
	case IDM_DATAGRID_REPLACE:
	case IDM_DATAGRID_FINDREPLACE:
	case IDM_DATAGRID_FILTER:
	case IDM_DATAGRID_SELECTALL:
			bEnable &= (p_Itinerary->m_Flights.m_ItemCount!=0);

		break;
	}

	pCmdUI->Enable(bEnable);
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

		for (INT Col=m_FocusItem.x; Col>=0; Col--)
			if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
			{
				m_FocusItem.x = Col;
				break;
			}

		AdjustHeader();
		AdjustLayout();
	}
}

void CDataGrid::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (pCmdUI->m_nID!=IDM_DETAILS_AUTOSIZE);

	if (pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE)
		if (m_HeaderItemClicked!=-1)
			bEnable = (FMAttributes[m_HeaderItemClicked].Type!=FMTypeRating) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeColor) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags);

	pCmdUI->Enable(bEnable);
}


// Header

void CDataGrid::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	DestroyEdit(TRUE);

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
			// 1: remove column at old position
			for (UINT a=0; a<FMAttributeCount; a++)
				if (m_ViewParameters.ColumnOrder[a]==pHdr->iItem)
				{
					for (UINT b=a; b<FMAttributeCount-1; b++)
						m_ViewParameters.ColumnOrder[b] = m_ViewParameters.ColumnOrder[b+1];

					break;
				}

			// 2: insert column at new position
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
	{
		DestroyEdit(TRUE);

		// Do not track rating columns, hidden columns or the preview column
		*pResult = (FMAttributes[pHdr->iItem].Type==FMTypeRating) || (FMAttributes[pHdr->iItem].Type==FMTypeColor) || (FMAttributes[pHdr->iItem].Type==FMTypeFlags) || (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
	}
}

void CDataGrid::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (!m_IgnoreHeaderItemChange && (pHdr->pitem->mask & HDI_WIDTH))
	{
		// Guarantee MINCOLUMNWIDTH, or hide column
		const INT Width = (pHdr->pitem->cxy<MINCOLUMNWIDTH) ? (pHdr->iItem==0) || (pHdr->iItem==3) ? MINCOLUMNWIDTH : 0 : pHdr->pitem->cxy;

		if (Width!=m_ViewParameters.ColumnWidth[pHdr->iItem])
		{
			theApp.m_ViewParameters.ColumnWidth[pHdr->iItem] = m_ViewParameters.ColumnWidth[pHdr->iItem] = Width;

			AdjustHeader();
			AdjustLayout();
		}

		*pResult = TRUE;
	}
}

void CDataGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
