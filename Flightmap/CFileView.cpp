
// CAttachmentList.cpp: Implementierung der Klasse CAttachmentList
//

#include "stdafx.h"
#include "CFileView.h"
#include "Flightmap.h"


// CAttachmentList
//

CString CAttachmentList::m_SubitemNames[FILEVIEWCOLUMNS];
UINT CAttachmentList::m_SortAttribute = 0;
BOOL CAttachmentList::m_SortDescending = FALSE;

CAttachmentList::CAttachmentList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM | FRONTSTAGE_ENABLELABELEDIT, sizeof(AttachmentItemData))
{
	p_Itinerary = NULL;

	// Subitem names
	if (m_SubitemNames[0].IsEmpty())
		for (UINT a=0; a<FILEVIEWCOLUMNS; a++)
			ENSURE(m_SubitemNames[a].LoadString(IDS_SUBITEM_NAME+a));

	// Item
	SetItemHeight(theApp.m_SystemImageListExtraLarge, 3);
}


// Header

void CAttachmentList::UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const
{
	HeaderItem.mask = HDI_WIDTH | HDI_FORMAT;
	HeaderItem.fmt = HDF_STRING | (Attr>0 ? HDF_RIGHT : HDF_LEFT);

	if (m_SortAttribute==Attr)
		HeaderItem.fmt |= m_SortDescending ? HDF_SORTDOWN : HDF_SORTUP;

	if ((HeaderItem.cxy=m_ColumnWidth[Attr])<ITEMVIEWMINWIDTH)
		HeaderItem.cxy = ITEMVIEWMINWIDTH;
}

void CAttachmentList::HeaderColumnClicked(UINT Attr)
{
	m_SortDescending = (m_SortAttribute==Attr) ? !m_SortDescending : (m_SortAttribute=Attr);

	UpdateHeader();
	SortItems();

	AdjustLayout();
}


// Menus

BOOL CAttachmentList::GetContextMenu(CMenu& Menu, INT Index)
{
	if ((Index>=0) || !HasHeader())
		Menu.LoadMenu(Index>=0 ? IDM_FILEVIEW_ITEM : IDM_FILEVIEW_BACKGROUND);

	return (Index>=0);
}


// Layouts

void CAttachmentList::AdjustLayout()
{
	if (HasHeader())
	{
		// Header
		m_ColumnWidth[0] = 0;
		m_ColumnWidth[3] = (m_ColumnWidth[1]=m_ColumnWidth[2]=theApp.m_DefaultFont.GetTextExtent(_T("00.00.0000 00:00ww")).cx+ITEMCELLSPACER)*2/3;

		SetFixedColumnWidths(m_ColumnOrder, m_ColumnWidth);

		UpdateHeader();

		// Item layout
		AdjustLayoutList();
	}
	else
	{
		// Item layout
		AdjustLayoutColumns(2, BACKSTAGEBORDER);
	}
}


// Item data

