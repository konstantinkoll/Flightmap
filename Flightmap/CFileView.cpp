
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"


// CFileView
//

#define GetSelectedFile() m_wndTooltipList.GetNextItem(-1, LVIS_SELECTED)
#define GetSelectedAttachment() GetAttachment(GetSelectedFile())

CFileView::CFileView()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CFileView";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CFileView", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Status = NULL;
	p_Itinerary = NULL;
	p_Flight = NULL;
	p_App = FMGetApp();
	m_Sorting = NULL;
	m_LastSortColumn = m_Count = 0;
	m_LastSortDirection = FALSE;
}

void CFileView::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CFileView::AdjustLayout()
{
	if (!IsWindow(m_wndTaskbar))
		return;
	if (!IsWindow(m_wndTooltipList))
		return;

	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndTooltipList.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::SetData(CWnd* pStatus, CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	p_Status = pStatus;
	p_Itinerary = pItinerary;
	p_Flight = pFlight;

	m_wndTooltipList.SetView(p_Flight ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	Reload();

	for (UINT a=0; a<4; a++)
		m_wndTooltipList.SetColumnWidth(a, m_Count==0 ? 130 : a<3 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

void CFileView::Reload()
{
	m_Count = p_Flight ? p_Flight->AttachmentCount : p_Itinerary->m_Attachments.m_ItemCount;
	m_wndTooltipList.SetItemCount(m_Count);

	if (m_Sorting)
		delete[] m_Sorting;

	m_Sorting = new UINT[m_Count];
	for (UINT a=0; a<m_Count; a++)
		m_Sorting[a] = a;

	Sort();

	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

	// Status
	if (p_Status)
	{
		INT64 FileSize = 0;
		for (UINT a=0; a<m_Count; a++)
			FileSize += GetAttachment(a)->Size;

		CString tmpMask;
		ENSURE(tmpMask.LoadString(m_Count==1 ? IDS_FILESTATUS_SINGULAR : IDS_FILESTATUS_PLURAL));

		WCHAR tmpBuf[256];
		StrFormatByteSize(FileSize, tmpBuf, 256);

		CString tmpStr;
		tmpStr.Format(tmpMask, m_Count, tmpBuf);
		p_Status->SetWindowText(tmpStr);
	}
}

AIRX_Attachment* CFileView::GetAttachment(INT idx)
{
	return idx==-1 ? NULL : p_Flight ? &p_Itinerary->m_Attachments.m_Items[p_Flight->Attachments[m_Sorting[idx]]] : &p_Itinerary->m_Attachments.m_Items[m_Sorting[idx]];
}

void CFileView::Init()
{
	ModifyStyle(0, WS_BORDER);

	m_wndTaskbar.Create(this, IDB_TASKS, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_SAVEAS, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS | LVS_SINGLESEL | LVS_NOLABELWRAP;
	CRect rect;
	rect.SetRectEmpty();
	m_wndTooltipList.Create(dwStyle, rect, this, 2);

	m_wndTooltipList.SetImageList(&p_App->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndTooltipList.SetImageList(&p_App->m_SystemImageListLarge, LVSIL_NORMAL);

	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_SUBITEM_NAME+a));

		m_wndTooltipList.AddColumn(a, tmpStr, a);
	}

	IMAGEINFO ii;
	p_App->m_SystemImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_wndTooltipList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy")).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndTooltipList.SetMenus(IDM_FILEVIEW_ITEM, TRUE, IDM_FILEVIEW_BACKGROUND);
	m_wndTooltipList.SetFocus();

	CHeaderCtrl* pHeaderCtrl = m_wndTooltipList.GetHeaderCtrl();
	if (pHeaderCtrl)
		m_wndHeader.SubclassWindow(pHeaderCtrl->GetSafeHwnd());

	AdjustLayout();
}

INT CFileView::Compare(INT n1, INT n2)
{
	INT res = 0;

	AIRX_Attachment* pFirst = GetAttachment(n1);
	AIRX_Attachment* pSecond = GetAttachment(n2);

	switch (m_LastSortColumn)
	{
	case 0:
		res = wcscmp(pFirst->Name, pSecond->Name);
		break;
	case 1:
		res = CompareFileTime(&pFirst->Created, &pSecond->Created);
		break;
	case 2:
		res = CompareFileTime(&pFirst->Modified, &pSecond->Modified);
		break;
	case 3:
		res = pFirst->Size-pSecond->Size;
		break;
	}

	if (m_LastSortDirection)
		res = -res;

	return res;
}

void CFileView::Heap(INT wurzel, INT anz)
{
	while (wurzel<=anz/2-1)
	{
		INT idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (Compare(idx, idx+1)<0)
				idx++;
		if (Compare(wurzel, idx)<0)
		{
			std::swap(m_Sorting[wurzel], m_Sorting[idx]);
			wurzel = idx;
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
		for (INT a=m_Count-1; a>0; )
		{
			std::swap(m_Sorting[0], m_Sorting[a]);
			Heap(0, a--);
		}
	}

	CHeaderCtrl* pHeaderCtrl = m_wndTooltipList.GetHeaderCtrl();

	HDITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = HDI_FORMAT;

	for (INT a=0; a<4; a++)
	{
		pHeaderCtrl->GetItem(a, &item);

		item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		if (a==(INT)m_LastSortColumn)
			item.fmt |= m_LastSortDirection ? HDF_SORTDOWN : HDF_SORTUP;

		pHeaderCtrl->SetItem(a, &item);
	}

	if (m_wndTooltipList.GetNextItem(-1, LVIS_FOCUSED)==-1)
		m_wndTooltipList.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_INITMENUPOPUP()
	ON_NOTIFY(NM_CUSTOMDRAW, 2, OnCustomDraw)
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, 2, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, 2, OnItemChanged)
	ON_NOTIFY(LVN_BEGINLABELEDIT, 2, OnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, 2, OnEndLabelEdit)
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
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CFileView::OnDestroy()
{
	if (m_Sorting)
		delete[] m_Sorting;

	CWnd::OnDestroy();
}

void CFileView::OnNcPaint()
{
	DrawControlBorder(this);
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndTooltipList.SetFocus();
}

void CFileView::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = state.m_pParentMenu = pPopupMenu;
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();

	ASSERT(!state.m_pOther);
	ASSERT(state.m_pMenu);

	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if ((state.m_nID) && (state.m_nID!=(UINT)-1))
			state.DoUpdate(this, FALSE);
	}
}

void CFileView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT==pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else
		if (CDDS_ITEMPREPAINT==pLVCD->nmcd.dwDrawStage)
		{
			AIRX_Attachment* pAttachment = GetAttachment((INT)pLVCD->nmcd.dwItemSpec);
			if (pAttachment->Flags & AIRX_Invalid)
			{
				pLVCD->clrText = 0x0000FF;
			}
			else
				if (pAttachment->Flags & AIRX_Valid)
				{
					pLVCD->clrText = 0x208040;
				}
		}
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

				swprintf_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, L"%s, %s", Date, Time);
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

			CEdit* pEdit = m_wndTooltipList.GetEditControl();
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

void CFileView::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		AIRX_Attachment* pAttachment = GetAttachment(pTooltipData->Item);

		CString SubitemNames[3];
		for (UINT a=1; a<4; a++)
			ENSURE(SubitemNames[a-1].LoadString(IDS_SUBITEM_NAME+a));

		swprintf_s(pTooltipData->Text, sizeof(pTooltipData->Text)/sizeof(WCHAR), L"%s: %s\n%s: %s\n%s: %s", SubitemNames[0], m_wndTooltipList.GetItemText(pTooltipData->Item, 1), SubitemNames[1], m_wndTooltipList.GetItemText(pTooltipData->Item, 2), SubitemNames[2], m_wndTooltipList.GetItemText(pTooltipData->Item, 3));

		CGdiPlusBitmap* pBitmap = p_Itinerary->DecodePictureAttachment(*pAttachment);
		if (pBitmap->m_pBitmap)
		{
			INT l = pBitmap->m_pBitmap->GetWidth();
			INT h = pBitmap->m_pBitmap->GetHeight();
			if ((l<16) || (h<16))
				goto UseIcon;

			// Resolution
			CString tmpMask;
			CString tmpStr;
			ENSURE(tmpMask.LoadString(IDS_RESOLUTION));

			tmpStr.Format(tmpMask, l, h);
			wcscat_s(pTooltipData->Text, sizeof(pTooltipData->Text)/sizeof(WCHAR), tmpStr);

			// Scaling
			DOUBLE ScaleX = 256.0/(DOUBLE)l;
			DOUBLE ScaleY = 256.0/(DOUBLE)h;
			DOUBLE Scale = min(ScaleX, ScaleY);
			if (Scale>1.0)
				Scale = 1.0;

			INT Width = 2+(INT)(Scale*(DOUBLE)l);
			INT Height = 2+(INT)(Scale*(DOUBLE)h);

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

			g.DrawImage(pBitmap->m_pBitmap, 1, 1, Width-2, Height-2);

			dc.Draw3dRect(0, 0, Width, Height, 0x000000, 0x000000);
			dc.SelectObject(hOldBitmap);

			pTooltipData->cx = Width;
			pTooltipData->cy = Height;
		}
		else
		{
UseIcon:
			// Icon
			IMAGEINFO ii;
			p_App->m_SystemImageListLarge.GetImageInfo(0, &ii);

			pTooltipData->hIcon = p_App->m_SystemImageListLarge.ExtractIcon(pAttachment->IconID);
			pTooltipData->cx = ii.rcImage.right-ii.rcImage.left;
			pTooltipData->cy = ii.rcImage.bottom-ii.rcImage.top;
		}

		pTooltipData->Show = TRUE;
		delete pBitmap;
	}

	*pResult = 0;
}

