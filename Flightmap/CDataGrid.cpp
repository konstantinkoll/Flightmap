
// CDataGrid.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AddRouteDlg.h"
#include "CDataGrid.h"
#include "ChooseDetailsDlg.h"
#include "EditFlightDlg.h"
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
static const UINT DisplayFlags[] = { 0, AIRX_AwardFlight, AIRX_GroundTransportation, AIRX_BusinessTrip, AIRX_LeisureTrip };

CDataGrid::CDataGrid()
{
	p_Itinerary = NULL;
	p_Edit = NULL;
	m_HeaderItemClicked = m_SelectedItem.x = m_SelectedItem.y = m_HotItem.x = m_HotItem.y = m_HotSubitem = m_SelectionAnchor = -1;
	m_Hover = m_IgnoreHeaderItemChange = FALSE;
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
				SelectItem(CPoint(m_SelectedItem.x, m_SelectedItem.y+1), FALSE);
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

		m_SelectedItem.x = p_Itinerary ? 0 : -1;
		m_SelectedItem.y = p_Itinerary ? Row : -1;
		m_SelectionAnchor = -1;
		AdjustLayout();
		EnsureVisible();
	}
}

BOOL CDataGrid::HasSelection(BOOL CurrentLineIfNoneSelected)
{
	return p_Itinerary ? ((m_SelectionAnchor>=0) && (m_SelectionAnchor<(INT)p_Itinerary->m_Flights.m_ItemCount)) || (CurrentLineIfNoneSelected && (m_SelectedItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)) : FALSE;
}

void CDataGrid::GetSelection(INT& First, INT& Last, BOOL CurrentLineIfNoneSelected)
{
	ASSERT(p_Itinerary);

	if (!p_Itinerary->m_Flights.m_ItemCount)
	{
		First = 0;
		Last = -1;
	}
	else
		if (HasSelection())
		{
			First = min(m_SelectionAnchor, m_SelectedItem.y);
			Last = max(m_SelectionAnchor, m_SelectedItem.y);

			if (First>=(INT)p_Itinerary->m_Flights.m_ItemCount)
				First = 0;
			if (Last>=(INT)p_Itinerary->m_Flights.m_ItemCount)
				Last = p_Itinerary->m_Flights.m_ItemCount-1;
		}
		else
			if (HasSelection(CurrentLineIfNoneSelected))
			{
				First = Last = m_SelectedItem.y;
			}
			else
			{
				First = 0;
				Last = p_Itinerary->m_Flights.m_ItemCount-1;
			}
}

UINT CDataGrid::GetCurrentRow()
{
	if (p_Itinerary)
		if (m_SelectedItem.y!=-1)
			return (UINT)m_SelectedItem.y;

	return 0;
}

void CDataGrid::DoCopy(BOOL Cut)
{
	if (OpenClipboard())
	{
		INT Anfang;
		INT Ende;
		GetSelection(Anfang, Ende, TRUE);

		// CF_UNICODETEXT
		CString Buffer;
		for (INT a=Anfang; a<=Ende; a++)
			Buffer += p_Itinerary->Flight2Text(a);

		EmptyClipboard();

		// CF_UNICODETEXT
		SIZE_T sz = (Buffer.GetLength()+1)*sizeof(WCHAR);
		HGLOBAL ClipBuffer = GlobalAlloc(GMEM_DDESHARE, sz);
		LPVOID pBuffer = GlobalLock(ClipBuffer);
		wcscpy_s((WCHAR*)pBuffer, sz, Buffer.GetBuffer());
		GlobalUnlock(ClipBuffer);
		SetClipboardData(CF_UNICODETEXT, ClipBuffer);

		// CF_FLIGHTS
		sz = (Ende-Anfang+1)*sizeof(AIRX_Flight);
		ClipBuffer = GlobalAlloc(GMEM_DDESHARE, sz);
		pBuffer = GlobalLock(ClipBuffer);
		memcpy_s(pBuffer, sz, &p_Itinerary->m_Flights.m_Items[Anfang], sz);
		GlobalUnlock(ClipBuffer);
		SetClipboardData(theApp.CF_FLIGHTS, ClipBuffer);

		CloseClipboard();

		if (Cut)
			DoDelete(Anfang, Ende);
	}
}

