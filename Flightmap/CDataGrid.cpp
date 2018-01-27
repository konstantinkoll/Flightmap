
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
#define LEFTMARGIN         (BACKSTAGEBORDER-1)
#define MINCOLUMNWIDTH     50
#define MAXAUTOWIDTH       400

static const UINT DisplayFlags[] = { 0, AIRX_AwardFlight, AIRX_GroundTransportation, AIRX_BusinessTrip, AIRX_LeisureTrip, AIRX_Upgrade, AIRX_Cancelled };

CIcons CDataGrid::m_LargeIcons;
CIcons CDataGrid::m_SmallIcons;

CDataGrid::CDataGrid()
	: CFrontstageScroller(FRONTSTAGE_COMPLEXBACKGROUND)
{
	p_Itinerary = NULL;
	m_pWndEdit = NULL;
	m_HoverPart = m_SelectionAnchor = -1;
	m_ViewParameters = theApp.m_ViewParameters;

	m_FindReplaceSettings = theApp.m_FindReplaceSettings;
	m_FindReplaceSettings.FirstAction = FALSE;
}

BOOL CDataGrid::Create(CItinerary* pItinerary, CWnd* pParentWnd, UINT nID)
{
	ASSERT(pItinerary);

	m_FocusItem.x = 0;
	m_FocusItem.y = (p_Itinerary=pItinerary)->m_Metadata.CurrentRow;

	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	return CFrontstageScroller::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CDataGrid::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_pWndEdit)
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
		if (m_pWndEdit)
			return TRUE;

		break;
	}

	return CFrontstageScroller::PreTranslateMessage(pMsg);
}

void CDataGrid::SetItinerary(CItinerary* pItinerary)
{
	ASSERT(pItinerary);

	if (p_Itinerary!=pItinerary)
	{
		m_FocusItem.x = 0;
		m_FocusItem.y = (p_Itinerary=pItinerary)->m_Metadata.CurrentRow;
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
	return (Index<p_Itinerary->m_Flights.m_ItemCount) && (p_Itinerary->m_Flights[Index].Flags & AIRX_Selected);
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
			if (m_FocusItem.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				WCHAR tmpBuf[256];
				AttributeToString(p_Itinerary->m_Flights[m_FocusItem.y], m_ViewParameters.ColumnOrder[m_FocusItem.x], tmpBuf, 256);

				Text = tmpBuf;
			}

		EmptyClipboard();

		// CF_UNICODETEXT
		SIZE_T Size = (Text.GetLength()+1)*sizeof(WCHAR);
		HGLOBAL hClipBuffer;

		if ((hClipBuffer=GlobalAlloc(GMEM_MOVEABLE, Size))!=NULL)
		{
			LPWSTR pBuffer = (LPWSTR)GlobalLock(hClipBuffer);
			wcscpy_s((LPWSTR)pBuffer, Size/sizeof(WCHAR), Text);

			GlobalUnlock(hClipBuffer);

			SetClipboardData(CF_UNICODETEXT, hClipBuffer);
		}

		// CF_FLIGHTS
		if (Count)
		{
			Size = Count*sizeof(AIRX_Flight);

			if ((hClipBuffer=GlobalAlloc(GMEM_MOVEABLE, Size))!=NULL)
			{
				AIRX_Flight* pFlight = (AIRX_Flight*)GlobalLock(hClipBuffer);

				for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
					if (IsSelected(a))
						*(pFlight++) = p_Itinerary->m_Flights[a];

				GlobalUnlock(hClipBuffer);

				SetClipboardData(theApp.CF_FLIGHTS, hClipBuffer);
			}
		}

		CloseClipboard();

		if (Cut)
			DoDelete();
	}
}

CRect CDataGrid::GetItemRect(const CPoint& Item, BOOL Inflate) const
{
	INT x = -m_HScrollPos+LEFTMARGIN;
	for (INT Col=0; Col<Item.x; Col++)
		x += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]];

	CRect rect;
	rect.right = (rect.left=x)+m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Item.x]]-1;
	rect.bottom = (rect.top=(Item.y*m_ItemHeight-m_VScrollPos+(INT)m_HeaderHeight))+m_ItemHeight-1;

	if (Inflate)
		rect.InflateRect(2, 2);

	return rect;
}