void CAttachmentList::AddAttachment(UINT Index, AIRX_Attachment& Attachment)
{
	// Icon
	if (Attachment.IconID==-1)
	{
		CString tmpStr(Attachment.Name);
		const INT Pos = tmpStr.ReverseFind(L'.');

		const CString Ext = (Pos==-1) ? _T("*") : tmpStr.Mid(Pos);

		SHFILEINFO ShellFileInfo;
		Attachment.IconID = SUCCEEDED(SHGetFileInfo(Ext, 0, &ShellFileInfo, sizeof(ShellFileInfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? ShellFileInfo.iIcon : 3;
	}

	// Item data
	AttachmentItemData Data;

	Data.Index = Index;
	Data.pAttachment = &Attachment;

	AddItem(&Data);
}

void CAttachmentList::SetAttachments(CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	ASSERT(pItinerary);

	DestroyEdit();
	HideTooltip();

	p_Itinerary = pItinerary;

	// Header
	if (!pFlight && !HasHeader())
		for (UINT a=0; a<FILEVIEWCOLUMNS; a++)
			AddHeaderColumn(TRUE, m_SubitemNames[a], a>0);

	// Items
	if (pFlight)
	{
		SetItemCount(AIRX_MAXATTACHMENTCOUNT, FALSE);

		for (UINT a=0; a<pFlight->AttachmentCount; a++)
			AddAttachment(pFlight->Attachments[a], pItinerary->m_Attachments[pFlight->Attachments[a]]);
	}
	else
	{
		SetItemHeight(GetSystemMetrics(SM_CYSMICON), 1, ITEMCELLPADDINGY);
		SetItemCount(pItinerary->m_Attachments.m_ItemCount, FALSE);

		for (UINT a=0; a<pItinerary->m_Attachments.m_ItemCount; a++)
			AddAttachment(a, pItinerary->m_Attachments[a]);
	}

	LastItem();
	SortItems();

	AdjustLayout();
}

AIRX_Attachment* CAttachmentList::GetSelectedAttachment() const
{
	const INT Index = GetSelectedItem();

	return (Index<0) ? NULL : GetAttachment(Index);
}

INT CAttachmentList::GetSelectedAttachmentIndex() const
{
	const INT Index = GetSelectedItem();

	return (Index<0) ? -1 : GetAttachmentIndex(Index);
}


// Item sort

INT CAttachmentList::CompareItems(AttachmentItemData* pData1, AttachmentItemData* pData2, const SortParameters& Parameters)
{
	const AIRX_Attachment* pAttachment1 = pData1->pAttachment;
	const AIRX_Attachment* pAttachment2 = pData2->pAttachment;

	INT Result = 0;

	switch (Parameters.Attr)
	{
	case 0:
		Result = wcscmp(pAttachment1->Name, pAttachment2->Name);
		break;

	case 1:
		Result = CompareFileTime(&pAttachment1->Created, &pAttachment2->Created);
		break;

	case 2:
		Result = CompareFileTime(&pAttachment1->Modified, &pAttachment2->Modified);
		break;

	case 3:
		Result = pAttachment1->FileSize-pAttachment2->FileSize;
		break;
	}

	return Parameters.Descending ? -Result : Result;
}


// Item handling

void CAttachmentList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	if (IsEditing())
		return;

	const AIRX_Attachment* pAttachment = GetAttachment(m_HoverItem);
	ASSERT(pAttachment->IconID!=-1);

	// Hint
	WCHAR strSize[256];
	StrFormatByteSize(pAttachment->FileSize, strSize, 256);

	CString Hint;
	Hint.Format(_T("%s: %s\n%s: %s\n%s: %s"),
		m_SubitemNames[1], (LPCWSTR)FMTimeToString(pAttachment->Created),
		m_SubitemNames[2], (LPCWSTR)FMTimeToString(pAttachment->Modified),
		m_SubitemNames[3], strSize);

	// Preview
	HBITMAP hBitmap = NULL;
	HICON hIcon =  NULL;

	Bitmap* pBitmap = CItinerary::DecodePictureAttachment(*pAttachment);
	if (pBitmap)
	{
		INT Width = pBitmap->GetWidth();
		INT Height = pBitmap->GetHeight();
		if ((Width<16) || (Height<16))
			goto UseIcon;

		// Resolution
		CString tmpStr;
		tmpStr.Format(IDS_RESOLUTION, Width, Height);

		Hint += tmpStr;

		// Scaling
		const DOUBLE ScaleX = 256.0/(DOUBLE)Width;
		const DOUBLE ScaleY = 256.0/(DOUBLE)Height;
		DOUBLE Scale = min(ScaleX, ScaleY);
		if (Scale>1.0)
			Scale = 1.0;

		Width = (INT)(Scale*(DOUBLE)Width);
		Height = (INT)(Scale*(DOUBLE)Height);

		// Create bitmap
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		BITMAPINFOHEADER bmi = { sizeof(bmi) };
		bmi.biWidth = Width;
		bmi.biHeight = Height;
		bmi.biPlanes = 1;
		bmi.biBitCount = 24;

		LPBYTE pbData = NULL;
		hBitmap = CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pbData, NULL, 0);
		HGDIOBJ hOldBitmap = dc.SelectObject(hBitmap);

		// Draw
		dc.FillSolidRect(0, 0, Width, Height, 0xFFFFFF);

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		g.DrawImage(pBitmap, -1, -1, Width+1, Height+1);

		dc.SelectObject(hOldBitmap);
	}
	else
	{
UseIcon:
		hIcon = theApp.m_SystemImageListExtraLarge.ExtractIcon(pAttachment->IconID);
	}

	// Show tooltip
	theApp.ShowTooltip(this, point, pAttachment->Name, Hint, hIcon, hBitmap);
}

COLORREF CAttachmentList::GetItemTextColor(INT Index) const
{
	const AIRX_Attachment* pAttachment = GetAttachment(Index);

	return (pAttachment->Flags & AIRX_INVALID) ? 0x2020FF : (pAttachment->Flags & AIRX_VALID) ? 0x208040 : (COLORREF)-1;
}


// Selected item commands

void CAttachmentList::FireSelectedItem() const
{
	GetOwner()->PostMessage(WM_COMMAND, (WPARAM)IDM_FILEVIEW_OPEN);
}


// Drawing

void CAttachmentList::DrawItemCell(CDC& dc, CRect& rectCell, INT Index, UINT Attr, BOOL /*Themed*/)
{
	const AIRX_Attachment* pAttachment = GetAttachment(Index);
	ASSERT(pAttachment->IconID!=-1);

	if (Attr==0)
	{
		// Icon
		theApp.m_SystemImageListSmall.DrawEx(&dc, pAttachment->IconID, CPoint(rectCell.left, rectCell.top+(rectCell.Height()-m_IconSize)/2), CSize(m_IconSize, m_IconSize), CLR_NONE, CLR_NONE, ILD_TRANSPARENT);

		rectCell.left += m_IconSize+ITEMCELLPADDINGX;

		// Label
		if (IsEditing() && (Index==m_EditItem))
			return;
	}

	// Column
	CString strCell;

	switch (Attr)
	{
	case 0:
		strCell = pAttachment->Name;
		break;

	case 1:
	case 2:
		strCell = FMTimeToString(Attr==1 ? pAttachment->Created : pAttachment->Modified);
		break;

	case 3:
		WCHAR strSize[256];
		StrFormatByteSize(pAttachment->FileSize, strSize, 256);

		strCell = strSize;
	}

	dc.DrawText(strCell, rectCell, (Attr>0 ? DT_RIGHT : DT_LEFT) | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
}

void CAttachmentList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	if (HasHeader())
	{
		// List view
		DrawListItem(dc, rectItem, Index, Themed, m_ColumnOrder, m_ColumnWidth);
	}
	else
	{
		// Tile view
		const AIRX_Attachment* pAttachment = GetAttachment(Index);
		ASSERT(pAttachment->IconID!=-1);

		WCHAR strSize[256];
		StrFormatByteSize(pAttachment->FileSize, strSize, 256);

		DrawTile(dc, rectItem, theApp.m_SystemImageListExtraLarge, pAttachment->IconID,
			ILD_TRANSPARENT,
			GetDarkTextColor(dc, Index, Themed), 3,
			pAttachment->Name, FMTimeToString(pAttachment->Modified), strSize);
	}
}


// Label edit

void CAttachmentList::DestroyEdit(BOOL Accept)
{
	if (IsEditing())
	{
		ASSERT(p_Itinerary);
		const INT EditItem = m_EditItem;

		// Set m_pWndEdit to NULL to avoid recursive calls when the edit window loses focus
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		// Get text
		CString Name;
		pVictim->GetWindowText(Name);

		// Destroy window; this will trigger another DestroyEdit() call!
		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && (EditItem>=0) && !Name.IsEmpty())
			p_Itinerary->RenameAttachment(GetAttachmentIndex(EditItem), Name);
	}

	CFrontstageItemView::DestroyEdit(Accept);
}

