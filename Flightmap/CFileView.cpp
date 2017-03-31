
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"
#include "Flightmap.h"


// CFileView
//

#define GetSelectedFile()           m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)
#define GetSelectedAttachment()     GetAttachment(GetSelectedFile())

CIcons CFileView::m_LargeIcons;
CIcons CFileView::m_SmallIcons;

CFileView::CFileView()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CFileView";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CFileView", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Itinerary = NULL;
	p_Flight = NULL;
	m_pSortArray = NULL;
	m_LastSortColumn = m_Count = 0;
	m_LastSortDirection = FALSE;
}

void CFileView::PreSubclassWindow()
{
	CFrontstageWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CFileView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const INT Indent = (m_wndExplorerList.GetView()!=LV_VIEW_TILE) ? BACKSTAGEBORDER-1 : 0;
	m_wndExplorerList.SetWindowPos(NULL, rect.left+Indent, rect.top+TaskHeight, rect.Width()-Indent, rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::SetItinerary(CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	p_Itinerary = pItinerary;
	p_Flight = pFlight;

	m_wndExplorerList.SetView(p_Flight ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	m_wndExplorerList.SetItemsPerRow(2, 3);
	Reload();

	for (UINT a=0; a<4; a++)
		m_wndExplorerList.SetColumnWidth(a, m_Count==0 ? 130 : a<3 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

void CFileView::Reload()
{
	m_wndExplorerList.SetRedraw(FALSE);
	m_wndExplorerList.SetItemCount(0);

	m_Count = p_Flight ? p_Flight->AttachmentCount : p_Itinerary->m_Attachments.m_ItemCount;

	if (m_pSortArray)
		delete[] m_pSortArray;

	m_pSortArray = new UINT[m_Count];
	for (UINT a=0; a<m_Count; a++)
		m_pSortArray[a] = a;

	Sort();

	m_wndExplorerList.SetItemCount(m_Count);

	if (m_wndExplorerList.GetNextItem(-1, LVIS_FOCUSED)==-1)
		m_wndExplorerList.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	m_wndExplorerList.SetRedraw(TRUE);
	m_wndExplorerList.Invalidate();

	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);
}

AIRX_Attachment* CFileView::GetAttachment(INT Index)
{
	return Index==-1 ? NULL : p_Flight ? &p_Itinerary->m_Attachments[p_Flight->Attachments[m_pSortArray[Index]]] : &p_Itinerary->m_Attachments[m_pSortArray[Index]];
}

void CFileView::Init()
{
	// Taskbar
	m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_ATTACHMENTS_16, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_SAVEAS, 2);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	// ExplorerList
	m_wndExplorerList.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_OWNERDATA | LVS_EDITLABELS | LVS_SHAREIMAGELISTS, CRect(0, 0, 0, 0), this, 2);

	m_wndExplorerList.SetImageList(&theApp.m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&theApp.m_SystemImageListLarge, LVSIL_NORMAL);

	for (UINT a=0; a<4; a++)
		m_wndExplorerList.AddColumn(a, CString((LPCSTR)IDS_SUBITEM_NAME+a), 100, a);

	m_wndExplorerList.SetMenus(IDM_FILEVIEW_ITEM, TRUE, IDM_FILEVIEW_BACKGROUND);
	m_wndExplorerList.SetFocus();

	// Header
	CHeaderCtrl* pHeaderCtrl = m_wndExplorerList.GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		m_wndHeader.SubclassWindow(pHeaderCtrl->GetSafeHwnd());
		m_wndHeader.SetShadow(TRUE);
	}

	AdjustLayout();
}

INT CFileView::Compare(INT n1, INT n2)
{
	INT Result = 0;

	AIRX_Attachment* pFirst = GetAttachment(n1);
	AIRX_Attachment* pSecond = GetAttachment(n2);

	switch (m_LastSortColumn)
	{
	case 0:
		Result = wcscmp(pFirst->Name, pSecond->Name);
		break;

	case 1:
		Result = CompareFileTime(&pFirst->Created, &pSecond->Created);
		break;

	case 2:
		Result = CompareFileTime(&pFirst->Modified, &pSecond->Modified);
		break;

	case 3:
		Result = pFirst->Size-pSecond->Size;
		break;
	}

	if (m_LastSortDirection)
		Result = -Result;

	return Result;
}

void CFileView::Heap(INT Wurzel, INT Anzahl)
{
	while (Wurzel<=Anzahl/2-1)
	{
		INT Index = (Wurzel+1)*2-1;
		if (Index+1<Anzahl)
			if (Compare(Index, Index+1)<0)
				Index++;

		if (Compare(Wurzel, Index)<0)
		{
			Swap(m_pSortArray[Wurzel], m_pSortArray[Index]);
			Wurzel = Index;
		}
		else
		{
			break;
		}
	}
}

void CFileView::Sort()
{
	if (m_Count>1)
	{
		for (INT a=m_Count/2-1; a>=0; a--)
			Heap(a, m_Count);

		for (INT a=m_Count-1; a>0; a--)
		{
			Swap(m_pSortArray[0], m_pSortArray[a]);
			Heap(0, a);
		}
	}

	CHeaderCtrl* pHeaderCtrl = m_wndExplorerList.GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		HDITEM Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = HDI_FORMAT;

		for (INT a=0; a<4; a++)
		{
			pHeaderCtrl->GetItem(a, &Item);

			Item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			if (a==(INT)m_LastSortColumn)
				Item.fmt |= m_LastSortDirection ? HDF_SORTDOWN : HDF_SORTUP;

			pHeaderCtrl->SetItem(a, &Item);
		}
	}
}