void CDataGrid::EnsureVisible(const CPoint& Item)
{
	ASSERT(IsPointValid(Item));
	ASSERT(Item.x<FMAttributeCount);
	ASSERT(Item.y<=(INT)(p_Itinerary->m_Flights.m_ItemCount));

	CFrontstageScroller::EnsureVisible(GetItemRect(Item, FALSE));
}

void CDataGrid::EditCell(BOOL AllowCursor, BOOL Delete, WCHAR PushChar)
{
	ASSERT(IsPointValid(m_FocusItem));
	ASSERT(m_FocusItem.y<=(INT)(p_Itinerary->m_Flights.m_ItemCount));

	const CPoint Item = m_FocusItem;

	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	if (!FMAttributes[Attr].Editable)
		return;

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);
	const LPVOID pData = NewLine ? NULL : (((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset);
	Delete |= NewLine;

	// Special cell types
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
					AdjustLayout();
				}

				*((COLORREF*)(((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset)) = clr;

				p_Itinerary->m_IsModified = TRUE;
				InvalidatePoint(Item);
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
		InvalidatePoint(Item);

		return;

	case FMTypeRating:
		if (pData && (PushChar>=L'0') && (PushChar<=L'5'))
		{
			*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
			*((DWORD*)pData) |= (((PushChar-'0')*2)<<FMAttributes[Attr].DataParameter);

			p_Itinerary->m_IsModified = TRUE;
			InvalidatePoint(Item);
		}

		return;
	}

	// Create edit control
	WCHAR tmpBuf[256];
	if (PushChar)
	{
		tmpBuf[0] = PushChar;
		tmpBuf[1] = L'\0';
	}
	else
	{
		tmpBuf[0] = L'\0';

		if (!Delete)
			AttributeToString(p_Itinerary->m_Flights[Item.y], Attr, tmpBuf, 256);
	}

	m_pWndEdit = new CMFCMaskedEdit();
	m_pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, GetItemRect(Item), this, 2);

	PrepareEditCtrl(m_pWndEdit, Attr);
	m_EditAllowCursor = AllowCursor;

	m_pWndEdit->SetWindowText(tmpBuf);
	m_pWndEdit->SetFont(&theApp.m_DefaultFont);
	m_pWndEdit->SetFocus();
	m_pWndEdit->SetSel(PushChar ? -1 : 0, PushChar ? 0 : -1);
}

void CDataGrid::EditFlight(const CPoint& Item, INT SelectTab)
{
	ASSERT(IsPointValid(Item));
	ASSERT(Item.y<=(INT)(p_Itinerary->m_Flights.m_ItemCount));

	EnsureVisible(Item);
	const BOOL NewLine = (Item.y>=(INT)p_Itinerary->m_Flights.m_ItemCount);

	EditFlightDlg dlg(NewLine ? NULL : &p_Itinerary->m_Flights[Item.y], this, p_Itinerary, SelectTab);
	if (dlg.DoModal()==IDOK)
	{
		if (NewLine)
		{
			p_Itinerary->AddFlight();
			AdjustLayout();
		}
		else
		{
			Invalidate();
		}

		p_Itinerary->m_Flights[Item.y] = dlg.m_Flight;
		p_Itinerary->UpdateFlight(Item.y);

		p_Itinerary->m_IsModified = TRUE;
	}
	else
		if (NewLine)
		{
			p_Itinerary->DeleteAttachments(dlg.m_Flight);
		}
		else
		{
			p_Itinerary->m_Flights[Item.y].AttachmentCount = dlg.m_Flight.AttachmentCount;
			memcpy(p_Itinerary->m_Flights[Item.y].Attachments, dlg.m_Flight.Attachments, AIRX_MaxAttachmentCount*sizeof(UINT));
		}
}

CPoint CDataGrid::PointAtPosition(CPoint point) const
{
	point.y -= m_HeaderHeight-m_VScrollPos;

	const INT Row = (point.y>=0) ? point.y/m_ItemHeight : -1;
	if ((Row!=-1) && (Row<=(INT)p_Itinerary->m_Flights.m_ItemCount))
	{
		INT x = -m_HScrollPos+LEFTMARGIN;

		for (UINT Col=0; Col<FMAttributeCount; Col++)
		{
			const UINT Attr = m_ViewParameters.ColumnOrder[Col];
			if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[Attr]))
				return CPoint(Col, Row);

			x += m_ViewParameters.ColumnWidth[Attr];
		}
	}

	return CPoint(-1, -1);
}