void CDataGrid::DoDelete(INT Anfang, INT Ende)
{
	if (Ende>=Anfang)
	{
		p_Itinerary->DeleteFlights(Anfang, Ende-Anfang+1);
		p_Itinerary->m_IsModified = TRUE;

		m_SelectionAnchor = -1;
		AdjustLayout();
	}
}

void CDataGrid::AdjustLayout()
{
	if (p_Itinerary)
		if (m_SelectedItem.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
			m_SelectedItem.y = p_Itinerary->m_Flights.m_ItemCount;

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

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	m_IgnoreHeaderItemChange = FALSE;
}

void CDataGrid::EditCell(BOOL AllowCursor, BOOL Delete, WCHAR PushChar, CPoint item)
{
	if (!p_Itinerary)
		return;

	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=FMAttributeCount) || (item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	const UINT Attr = m_ViewParameters.ColumnOrder[item.x];
	if (!FMAttributes[Attr].Editable)
		return;

	EnsureVisible(item);
	const BOOL NewLine = (item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);
	const LPVOID pData = NewLine ? NULL : (((BYTE*)&p_Itinerary->m_Flights.m_Items[item.y])+FMAttributes[Attr].Offset);
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
					item.y = p_Itinerary->m_Flights.m_ItemCount-1;

					AdjustLayout();
				}

				*((COLORREF*)(((BYTE*)&p_Itinerary->m_Flights.m_Items[item.y])+FMAttributes[Attr].Offset)) = col;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(item);
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

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(item);
				break;
			case L'G':
			case L'g':
				*((DWORD*)pData) ^= AIRX_GroundTransportation;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(item);
				break;
			case L'B':
			case L'b':
				*((DWORD*)pData) ^= AIRX_BusinessTrip;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(item);
				break;
			case L'L':
			case L'l':
				*((DWORD*)pData) ^= AIRX_LeisureTrip;

				p_Itinerary->m_IsModified = TRUE;
				InvalidateItem(item);
				break;
			}

		return;
	case FMTypeRating:
		if (pData && (PushChar>=L'0') && (PushChar<=L'5'))
		{
			*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
			*((DWORD*)pData) |= (((PushChar-'0')*2)<<FMAttributes[Attr].DataParameter);

			p_Itinerary->m_IsModified = TRUE;
			InvalidateItem(item);
		}

		return;
	}

	INT y = item.y*m_RowHeight+m_HeaderHeight-m_VScrollPos;
	INT x = -m_HScrollPos;
	for (INT a=0; a<item.x; a++)
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
			AttributeToString(p_Itinerary->m_Flights.m_Items[item.y], Attr, tmpBuf, 256);
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

void CDataGrid::EditFlight(CPoint item, INT iSelectPage)
{
	if (!p_Itinerary)
		return;

	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
		return;

	EnsureVisible(item);
	const BOOL NewLine = (item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);

	EditFlightDlg dlg(NewLine ? NULL : &p_Itinerary->m_Flights.m_Items[item.y], this, p_Itinerary, iSelectPage);
	if (dlg.DoModal()==IDOK)
	{
		if (NewLine)
		{
			p_Itinerary->AddFlight();
			item.y = p_Itinerary->m_Flights.m_ItemCount-1;

			AdjustLayout();
		}
		else
		{
			Invalidate();
		}

		p_Itinerary->m_Flights.m_Items[item.y] = dlg.m_Flight;
		p_Itinerary->m_IsModified = TRUE;
	}
	else
		if (NewLine)
		{
			p_Itinerary->DeleteAttachments(&dlg.m_Flight);
		}
		else
		{
			p_Itinerary->m_Flights.m_Items[item.y].AttachmentCount = dlg.m_Flight.AttachmentCount;
			memcpy_s(p_Itinerary->m_Flights.m_Items[item.y].Attachments, AIRX_MaxAttachmentCount*sizeof(UINT), dlg.m_Flight.Attachments, AIRX_MaxAttachmentCount*sizeof(UINT));
		}
}