BEGIN_MESSAGE_MAP(CFileView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, 2, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, 2, OnItemChanged)
	ON_NOTIFY(LVN_BEGINLABELEDIT, 2, OnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, 2, OnEndLabelEdit)
	ON_NOTIFY(REQUEST_TEXTCOLOR, 2, OnRequestTextColor)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 2, OnRequestTooltipData)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortItems)

	ON_COMMAND(IDM_FILEVIEW_ADD, OnAdd)
	ON_COMMAND(IDM_FILEVIEW_OPEN, OnOpen)
	ON_COMMAND(IDM_FILEVIEW_SAVEAS, OnSaveAs)
	ON_COMMAND(IDM_FILEVIEW_DELETE, OnDelete)
	ON_COMMAND(IDM_FILEVIEW_RENAME, OnRename)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILEVIEW_ADD, IDM_FILEVIEW_RENAME, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CFileView::OnDestroy()
{
	if (m_pSortArray)
		delete[] m_pSortArray;

	CFrontstageWnd::OnDestroy();
}

void CFileView::OnPaint()
{
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

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	if (m_wndExplorerList.GetView()==LV_VIEW_DETAILS)
	{
		const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();

		CRect rectWindow;
		m_wndExplorerList.GetWindowRect(rectWindow);

		WINDOWPOS wp;
		HDLAYOUT HdLayout;
		HdLayout.prc = &rectWindow;
		HdLayout.pwpos = &wp;
		m_wndHeader.Layout(&HdLayout);

		ScreenToClient(rectWindow);

		if (Themed)
		{
			Bitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP);

			Graphics g(dc);
			g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+BACKSTAGEBORDER-1, wp.cy+TaskHeight-(INT)pDivider->GetHeight());

			CTaskbar::DrawTaskbarShadow(g, CRect(0, TaskHeight, rect.right, rect.bottom));
		}
		else
		{
			dc.FillSolidRect(0, TaskHeight, rect.Width(), wp.cy, GetSysColor(COLOR_3DFACE));
		}
	}

	DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_Count)
	{
		m_wndExplorerList.SetFocus();
	}
	else
	{
		m_wndTaskbar.SetFocus();
	}
}

void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	CMenu Menu;
	Menu.LoadMenu(IDM_FILEVIEW_BACKGROUND);
	ASSERT_VALID(&Menu);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
}

void CFileView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO*)pNMHDR;
	AIRX_Attachment* pAttachment = GetAttachment(pDispInfo->item.iItem);

	// Columns
	if (pDispInfo->item.mask & LVIF_COLUMNS)
	{
		pDispInfo->item.cColumns = 2;
		pDispInfo->item.puColumns[0] = 2;
		pDispInfo->item.puColumns[1] = 3;
	}

	// Text
	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			pDispInfo->item.pszText = pAttachment->Name;
			break;

		case 1:
		case 2:
			{
				FILETIME ft;
				SYSTEMTIME st;
				FileTimeToLocalFileTime(pDispInfo->item.iSubItem==1 ? &pAttachment->Created : &pAttachment->Modified, &ft);
				FileTimeToSystemTime(&ft, &st);

				WCHAR Date[256] = L"";
				WCHAR Time[256] = L"";
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, Date, 256);
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, Time, 256);

				swprintf_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, L"%s %s", Date, Time);
				break;
			}

		case 3:
			StrFormatByteSize(pAttachment->Size, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
			break;
		}
	}

	// Icon
	if (pDispInfo->item.mask & LVIF_IMAGE)
	{
		if (pAttachment->IconID==-1)
		{
			CString tmpStr(pAttachment->Name);
			INT Pos = tmpStr.ReverseFind(L'.');

			CString Ext = (Pos==-1) ? _T("*") : tmpStr.Mid(Pos);

			SHFILEINFO sfi;
			pAttachment->IconID = SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? sfi.iIcon : 3;
		}

		pDispInfo->item.iImage = pAttachment->IconID;
	}

	*pResult = 0;
}