INT CDataGrid::PartAtPosition(const CPoint& point) const
{
	INT x = -m_HScrollPos+LEFTMARGIN;

	for (UINT Col=0; Col<FMAttributeCount; Col++)
	{
		const UINT Attr = m_ViewParameters.ColumnOrder[Col];
		if ((point.x>=x) && (point.x<x+m_ViewParameters.ColumnWidth[Attr]))
		{
			if ((x=point.x-x-ITEMCELLPADDINGX)>0)
				switch (FMAttributes[Attr].Type)
				{
				case FMTypeFlags:
					return ((x<18*FLAGCOUNT) && (x%18<16)) ? x/18 : -1;

				case FMTypeRating:
					return (x<RATINGBITMAPWIDTH+6) ? (x<6) ? 0 : (x%18<16) ? 2*(x/18)+(x%18>8)+1 : -1 : -1;
				}

			return -1;
		}

		x += m_ViewParameters.ColumnWidth[Attr];
	}

	return -1;
}

void CDataGrid::InvalidatePoint(const CPoint& point)
{
	if (IsPointValid(point))
		InvalidateRect(GetItemRect(point));
}

void CDataGrid::InvalidateRow(UINT Row)
{
	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectItem = GetItemRect(CPoint(0, Row));

	InvalidateRect(CRect(rectClient.left, rectItem.top, rectClient.right, rectItem.bottom));
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

	Invalidate();
	EnsureVisible(m_FocusItem=FocusItem);
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
	rectItem.left += ITEMCELLPADDINGX;
	rectItem.right -= ITEMCELLPADDINGX+1;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeColor:
		if (*((COLORREF*)(((LPBYTE)&Flight)+FMAttributes[Attr].Offset))!=(COLORREF)-1)
		{
			rectItem.InflateRect(ITEMCELLPADDINGX-ITEMCELLPADDINGY, 0);
			rectItem.DeflateRect(0, ITEMCELLPADDINGY);
			dc.Draw3dRect(rectItem, 0x000000, 0x000000);

			rectItem.DeflateRect(1, 1);
			dc.FillSolidRect(rectItem, Flight.Color);
		}

		break;

	case FMTypeFlags:
		rectItem.top += (rectItem.Height()-m_SmallIcons.GetIconSize())/2;

		for (UINT a=0; a<FLAGCOUNT; a++)
		{
			const BOOL Enabled = a ? (Flight.Flags & DisplayFlags[a]) : Flight.AttachmentCount;
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
		WCHAR tmpStr[256];
		AttributeToString(p_Itinerary->m_Flights[Row], Attr, tmpStr, 256);

		const INT cx = dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr)).cx+ITEMCELLSPACER;
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

void CDataGrid::FinishEdit(LPCWSTR pStr, const CPoint& Item)
{
	const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
	StringToAttribute(pStr, p_Itinerary->m_Flights[Item.y], Attr);

	p_Itinerary->m_IsModified = TRUE;
	InvalidatePoint(Item);

	switch (Attr)
	{
	case 0:
	case 1:
	case 3:
		p_Itinerary->UpdateFlight(Item.y);

	case 20:
		InvalidateRow(Item.y);
		break;
	}
}