void CDataGrid::EnsureVisible(CPoint item)
{
	if (!p_Itinerary)
		return;

	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=FMAttributeCount) || (item.y>(INT)(p_Itinerary->m_Flights.m_ItemCount)))
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

BOOL CDataGrid::HitTest(CPoint point, CPoint* item, INT* subitem)
{
	ASSERT(item);

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
				item->x = a;
				item->y = row;

				if (subitem)
				{
					*subitem = -1;

					if (row<(INT)p_Itinerary->m_Flights.m_ItemCount)
					{
						x = point.x-x-MARGIN;

						if (x>0)
							switch (FMAttributes[Attr].Type)
							{
							case FMTypeFlags:
								if (x<18*5)
									if (x%18<16)
										*subitem = x/18;
								break;
							case FMTypeRating:
								if (x<RatingBitmapWidth+6)
									if ((x<6) || (x%18<16))
										*subitem = (x<6) ? 0 : 2*(x/18)+(x%18>8)+1;
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

void CDataGrid::SelectItem(CPoint Item, BOOL ShiftSelect)
{
	if (Item==m_SelectedItem)
		return;

	if (ShiftSelect)
	{
		if (m_SelectionAnchor==-1)
			m_SelectionAnchor = m_SelectedItem.y;

		Invalidate();
	}
	else
		if (m_SelectionAnchor!=-1)
		{
			m_SelectionAnchor = -1;
			Invalidate();
		}

	InvalidateItem(m_SelectedItem);
	m_SelectedItem = Item;
	EnsureVisible();
	InvalidateItem(Item);
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

			for (UINT a=0; a<5; a++)
			{
				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, theApp.m_FlagIcons16[a ? Flight.Flags & DisplayFlags[a] ? 1 : 0 : Flight.AttachmentCount>0 ? 1 : 0]);
				AlphaBlend(dc, pt.x, pt.y, 16, 16, hdcMem, a*16, 0, 16, 16, BF);
				SelectObject(hdcMem, hbmOld);

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

void CDataGrid::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		CPoint item = m_SelectedItem;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		WCHAR tmpBuf[256];
		victim->GetWindowText(tmpBuf, 256);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (p_Itinerary) && (item.x!=-1) && (item.y!=-1))
		{
			if (item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				if (tmpBuf[0]==L'\0')
					return;

				p_Itinerary->AddFlight();
				item.y = p_Itinerary->m_Flights.m_ItemCount-1;

				AdjustLayout();
			}

			const UINT Attr = m_ViewParameters.ColumnOrder[item.x];
			StringToAttribute(tmpBuf, p_Itinerary->m_Flights.m_Items[item.y], Attr);

			p_Itinerary->m_IsModified = TRUE;
			InvalidateItem(item);

			if ((Attr==0) || (Attr==3))
			{
				CalcDistance(p_Itinerary->m_Flights.m_Items[item.y], TRUE);
				InvalidateItem(item.y, 6);
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CDataGrid, CWnd)
	ON_WM_CREATE()
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
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
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

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (2*(MARGIN-1)+max(abs(lf.lfHeight), 16)) & ~1;

	ResetScrollbars();

	return 0;
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
	const COLORREF colBackground = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	const COLORREF colLines = Themed ? theApp.OSVersion==OS_Eight ? 0xEAE9E8 : 0xDDDCDA : GetSysColor(COLOR_SCROLLBAR);
	const COLORREF colText = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);
	dc.FillSolidRect(rect, colBackground);

	if (p_Itinerary)
	{
		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		INT start = m_VScrollPos/m_RowHeight;
		INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
		for (UINT row=start; row<=p_Itinerary->m_Flights.m_ItemCount; row++)
		{
			const BOOL Selected = (m_SelectionAnchor!=-1) && (row<p_Itinerary->m_Flights.m_ItemCount) && ((((INT)row>=m_SelectionAnchor) && ((INT)row<=m_SelectedItem.y)) || (((INT)row<=m_SelectionAnchor) && ((INT)row>=m_SelectedItem.y)));

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
							dc.SetTextColor((Selected && (!Themed || (theApp.OSVersion==OS_XP) || (theApp.OSVersion==OS_Eight))) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : ((Attr==0) && (Flags & AIRX_UnknownFrom)) || ((Attr==3) && (Flags & AIRX_UnknownTo)) ? 0x0000FF : colText);
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

		if ((m_SelectedItem.x!=-1) && (m_SelectedItem.y!=-1))
		{
			INT x = -m_HScrollPos;
			for (INT col=0; col<m_SelectedItem.x; col++)
				x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]];

			CRect rect(x-2, m_SelectedItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos-2, x+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[m_SelectedItem.x]]+1, m_SelectedItem.y*m_RowHeight+m_HeaderHeight-m_VScrollPos+m_RowHeight+1);
			//rect.InflateRect(1, 1);

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

	if (OnItem && ((Item!=m_HotItem) || (Subitem!=m_HotSubitem)))
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
					if (FMAttributes[Attr].Type==FMTypeFlags)
					{
						if (m_HotSubitem!=-1)
						{
							CString caption;
							CString message;
							ENSURE(message.LoadString(IDS_ATTACHMENTS+m_HotSubitem));

							INT pos = message.Find(L'\n');
							if (pos!=-1)
							{
								caption = message.Left(pos);
								message = message.Mid(pos+1);
							}

							m_TooltipCtrl.Track(point, theApp.m_FlagIcons32.ExtractIcon(m_HotSubitem), NULL, CSize(32, 32), caption, message);
						}
					}
					else
					{
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
								CSize szText = dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr));
								dc.SelectObject(pOldFont);

								if (szText.cx>m_ViewParameters.ColumnWidth[Attr]-2*MARGIN-1)
									m_TooltipCtrl.Track(point, NULL, NULL, CSize(0, 0), _T(""), tmpStr);
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

		CPoint item(m_SelectedItem);

		switch (nChar)
		{
		case VK_F2:
			EditCell(TRUE);
			return;
		case VK_BACK:
			if (m_SelectedItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				EditCell(FALSE, TRUE);
			return;
		case VK_DELETE:
			if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			{
				if (HasSelection(TRUE))
					OnDelete();
			}
			else
				if (m_SelectedItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				{
					const UINT Attr = m_ViewParameters.ColumnOrder[m_SelectedItem.x];
					StringToAttribute(L"", p_Itinerary->m_Flights.m_Items[m_SelectedItem.y], Attr);

					p_Itinerary->m_IsModified = TRUE;
					InvalidateItem(m_SelectedItem);

					if ((Attr==0) || (Attr==3))
					{
						CalcDistance(p_Itinerary->m_Flights.m_Items[m_SelectedItem.y], TRUE);
						InvalidateItem(m_SelectedItem.y, 6);
					}
				}
			return;
		case VK_LEFT:
			for (INT col=item.x-1; col>=0; col--)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
				{
					item.x = col;
					break;
				}
			break;
		case VK_RIGHT:
			for (INT col=item.x+1; col<FMAttributeCount; col++)
				if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
				{
					item.x = col;
					break;
				}
			break;
		case VK_UP:
			if (item.y>0)
				item.y--;
			break;
		case VK_PRIOR:
			item.y -= (rect.Height()-m_HeaderHeight)/(INT)m_RowHeight;
			if (item.y<0)
				item.y = 0;
			break;
		case VK_DOWN:
		case VK_EXECUTE:
		case VK_RETURN:
			if (item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				item.y++;
			break;
		case VK_NEXT:
			item.y += (rect.Height()-m_HeaderHeight)/(INT)m_RowHeight;
			if (item.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
				item.y = p_Itinerary->m_Flights.m_ItemCount;
			break;
		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.y = 0;
			}
			else
			{
				for (INT col=0; col<FMAttributeCount; col++)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					{
						item.x = col;
						break;
					}
			}
			break;
		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.y = p_Itinerary->m_Flights.m_ItemCount;
			}
			else
			{
				for (INT col=FMAttributeCount-1; col>=0; col--)
					if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[col]])
					{
						item.x = col;
						break;
					}
			}
			break;
		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			return;
		}

		SelectItem(item, GetKeyState(VK_SHIFT)<0);
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

	CPoint Item;
	INT Subitem = -1;
	if (HitTest(point, &Item, &Subitem))
	{
		if (p_Itinerary && (Item==m_SelectedItem) && (Subitem!=-1))
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
					{
						*((DWORD*)pData) ^= DisplayFlags[Subitem];

						p_Itinerary->m_IsModified = TRUE;
						InvalidateItem(Item);
					}
					break;
				case FMTypeRating:
					*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
					*((DWORD*)pData) |= (Subitem<<FMAttributes[Attr].DataParameter);

					p_Itinerary->m_IsModified = TRUE;
					InvalidateItem(Item);
					break;
				}
			}

		SelectItem(Item, nFlags & MK_SHIFT);
	}
}