RECT CAttachmentList::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	if (HasHeader())
	{
		rect.bottom = (rect.top+=ITEMCELLPADDINGY-2)+m_DefaultFontHeight+4;
		rect.right = rect.left+m_ColumnWidth[0];
		rect.left += m_IconSize+2*ITEMCELLPADDINGX-6;
	}
	else
	{
		rect.bottom = (rect.top+=(rect.bottom-rect.top-3*m_DefaultFontHeight)/2-2)+m_DefaultFontHeight+4;
		rect.left += m_IconSize+2*ITEMVIEWPADDING-5;
		rect.right -= ITEMVIEWPADDING-2;
	}

	return rect;
}

void CAttachmentList::EditLabel(INT Index)
{
	m_EditItem = -1;

	if (IsLabelEditEnabled() && (Index>=0) && (Index<m_ItemCount))
	{
		HideTooltip();

		m_EditItem = Index;
		InvalidateItem(Index);
		EnsureVisible(Index);

		CString strLabel(GetAttachment(Index)->Name);

		const INT Pos = strLabel.ReverseFind(L'.');
		if (Pos!=-1)
			strLabel.Truncate(Pos);

		ASSERT(!m_pWndEdit);
		m_pWndEdit = new CEdit();
		m_pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, GetLabelRect(Index), this, 2);

		m_pWndEdit->SetWindowText(strLabel);
		m_pWndEdit->SetFont(&FMGetApp()->m_DefaultFont);
		m_pWndEdit->SetFocus();
		m_pWndEdit->SetSel(0, -1);
	}
}