void CFileView::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
	INT col = pLV->iItem;

	if (col!=(INT)m_LastSortColumn)
	{
		m_LastSortColumn = col;
		m_LastSortDirection = FALSE;
	}
	else
	{
		m_LastSortDirection = !m_LastSortDirection;
	}

	Sort();

	m_wndTooltipList.Invalidate();

	*pResult = 0;
}

void CFileView::OnAdd()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, NULL, this);
	if (dlg.DoModal()==IDOK)
	{
		POSITION pos = dlg.GetStartPosition();
		while (pos)
			if (!p_Itinerary->AddAttachment(*p_Flight, dlg.GetNextPathName(pos)))
				break;

		Reload();
	}
}

void CFileView::OnOpen()
{
	INT idx = GetSelectedFile();
	if (idx!=-1)
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
			FMErrorBox(IDS_DRIVENOTREADY, GetSafeHwnd());
		}
		else
		{
			try
			{
				f.Write(pAttachment->pData, pAttachment->Size);
				f.Close();

				ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOW);
			}
			catch(CFileException ex)
			{
				FMErrorBox(IDS_DRIVENOTREADY, GetSafeHwnd());
				f.Close();
			}
		}
	}
}

void CFileView::OnSaveAs()
{
	INT idx = GetSelectedFile();
	if (idx!=-1)
	{
		AIRX_Attachment* pAttachment = GetAttachment(idx);
		if (!pAttachment)
			return;

		CFileDialog dlg(FALSE, NULL, pAttachment->Name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
		if (dlg.DoModal()==IDOK)
		{
			CWaitCursor csr;

			CFile f;
			if (!f.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
			{
				FMErrorBox(IDS_DRIVENOTREADY, GetSafeHwnd());
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
					FMErrorBox(IDS_DRIVENOTREADY, GetSafeHwnd());
					f.Close();
				}
			}
		}
	}
}

void CFileView::OnDelete()
{
	INT idx = GetSelectedFile();
	if (idx!=-1)
	{
		CString message;
		ENSURE(message.LoadString(IDS_DELETE_FILE));

		if (MessageBox(message, GetAttachment(idx)->Name, MB_YESNO | MB_ICONWARNING)==IDYES)
		{
			p_Itinerary->DeleteAttachment(p_Flight ? p_Flight->Attachments[idx] : idx, p_Flight);
			Reload();
		}
	}
}

void CFileView::OnRename()
{
	INT idx = GetSelectedFile();
	if (idx!=-1)
	{
		if (GetFocus()!=&m_wndTooltipList)
			m_wndTooltipList.SetFocus();

		m_wndTooltipList.EditLabel(idx);
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	switch (pCmdUI->m_nID)
	{
	case IDM_FILEVIEW_ADD:
		if (p_Flight)
			b = (p_Flight->AttachmentCount<AIRX_MaxAttachmentCount);
		break;
	default:
		if (m_Count)
			b = (GetSelectedFile()!=-1);
	}

	pCmdUI->Enable(b);
}