void CDataGrid::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint Item;
	if (HitTest(point, &Item))
		if (Item==m_SelectedItem)
			switch (FMAttributes[m_ViewParameters.ColumnOrder[Item.x]].Type)
			{
			case FMTypeFlags:
			case FMTypeRating:
				OnLButtonDown(nFlags, point);
				break;
			default:
				EditCell();
			}
}

void CDataGrid::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item))
		SelectItem(Item, nFlags & MK_SHIFT);
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
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZEALL, 0);
		pPopup->AddCommand(IDM_DETAILS_AUTOSIZE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_DETAILS_CHOOSE, 1);

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

BOOL CDataGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(theApp.LoadStandardCursor(m_HotSubitem==-1 ? IDC_ARROW : IDC_HAND));

	return TRUE;
}

void CDataGrid::OnSetFocus(CWnd* /*pOldWnd*/)
{
	InvalidateItem(m_SelectedItem);
}

void CDataGrid::OnKillFocus(CWnd* /*pNewWnd*/)
{
	InvalidateItem(m_SelectedItem);
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
		HGLOBAL ClipBuffer = GetClipboardData(theApp.CF_FLIGHTS);
		LPVOID pBuffer = GlobalLock(ClipBuffer);

		p_Itinerary->InsertFlights(m_SelectedItem.y, (UINT)(GlobalSize(ClipBuffer)/sizeof(AIRX_Flight)), (AIRX_Flight*)pBuffer);
		p_Itinerary->m_IsModified = TRUE;
		m_SelectionAnchor = -1;
		AdjustLayout();

		GlobalUnlock(ClipBuffer);
		CloseClipboard();
	}
}