void CDataGrid::DestroyEdit(BOOL Accept)
{
	if (m_pWndEdit)
	{
		CPoint EditItem = m_FocusItem;

		// Set m_pWndEdit to NULL to avoid recursive calls when the edit window loses focus
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		// Get text
		WCHAR tmpBuf[256];
		pVictim->GetWindowText(tmpBuf, 256);

		// Destroy window; this will trigger another DestroyEdit() call!
		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && IsPointValid(EditItem))
		{
			if (EditItem.y>=(INT)p_Itinerary->m_Flights.m_ItemCount)
			{
				if (tmpBuf[0]==L'\0')
					return;

				p_Itinerary->AddFlight();
				EditItem.y = p_Itinerary->m_Flights.m_ItemCount-1;

				AdjustLayout();
			}

			FinishEdit(tmpBuf, EditItem);
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

void CDataGrid::ShowTooltip(const CPoint& point)
{
	if (!m_pWndEdit && (m_HoverPoint.y<(INT)p_Itinerary->m_Flights.m_ItemCount))
	{
		const AIRX_Flight* pFlight = &p_Itinerary->m_Flights[m_HoverPoint.y];
		const UINT Attr = m_ViewParameters.ColumnOrder[m_HoverPoint.x];
	
		WCHAR tmpStr[256];
		switch (Attr)
		{
		case 0:
			if (strlen(pFlight->From.Code)==3)
				theApp.ShowTooltip(this, point, pFlight->From.Code, _T(""));

			break;

		case 3:
			if (strlen(pFlight->To.Code)==3)
				theApp.ShowTooltip(this, point, pFlight->To.Code, _T(""));

			break;

		default:
			switch (FMAttributes[Attr].Type)
			{
			case FMTypeFlags:
				if (m_HoverPart!=-1)
				{
					CString Caption((LPCSTR)IDS_ATTACHMENTS+m_HoverPart);
					CString Hint;

					const INT Pos = Caption.Find(L'\n');
					if (Pos!=-1)
					{
						Hint = Caption.Mid(Pos+1);
						Caption = Caption.Left(Pos);
					}

					theApp.ShowTooltip(this, point, Caption, Hint, m_LargeIcons.ExtractIcon(m_HoverPart));
				}

			case FMTypeUINT:
			case FMTypeDistance:
			case FMTypeDateTime:
			case FMTypeTime:
				if (IsSelected(m_HoverPoint.y))
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
								F = *((LPFILETIME)pData);
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
				AttributeToString(p_Itinerary->m_Flights[m_HoverPoint.y], Attr, tmpStr, 256);

				if (tmpStr[0]!=L'\0')
				{
					CSize szText = theApp.m_DefaultFont.GetTextExtent(tmpStr);

					if ((szText.cx>m_ViewParameters.ColumnWidth[Attr]-ITEMCELLSPACER) || (FMAttributes[Attr].Type==FMTypeColor))
						theApp.ShowTooltip(this, point, _T(""), tmpStr);
				}
			}
		}
	}
}

BOOL CDataGrid::GetContextMenu(CMenu& Menu, const CPoint& /*point*/)
{
	Menu.LoadMenu(IDM_DATAGRID);

	return FALSE;
}

void CDataGrid::GetHeaderContextMenu(CMenu& Menu)
{
	Menu.LoadMenu(IDM_DETAILS);
}

BOOL CDataGrid::AllowHeaderColumnDrag(UINT /*Attr*/) const
{
	return TRUE;
}

BOOL CDataGrid::AllowHeaderColumnTrack(UINT Attr) const
{
	return
		(FMAttributes[Attr].Type!=FMTypeRating) &&			// Variable width
		(FMAttributes[Attr].Type!=FMTypeColor) &&
		(FMAttributes[Attr].Type!=FMTypeFlags);
}

void CDataGrid::UpdateHeaderColumnOrder(UINT Attr, INT Position)
{
	CFrontstageScroller::UpdateHeaderColumnOrder(Attr, Position, theApp.m_ViewParameters.ColumnOrder, theApp.m_ViewParameters.ColumnWidth);

	UpdateHeader();
	AdjustLayout();
}

void CDataGrid::UpdateHeaderColumnWidth(UINT Attr, INT Width)
{
	if (Width<MINCOLUMNWIDTH)
		Width = ((Attr==0) || (Attr==3)) ? MINCOLUMNWIDTH : 0;

	if (Width!=theApp.m_ViewParameters.ColumnWidth[Attr])
	{
		theApp.m_ViewParameters.ColumnWidth[Attr] = Width;

		UpdateHeader();
		AdjustLayout();
	}
}

void CDataGrid::UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const
{
	HeaderItem.mask = HDI_WIDTH | HDI_FORMAT;
	HeaderItem.cxy = theApp.m_ViewParameters.ColumnWidth[Attr];
	HeaderItem.fmt = HDF_STRING | HDF_CENTER;

	if (HeaderItem.cxy)
		if ((FMAttributes[Attr].Type==FMTypeRating) || (FMAttributes[Attr].Type==FMTypeColor) || (FMAttributes[Attr].Type==FMTypeFlags))
		{
			HeaderItem.cxy = FMAttributes[Attr].DefaultColumnWidth;
		}
		else
		{
			if (HeaderItem.cxy<MINCOLUMNWIDTH)
				HeaderItem.cxy = MINCOLUMNWIDTH;
		}
}

void CDataGrid::UpdateHeader()
{
	m_ViewParameters = theApp.m_ViewParameters;

	CFrontstageScroller::UpdateHeader(m_ViewParameters.ColumnOrder, m_ViewParameters.ColumnWidth);
}

void CDataGrid::AdjustLayout()
{
	m_ScrollHeight = (p_Itinerary->m_Flights.m_ItemCount+1)*m_ItemHeight-1;
	m_ScrollWidth = LEFTMARGIN-1;

	for (UINT a=0; a<FMAttributeCount; a++)
		m_ScrollWidth += m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[a]];

	if (m_FocusItem.y>(INT)p_Itinerary->m_Flights.m_ItemCount)
		m_FocusItem.y = p_Itinerary->m_Flights.m_ItemCount;

	CFrontstageScroller::AdjustLayout();
}

void CDataGrid::DrawStage(CDC& dc, Graphics& /*g*/, const CRect& rect, const CRect& rectUpdate, BOOL Themed)
{
	const COLORREF clrLines = Themed ? 0xEAE9E8 : GetSysColor(COLOR_3DFACE);
	const COLORREF clrText = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);
	const COLORREF clrDisabled = Themed ? 0xA0A0A0 : GetSysColor(COLOR_GRAYTEXT);

	// Cells
	INT Start = m_VScrollPos/m_ItemHeight;
	INT y = m_HeaderHeight-(m_VScrollPos % m_ItemHeight);
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
				CRect rectItem(x, y, x+Width, y+m_ItemHeight-1);
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
						dc.SetTextColor(Selected ? Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT) : ((Attr==0) && (Flags & AIRX_UnknownFrom)) || ((Attr==3) && (Flags & AIRX_UnknownTo)) ? 0x2020FF : (Flags & AIRX_Cancelled) ? clrDisabled : (Flags & AIRX_FutureFlight) ? 0x008000 : clrText);

						DrawCell(dc, p_Itinerary->m_Flights[Row], Attr, rectItem, Selected);
					}
				}

				x += Width;
			}
		}

		// Horizontal lines
		y += m_ItemHeight;
		dc.FillSolidRect(-m_HScrollPos+LEFTMARGIN, y-1, x-(-m_HScrollPos+LEFTMARGIN), 1, clrLines);

		if (y>rect.Height())
			break;
	}

	// Vertical lines
	INT x = -m_HScrollPos+LEFTMARGIN;
	dc.FillSolidRect(x, 0, 1, y-1, clrLines);

	for (UINT Col=0; Col<FMAttributeCount; Col++)
	{
		const INT Width = m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]];
		if (Width)
		{
			x += Width;
			dc.FillSolidRect(x-1, 0, 1, y-1, clrLines);
		}
	}

	// Focus cell
	if (IsPointValid(m_FocusItem))
	{
		CRect rectItem = GetItemRect(m_FocusItem);

		for (UINT a=0; a<3; a++)
		{
			dc.Draw3dRect(rectItem, clrText, clrText);
			rectItem.DeflateRect(1, 1);
		}
	}
}