BEGIN_MESSAGE_MAP(CAttachmentList, CFrontstageItemView)
	ON_WM_MOUSEHOVER()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CAttachmentList::OnMouseHover(UINT nFlags, CPoint point)
{
	if (!IsEditing() && (m_HoverItem==m_EditItem))
	{
		EditLabel(m_EditItem);
	}
	else
	{
		CFrontstageItemView::OnMouseHover(nFlags, point);
	}
}

void CAttachmentList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0);

	if (nChar==VK_F2)
	{
		if (Plain && (m_FocusItem>=0) && IsItemSelected(m_FocusItem))
			EditLabel(m_FocusItem);
	}
	else
	{
		CFrontstageItemView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


// CFileView
//

CIcons CFileView::m_LargeIcons;
CIcons CFileView::m_SmallIcons;

CFileView::CFileView()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = theApp.LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CFileView";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CFileView", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Itinerary and flight
	p_Itinerary = NULL;
	p_Flight = NULL;
}

void CFileView::PreSubclassWindow()
{
	CFrontstageWnd::PreSubclassWindow();

	// Taskbar
	m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_ATTACHMENTS_16, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_SAVEAS, 2);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	// Attachment list
	m_wndAttachmentList.Create(this, 2);

	AdjustLayout();
}

void CFileView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndAttachmentList.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::SetAttachments(CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;
	p_Flight = pFlight;

	Reload();
}

void CFileView::Reload()
{
	m_wndAttachmentList.SetAttachments(p_Itinerary, p_Flight);

	m_wndTaskbar.SendMessage(WM_IDLEUPDATECMDUI);
}


BEGIN_MESSAGE_MAP(CFileView, CFrontstageWnd)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()

	ON_NOTIFY(IVN_SELECTIONCHANGED, 2, OnSelectionChanged)

	ON_COMMAND(IDM_FILEVIEW_ADD, OnAdd)
	ON_COMMAND(IDM_FILEVIEW_OPEN, OnOpen)
	ON_COMMAND(IDM_FILEVIEW_SAVEAS, OnSaveAs)
	ON_COMMAND(IDM_FILEVIEW_DELETE, OnDelete)
	ON_COMMAND(IDM_FILEVIEW_RENAME, OnRename)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILEVIEW_ADD, IDM_FILEVIEW_RENAME, OnUpdateCommands)