void CFileView::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (GetSelectedFile()!=-1)
		PostMessage(WM_COMMAND, (WPARAM)IDM_FILEVIEW_OPEN);
}

void CFileView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged & LVIF_STATE)
		m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

	*pResult = 0;
}

void CFileView::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	AIRX_Attachment* pAttachment = GetAttachment(pDispInfo->item.iItem);
	if (pAttachment)
	{
		CString tmpStr(pAttachment->Name);
		INT Pos = tmpStr.ReverseFind(L'.');
		if (Pos!=-1)
		{
			tmpStr.Truncate(Pos);

			CEdit* pEdit = m_wndExplorerList.GetEditControl();
			if (pEdit)
				pEdit->SetWindowText(tmpStr);
		}
	}

	*pResult = FALSE;
}

void CFileView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = FALSE;

	if (pDispInfo->item.pszText)
		if (pDispInfo->item.pszText[0]!=L'\0')
		{
			AIRX_Attachment* pAttachment = GetAttachment(pDispInfo->item.iItem);
			if (pAttachment)
			{
				CString tmpStr(pAttachment->Name);
				INT Pos = tmpStr.ReverseFind(L'.');
				tmpStr.Delete(0, Pos!=-1 ? Pos : tmpStr.GetLength());

				wcscpy_s(pAttachment->Name, MAX_PATH, pDispInfo->item.pszText);

				WCHAR* pChar = pAttachment->Name;
				while (*pChar)
				{
					if (wcschr(L"<>:\"/\\|?*", *pChar))
						*pChar = L'_';

					pChar++;
				}

				wcscat_s(pAttachment->Name, MAX_PATH, tmpStr.GetBuffer());
				pAttachment->IconID = -1;
				p_Itinerary->m_IsModified = TRUE;

				*pResult = TRUE;
			}
		}
}

void CFileView::OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TEXTCOLOR* pTextColor = (NM_TEXTCOLOR*)pNMHDR;

	if (pTextColor->Item!=-1)
	{
		AIRX_Attachment* pAttachment = GetAttachment(pTextColor->Item);

		if (pAttachment->Flags & AIRX_Invalid)
		{
			pTextColor->Color = 0x0000FF;
		}
		else
			if (pAttachment->Flags & AIRX_Valid)
			{
				pTextColor->Color = 0x208040;
			}
	}

	*pResult = 0;
}

void CFileView::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		AIRX_Attachment* pAttachment = GetAttachment(pTooltipData->Item);

		CString SubitemNames[3];
		for (UINT a=1; a<4; a++)
			ENSURE(SubitemNames[a-1].LoadString(IDS_SUBITEM_NAME+a));

		swprintf_s(pTooltipData->Hint, 4096, L"%s: %s\n%s: %s\n%s: %s", SubitemNames[0], m_wndExplorerList.GetItemText(pTooltipData->Item, 1), SubitemNames[1], m_wndExplorerList.GetItemText(pTooltipData->Item, 2), SubitemNames[2], m_wndExplorerList.GetItemText(pTooltipData->Item, 3));

		Bitmap* pBitmap = p_Itinerary->DecodePictureAttachment(*pAttachment);
		if (pBitmap)
		{
			INT L = pBitmap->GetWidth();
			INT H = pBitmap->GetHeight();
			if ((L<16) || (H<16))
				goto UseIcon;

			// Resolution
			CString tmpStr;
			tmpStr.Format(IDS_RESOLUTION, L, H);

			wcscat_s(pTooltipData->Hint, 4096, tmpStr);

			// Scaling
			DOUBLE ScaleX = 256.0/(DOUBLE)L;
			DOUBLE ScaleY = 256.0/(DOUBLE)H;
			DOUBLE Scale = min(ScaleX, ScaleY);
			if (Scale>1.0)
				Scale = 1.0;

			INT Width = (INT)(Scale*(DOUBLE)L);
			INT Height = (INT)(Scale*(DOUBLE)H);

			// Create bitmap
			CDC dc;
			dc.CreateCompatibleDC(NULL);

			BITMAPINFOHEADER bmi = { sizeof(bmi) };
			bmi.biWidth = Width;
			bmi.biHeight = Height;
			bmi.biPlanes = 1;
			bmi.biBitCount = 24;

			BYTE* pbData = NULL;
			pTooltipData->hBitmap = CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pbData, NULL, 0);
			HGDIOBJ hOldBitmap = dc.SelectObject(pTooltipData->hBitmap);

			// Draw
			dc.FillSolidRect(0, 0, Width, Height, 0xFFFFFF);

			Graphics g(dc);
			g.SetCompositingMode(CompositingModeSourceOver);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			g.DrawImage(pBitmap, 0, 0, Width, Height);

			dc.SelectObject(hOldBitmap);
		}
		else
		{
UseIcon:
			// Icon
			pTooltipData->hIcon = FMGetApp()->m_SystemImageListLarge.ExtractIcon(pAttachment->IconID);
		}

		delete pBitmap;

		*pResult = TRUE;
	}
	else
	{
		*pResult = FALSE;
	}
}