BEGIN_MESSAGE_MAP(CDataGrid, CFrontstageScroller)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()

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

	ON_EN_KILLFOCUS(2, OnDestroyEdit)
END_MESSAGE_MAP()

INT CDataGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageScroller::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Header
	for (UINT a=0; a<FMAttributeCount; a++)
		VERIFY(AddHeaderColumn(TRUE, FMAttributes[a].nNameID));

	UpdateHeader();

	// Time
	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);

	m_wDay = stLocal.wDay;

	// Items
	m_LargeIcons.Load(IDB_FLAGS_16, LI_FORTOOLTIPS);
	m_SmallIcons.Load(IDB_FLAGS_16, LI_NORMAL, &theApp.m_DefaultFont);

	SetItemHeight((max(m_SmallIcons.GetIconSize(), theApp.m_DefaultFont.GetFontHeight())+2*ITEMCELLPADDINGY+1) & ~1);
	m_szScrollStep.cy = m_ItemHeight;

	// Timer
	SetTimer(1, 1000, NULL);

	return 0;
}

void CDataGrid::OnDestroy()
{
	DestroyEdit();

	KillTimer(1);

	CFrontstageScroller::OnDestroy();
}

void CDataGrid::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
	{
		SYSTEMTIME stLocal;
		GetLocalTime(&stLocal);

		if (stLocal.wDay!=m_wDay)
		{
			m_wDay = stLocal.wDay;

			for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
				p_Itinerary->UpdateFlight(a, &stLocal);

			Invalidate();
		}
	}

	CFrontstageScroller::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void CDataGrid::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrontstageScroller::OnMouseMove(nFlags, point);

	const INT Part = IsPointValid(m_HoverPoint) ? PartAtPosition(point) : -1;
	if (Part!=m_HoverPart)
	{
		m_HoverPart = Part;
		HideTooltip();
	}

	SetCursor(theApp.LoadStandardCursor(m_HoverPart==-1 ? IDC_ARROW : IDC_HAND));
}

