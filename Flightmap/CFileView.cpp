
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"


// CFileView
//

#define GetSelectedFile() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)
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
	if (!IsWindow(m_wndExplorerList))
		return;

	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndExplorerList.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::SetData(CWnd* pStatus, CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	p_Status = pStatus;
	p_Itinerary = pItinerary;
	p_Flight = pFlight;

	m_wndExplorerList.SetView(p_Flight ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	Reload();

	for (UINT a=0; a<4; a++)
		m_wndExplorerList.SetColumnWidth(a, m_wndExplorerList.GetItemCount()==0 ? 130 : a<3 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

void CFileView::Reload()
{
	m_wndExplorerList.SetItemCount(p_Flight ? p_Flight->AttachmentCount : p_Itinerary->m_Attachments.m_ItemCount);

	if (m_wndExplorerList.GetNextItem(-1, LVIS_FOCUSED)==-1)
		m_wndExplorerList.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

	// Status
	if (p_Status)
	{
		UINT FileCount = 0;
		INT64 FileSize = 0;

		if (p_Flight)
		{
			FileCount = p_Flight->AttachmentCount;
			for (UINT a=0; a<FileCount; a++)
				FileSize += p_Itinerary->m_Attachments.m_Items[p_Flight->Attachments[a]].Size;
		}
		else
		{
			FileCount = p_Itinerary->m_Attachments.m_ItemCount;
			for (UINT a=0; a<FileCount; a++)
				FileSize += p_Itinerary->m_Attachments.m_Items[a].Size;
		}

		CString tmpMask;
		ENSURE(tmpMask.LoadString(FileCount==1 ? IDS_FILESTATUS_SINGULAR : IDS_FILESTATUS_PLURAL));

		WCHAR tmpBuf[256];
		StrFormatByteSize(FileSize, tmpBuf, 256);

		CString tmpStr;
		tmpStr.Format(tmpMask, FileCount, tmpBuf);
		p_Status->SetWindowText(tmpStr);
	}
}

AIRX_Attachment* CFileView::GetAttachment(INT idx)
{
	return idx==-1 ? NULL : p_Flight ? &p_Itinerary->m_Attachments.m_Items[p_Flight->Attachments[idx]] : &p_Itinerary->m_Attachments.m_Items[idx];
}

void CFileView::Init()
{
	m_wndTaskbar.Create(this, IDB_TASKS, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_SAVEAS, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS | LVS_SINGLESEL;
	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(dwStyle, rect, this, 2);

	FMApplication* pApp = (FMApplication*)AfxGetApp();
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListLarge, LVSIL_NORMAL);

	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_SUBITEM_NAME+a));

		m_wndExplorerList.AddColumn(a, tmpStr, a);
	}

	IMAGEINFO ii;
	pApp->m_SystemImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndExplorerList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy")).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndExplorerList.SetMenus(IDM_FILEVIEW_ITEM, TRUE, IDM_FILEVIEW_BACKGROUND);
	m_wndExplorerList.SetFocus();

	CHeaderCtrl* pHeaderCtrl = m_wndExplorerList.GetHeaderCtrl();
	if (pHeaderCtrl)
		m_wndHeader.SubclassWindow(pHeaderCtrl->GetSafeHwnd());

	AdjustLayout();
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
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
	m_wndExplorerList.SetFocus();
}

void CFileView::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther==NULL);
	ASSERT(state.m_pParentMenu==NULL);

	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu==pPopupMenu->m_hMenu)
	{
		state.m_pParentMenu = pPopupMenu;
	}
	else
	{
		hParentMenu = ::GetMenu(m_hWnd);
		if (hParentMenu)
		{
			INT nIndexMax = GetMenuItemCount(hParentMenu);
			for (INT nIndex=0; nIndex<nIndexMax; nIndex++)
				if (GetSubMenu(hParentMenu, nIndex)==pPopupMenu->m_hMenu)
				{
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (!state.m_nID)
			continue;

		ASSERT(!state.m_pOther);
		ASSERT(state.m_pMenu);
		if (state.m_nID ==(UINT)-1)
		{
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if ((!state.m_pSubMenu) || ((state.m_nID=state.m_pSubMenu->GetMenuItemID(0))== 0) || (state.m_nID==(UINT)-1))
				continue;

			state.DoUpdate(this, TRUE);
		}
		else
		{
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount<state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax-nCount);
			while ((state.m_nIndex<nCount) && (pPopupMenu->GetMenuItemID(state.m_nIndex)==state.m_nID))
				state.m_nIndex++;
		}
		state.m_nIndexMax = nCount;
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
		pDispInfo->item.puColumns[0] = 3;
		pDispInfo->item.puColumns[1] = 1;
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
			StrFormatByteSize(pAttachment->Size, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
			break;
		case 2:
		case 3:
			{
				FILETIME ft;
				SYSTEMTIME st;
				FileTimeToLocalFileTime(pDispInfo->item.iSubItem==2 ? &pAttachment->Created : &pAttachment->Modified, &ft);
				FileTimeToSystemTime(&ft, &st);

				WCHAR Date[256] = L"";
				WCHAR Time[256] = L"";
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, Date, 256);
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, Time, 256);

				swprintf_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, L"%s, %s", Date, Time);
				break;
			}
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

void CFileView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged & LVIF_STATE)
		m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

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
		if (GetFocus()!=&m_wndExplorerList)
			m_wndExplorerList.SetFocus();

		m_wndExplorerList.EditLabel(idx);
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
		if (m_wndExplorerList.GetItemCount())
			b = (GetSelectedFile()!=-1);
	}

	pCmdUI->Enable(b);
}