void CFileView::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
	INT Column = pLV->iItem;

	if (Column!=(INT)m_LastSortColumn)
	{
		m_LastSortColumn = Column;
		m_LastSortDirection = FALSE;
	}
	else
	{
		m_LastSortDirection = !m_LastSortDirection;
	}

	Sort();

	m_wndExplorerList.Invalidate();

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
	INT Index = GetSelectedFile();
	if (Index!=-1)
	{
		AIRX_Attachment* pAttachment = GetSelectedAttachment();
		if (!pAttachment)
			return;

		CWaitCursor csr;

		// Dateinamen finden
		TCHAR Pathname[MAX_PATH];
		if (!GetTempPath(MAX_PATH, Pathname))
			return;

		CString szTempExt(pAttachment->Name);
		INT Pos = szTempExt.ReverseFind(L'.');
		szTempExt.Delete(0, Pos!=-1 ? Pos : szTempExt.GetLength());

		CString szTempName;
		srand(rand());
		szTempName.Format(_T("%sFlightmap%.4X%.4X%s"), Pathname, 32768+rand(), 32768+rand(), szTempExt);

		// Datei erzeugen
		CFile f;
		if (!f.Open(szTempName, CFile::modeWrite | CFile::modeCreate))
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}
		else
		{
			try
			{
				f.Write(pAttachment->pData, pAttachment->Size);
				f.Close();

				ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOWNORMAL);
			}
			catch(CFileException ex)
			{
				FMErrorBox(this, IDS_DRIVENOTREADY);
				f.Close();
			}
		}
	}
}

void CFileView::OnSaveAs()
{
	INT Index = GetSelectedFile();
	if (Index!=-1)
	{
		AIRX_Attachment* pAttachment = GetAttachment(Index);
		if (!pAttachment)
			return;

		CFileDialog dlg(FALSE, NULL, pAttachment->Name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
		if (dlg.DoModal()==IDOK)
		{
			CWaitCursor csr;

			CFile f;
			if (!f.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
			{
				FMErrorBox(this, IDS_DRIVENOTREADY);
			}
			else
			{
				try
				{
					f.Write(pAttachment->pData, pAttachment->Size);
					f.Close();
				}
				catch(CFileException ex)
				{
					FMErrorBox(this, IDS_DRIVENOTREADY);
					f.Close();
				}
			}
		}
	}
}

void CFileView::OnDelete()
{
	INT Index = GetSelectedFile();
	if (Index!=-1)
		if (FMMessageBox(this, CString((LPCSTR)IDS_DELETE_FILE), GetAttachment(Index)->Name, MB_YESNO | MB_ICONWARNING)==IDYES)
		{
			p_Itinerary->DeleteAttachment(p_Flight ? p_Flight->Attachments[m_pSortArray[Index]] : m_pSortArray[Index], p_Flight);
			Reload();
		}
}

void CFileView::OnRename()
{
	INT Index = GetSelectedFile();
	if (Index!=-1)
	{
		if (GetFocus()!=&m_wndExplorerList)
			m_wndExplorerList.SetFocus();

		m_wndExplorerList.EditLabel(Index);
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	switch (pCmdUI->m_nID)
	{
	case IDM_FILEVIEW_ADD:
		if (p_Flight)
			bEnable = (p_Flight->AttachmentCount<AIRX_MaxAttachmentCount);

		break;

	default:
		if (m_Count)
			bEnable = (GetSelectedFile()!=-1);
	}

	pCmdUI->Enable(bEnable);
}