END_MESSAGE_MAP()

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_wndAttachmentList.GetItemCount())
	{
		m_wndAttachmentList.SetFocus();
	}
	else
	{
		m_wndTaskbar.SetFocus();
	}
}


void CFileView::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_wndTaskbar.SendMessage(WM_IDLEUPDATECMDUI);

	*pResult = 0;
}


// File commands
//

void CFileView::OnAdd()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, NULL, this);
	if (dlg.DoModal()==IDOK)
	{
		POSITION Pos = dlg.GetStartPosition();
		while (Pos)
			if (!p_Itinerary->AddAttachment(*p_Flight, dlg.GetNextPathName(Pos)))
				break;

		Reload();
	}
}

void CFileView::OnOpen()
{
	const AIRX_Attachment* pAttachment = m_wndAttachmentList.GetSelectedAttachment();
	if (pAttachment)
	{
		CWaitCursor WaitCursor;

		// Dateinamen finden
		TCHAR Pathname[MAX_PATH];
		if (!GetTempPath(MAX_PATH, Pathname))
			return;

		CString szTempExt(pAttachment->Name);
		const INT Pos = szTempExt.ReverseFind(L'.');
		szTempExt.Delete(0, Pos!=-1 ? Pos : szTempExt.GetLength());

		CString szTempName;
		srand(rand());
		szTempName.Format(_T("%sFlightmap%.4X%.4X%s"), Pathname, 32768+rand(), 32768+rand(), szTempExt);

		// Datei erzeugen
		CFile File;
		if (!File.Open(szTempName, CFile::modeWrite | CFile::modeCreate))
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}
		else
		{
			try
			{
				File.Write(pAttachment->pData, pAttachment->FileSize);
				File.Close();

				ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOWNORMAL);
			}
			catch(CFileException ex)
			{
				File.Close();
				FMErrorBox(this, IDS_DRIVENOTREADY);
			}
		}
	}
}

void CFileView::OnSaveAs()
{
	const AIRX_Attachment* pAttachment = m_wndAttachmentList.GetSelectedAttachment();
	if (pAttachment)
	{
		CFileDialog dlg(FALSE, NULL, pAttachment->Name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
		if (dlg.DoModal()==IDOK)
		{
			CWaitCursor WaitCursor;

			CFile File;
			if (!File.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
			{
				FMErrorBox(this, IDS_DRIVENOTREADY);
			}
			else
			{
				try
				{
					File.Write(pAttachment->pData, pAttachment->FileSize);
				}
				catch(CFileException ex)
				{
					FMErrorBox(this, IDS_DRIVENOTREADY);
				}

				File.Close();
			}
		}
	}
}

void CFileView::OnDelete()
{
	const INT Index = m_wndAttachmentList.GetSelectedAttachmentIndex();
	if (Index!=-1)
		if (FMMessageBox(this, CString((LPCSTR)IDS_DELETE_FILE), m_wndAttachmentList.GetSelectedAttachment()->Name, MB_YESNO | MB_ICONWARNING)==IDYES)
		{
			p_Itinerary->DeleteAttachment(Index, p_Flight);

			Reload();
		}
}

void CFileView::OnRename()
{
	const INT Index = m_wndAttachmentList.GetSelectedItem();
	if (Index!=-1)
	{
		if (GetFocus()!=&m_wndAttachmentList)
			m_wndAttachmentList.SetFocus();

		m_wndAttachmentList.EditLabel(Index);
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	switch (pCmdUI->m_nID)
	{
	case IDM_FILEVIEW_ADD:
		if (p_Flight)
			bEnable = (p_Flight->AttachmentCount<AIRX_MAXATTACHMENTCOUNT);

		break;

	default:
		bEnable = (m_wndAttachmentList.GetSelectedItem()!=-1);
	}

	pCmdUI->Enable(bEnable);
}