void CDataGrid::OnMouseLeave()
{
	m_HoverPart = -1;

	CFrontstageScroller::OnMouseLeave();
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
		Item.y -= (rect.Height()-m_HeaderHeight)/(INT)m_ItemHeight;
		if (Item.y<0)
			Item.y = 0;

		break;

	case VK_DOWN:
		if (Item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
			Item.y++;

		break;

	case VK_NEXT:
		Item.y += (rect.Height()-m_HeaderHeight)/(INT)m_ItemHeight;
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
		CFrontstageScroller::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);
}

void CDataGrid::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0);

	if (Plain && (nChar>=32u) && !m_pWndEdit)
		EditCell(FALSE, FALSE, (WCHAR)nChar);
}

void CDataGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	DestroyEdit(TRUE);

	const CPoint Item = PointAtPosition(point);
	if (IsPointValid(Item))
		if (nFlags & MK_CONTROL)
		{
			InvalidatePoint(m_FocusItem);
			m_FocusItem = Item;
			SelectItem(Item.y, !IsSelected(Item.y));
		}
		else
			if (Item==m_FocusItem)
			{
				if (Item.y<(INT)p_Itinerary->m_Flights.m_ItemCount)
				{
					const UINT Attr = m_ViewParameters.ColumnOrder[Item.x];
					const UINT Part = PartAtPosition(point);
					const LPVOID pData = (((LPBYTE)&p_Itinerary->m_Flights[Item.y])+FMAttributes[Attr].Offset);

					switch (FMAttributes[Attr].Type)
					{
					case FMTypeFlags:
						if (Part==0)
						{
							EditFlight(Item, 2);
						}
						else
							if (Part!=-1)
							{
								*((DWORD*)pData) ^= DisplayFlags[Part];
								p_Itinerary->m_IsModified = TRUE;

								if (DisplayFlags[Part]==AIRX_Cancelled)
								{
									InvalidateRow(Item.y);
								}
								else
								{
									InvalidatePoint(Item);
								}
							}

						break;

					case FMTypeRating:
						if (Part!=-1)
						{
							*((DWORD*)pData) &= ~(15<<FMAttributes[Attr].DataParameter);
							*((DWORD*)pData) |= (Part<<FMAttributes[Attr].DataParameter);

							p_Itinerary->m_IsModified = TRUE;
							InvalidatePoint(Item);
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

	CFrontstageScroller::OnLButtonDown(nFlags, point);
}

void CDataGrid::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsPointValid(PointAtPosition(point)))
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
	const CPoint Item = PointAtPosition(point);
	if (IsPointValid(Item))
		EditFlight(Item);
}

void CDataGrid::OnRButtonDown(UINT nFlags, CPoint point)
{
	const CPoint Item = PointAtPosition(point);
	if (IsPointValid(Item))
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
				InvalidatePoint(m_FocusItem);
				EnsureVisible(m_FocusItem=Item);
				InvalidatePoint(m_FocusItem);
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
	const CPoint Item = PointAtPosition(point);
	if (IsPointValid(Item))
	{
		if (GetFocus()!=this)
			SetFocus();

		if (!IsSelected(Item.y))
		{
			EnsureVisible(m_FocusItem=Item);

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

	CFrontstageScroller::OnRButtonUp(nFlags, point);
}

BOOL CDataGrid::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	SetCursor(theApp.LoadStandardCursor(m_HoverPart==-1 ? IDC_ARROW : IDC_HAND));

	return TRUE;
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

			FinishEdit((LPWSTR)pBuffer, m_FocusItem);

			GlobalUnlock(hClipBuffer);
		}

Finish:
		CloseClipboard();
	}
}