void CDataGrid::OnInsertRow()
{
	ASSERT(p_Itinerary);

	if (m_SelectedItem.y!=-1)
	{
		p_Itinerary->InsertFlights(m_SelectedItem.y);
		p_Itinerary->m_IsModified = TRUE;

		m_SelectionAnchor = -1;
		AdjustLayout();
	}
}

void CDataGrid::OnDelete()
{
	ASSERT(p_Itinerary);
	ASSERT(HasSelection(TRUE));

	INT Anfang;
	INT Ende;
	GetSelection(Anfang, Ende, TRUE);

	DoDelete(Anfang, Ende);
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
				p_Itinerary->m_Flights.AddItem(dlg.m_FlightTemplate);

				Added = TRUE;
			}

			if (DelimiterFound==L',')
				To.Empty();
		}

		if (Added)
		{
			AdjustLayout();
			SelectItem(CPoint(m_SelectedItem.x, p_Itinerary->m_Flights.m_ItemCount-1), FALSE);
			p_Itinerary->m_IsModified = TRUE;
		}
	}
}

void CDataGrid::OnSelectAll()
{
	ASSERT(p_Itinerary);

	m_SelectionAnchor = 0;
	m_SelectedItem.y = p_Itinerary->m_Flights.m_ItemCount;
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
		b &= HasSelection(TRUE);
		break;
	case IDM_EDIT_PASTE:
		{
			COleDataObject dobj;
			if (dobj.AttachClipboard())
				b = dobj.IsDataAvailable(theApp.CF_FLIGHTS);
		}
		break;
	case IDM_EDIT_SELECTALL:
		if (p_Itinerary)
			b = p_Itinerary->m_Flights.m_ItemCount;
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

void CDataGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