void CDataGrid::OnInsertRow()
{
	if (IsPointValid(m_FocusItem))
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
	EditFlight(m_FocusItem);
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

			if (!From.IsEmpty() && !To.IsEmpty())
			{
				StringToAttribute(From, dlg.m_FlightTemplate, 0);
				StringToAttribute(To, dlg.m_FlightTemplate, 3);

				p_Itinerary->AddFlight(dlg.m_FlightTemplate);

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
	// If not called from dialog, go to next cell
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
				FMMessageBox(this, CString((LPCSTR)(Replaced ? IDS_ALLREPLACED : IDS_SEARCHTERMNOTFOUND)), CString((LPCSTR)IDS_FINDREPLACE), Replaced ? MB_OK : MB_ICONEXCLAMATION | MB_OK);

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

		InvalidatePoint(Item);
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

			if (dlg.m_Filter.Carrier[0] && wcscmp(dlg.m_Filter.Carrier, pFlight->Carrier))
				continue;

			if (dlg.m_Filter.Equipment[0] && wcscmp(dlg.m_Filter.Equipment, pFlight->Equipment))
				continue;

			if (dlg.m_Filter.Business && ((pFlight->Flags & AIRX_BusinessTrip)==0))
				continue;

			if (dlg.m_Filter.Leisure && ((pFlight->Flags & AIRX_LeisureTrip)==0))
				continue;

			if (pFlight->Flags>>28<dlg.m_Filter.Rating)
				continue;

			if (dlg.m_Filter.DepartureMonth | dlg.m_Filter.DepartureYear)
			{
				SYSTEMTIME SystemTime;
				FileTimeToSystemTime(&pFlight->From.Time, &SystemTime);

				if (dlg.m_Filter.DepartureMonth && (dlg.m_Filter.DepartureMonth!=SystemTime.wMonth))
					continue;

				if (dlg.m_Filter.DepartureYear && (dlg.m_Filter.DepartureYear!=SystemTime.wYear))
					continue;
			}

			pItinerary->AddFlight(p_Itinerary, a);
		}

		if (dlg.m_Filter.SortBy>=0)
			pItinerary->SortItems((UINT)dlg.m_Filter.SortBy, dlg.m_Filter.Descending);

		(new CMainWnd())->Create(pItinerary);
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
			COleDataObject DataObject;
			if (DataObject.AttachClipboard())
				bEnable &= DataObject.IsDataAvailable(theApp.CF_FLIGHTS) || DataObject.IsDataAvailable(CF_UNICODETEXT);
		}

		break;

	case IDM_DATAGRID_FINDREPLACEAGAIN:
		bEnable = (m_FindReplaceSettings.SearchTerm[0]!=L'\0');

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

	UpdateHeader();
	AdjustLayout();
}

void CDataGrid::OnAutosize()
{
	if (m_HeaderItemClicked!=-1)
	{
		AutosizeColumn(m_HeaderItemClicked);

		UpdateHeader();
		AdjustLayout();
	}
}

void CDataGrid::OnChooseDetails()
{
	if (ChooseDetailsDlg(&m_ViewParameters, this).DoModal()==IDOK)
	{
		theApp.m_ViewParameters = m_ViewParameters;

		for (INT Col=m_FocusItem.x; Col>=0; Col--)
			if (m_ViewParameters.ColumnWidth[m_ViewParameters.ColumnOrder[Col]])
			{
				m_FocusItem.x = Col;
				break;
			}

		UpdateHeader();
		AdjustLayout();
	}
}

void CDataGrid::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (pCmdUI->m_nID!=IDM_DETAILS_AUTOSIZE);

	if ((pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE) && (m_HeaderItemClicked!=-1))
		bEnable = (FMAttributes[m_HeaderItemClicked].Type!=FMTypeRating) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeColor) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags) && (FMAttributes[m_HeaderItemClicked].Type!=FMTypeFlags);

	pCmdUI->Enable(bEnable);
}



// Header notifications

void CDataGrid::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnDestroyEdit();

	CFrontstageScroller::OnBeginDrag(pNMHDR, pResult);
}

void CDataGrid::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnDestroyEdit();

	CFrontstageScroller::OnBeginTrack(pNMHDR, pResult);
}


// Label edit

void CDataGrid::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}
