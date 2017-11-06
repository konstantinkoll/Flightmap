

// CRecentFilesPane.cpp: Implementierung der Klasse CRecentFilesPane
//

#include "stdafx.h"
#include "CRecentFilesPane.h"
#include "Flightmap.h"


// CRecentFilesPane
//

#define GetSelectedFile()     m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

CRecentFilesPane::CRecentFilesPane()
	: CFrontstagePane()
{
	m_RecentFileCount = 0;
}

void CRecentFilesPane::AdjustLayout(CRect rectLayout)
{
	const INT BorderLeft = BACKSTAGEBORDER-PANEGRIPPER;
	m_wndExplorerList.SetWindowPos(NULL, rectLayout.left+BorderLeft, rectLayout.top, rectLayout.Width()-BorderLeft, rectLayout.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CRecentFilesPane::UpdateList()
{
	ZeroMemory(&m_RecentFiles, sizeof(m_RecentFiles));
	m_RecentFileCount = 0;

	if (!theApp.m_RecentFiles.IsEmpty())
	{
		for (POSITION p=theApp.m_RecentFiles.GetHeadPosition(); p; )
		{
			// Name
			CString tmpStr = theApp.m_RecentFiles.GetNext(p);
			wcscpy_s(m_RecentFiles[m_RecentFileCount].Path, MAX_PATH, tmpStr);

			// Basic file attributes
			WIN32_FIND_DATAW FindFileData;
			HANDLE hFind = FindFirstFile(tmpStr, &FindFileData);

			if (hFind!=INVALID_HANDLE_VALUE)
			{
				m_RecentFiles[m_RecentFileCount].Created = FindFileData.ftCreationTime;
				m_RecentFiles[m_RecentFileCount].Modified = FindFileData.ftLastWriteTime;
				m_RecentFiles[m_RecentFileCount].FileSize = (((INT64)FindFileData.nFileSizeHigh) << 32) | FindFileData.nFileSizeLow;

				FindClose(hFind);

				m_RecentFiles[m_RecentFileCount].IsValid = TRUE;
			}

			// Icon via extension
			tmpStr.Delete(0, tmpStr.ReverseFind(L'\\')+1);
			INT Pos = tmpStr.ReverseFind(L'.');

			CString Ext = (Pos==-1) ? _T("*") : tmpStr.Mid(Pos);

			SHFILEINFO sfi;
			m_RecentFiles[m_RecentFileCount].IconID = SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? sfi.iIcon : 3;

			if (++m_RecentFileCount>MAXRECENTFILES)
				break;
		}
	}

	m_wndExplorerList.SetItemCount(m_RecentFileCount);
}

CString CRecentFilesPane::GetSelectedRecentFilePath() const
{
	const INT Index = GetSelectedFile();
	if (Index!=-1)
		if (m_RecentFiles[Index].IsValid)
			return m_RecentFiles[Index].Path;

	return _T("");
}


BEGIN_MESSAGE_MAP(CRecentFilesPane, CFrontstagePane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, 1, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, 1, OnDoubleClick)
	ON_NOTIFY(REQUEST_TEXTCOLOR, 1, OnRequestTextColor)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 1, OnRequestTooltipData)

	ON_COMMAND(IDM_FILE_RECENTPANE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_RECENTPANE_REMOVE, OnFileRemove)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_RECENTPANE_OPEN, IDM_FILE_RECENTPANE_REMOVE, OnUpdateCommands)
END_MESSAGE_MAP()

INT CRecentFilesPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstagePane::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_wndExplorerList.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_SINGLESEL | LVS_SHAREIMAGELISTS | LVS_OWNERDATA, CRect(0, 0, 100, 100), this, 1))
		return -1;

	m_wndExplorerList.SetImageList(&theApp.m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&theApp.m_SystemImageListExtraLarge, LVSIL_NORMAL);

	m_wndExplorerList.SetMenus(IDM_FILE_RECENTPANE, TRUE);

	m_wndExplorerList.SetView(LV_VIEW_TILE);
	m_wndExplorerList.SetItemsPerRow(1, 2);

	UpdateList();

	return 0;
}

void CRecentFilesPane::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndExplorerList.SetFocus();
}

void CRecentFilesPane::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO*)pNMHDR;
	RecentFile* pRecentFile = &m_RecentFiles[pDispInfo->item.iItem];

	// Columns
	if (pDispInfo->item.mask & LVIF_COLUMNS)
		if (pRecentFile->IsValid)
		{
			pDispInfo->item.cColumns = 2;
			pDispInfo->item.puColumns[0] = 2;
			pDispInfo->item.puColumns[1] = 3;
		}
		else
		{
			pDispInfo->item.cColumns = 0;
		}

	// Text
	WCHAR* Ptr;

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			pDispInfo->item.pszText = pRecentFile->Path;

			Ptr = wcsrchr(pRecentFile->Path, L'\\');
			if(Ptr)
				pDispInfo->item.pszText = Ptr+1;

			break;

		case 1:
		case 2:
			{
				FILETIME ft;
				SYSTEMTIME st;
				FileTimeToLocalFileTime(pDispInfo->item.iSubItem==1 ? &pRecentFile->Created : &pRecentFile->Modified, &ft);
				FileTimeToSystemTime(&ft, &st);

				WCHAR Date[256] = L"";
				WCHAR Time[256] = L"";
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, Date, 256);
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, Time, 256);

				swprintf_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, L"%s %s", Date, Time);
				break;
			}

		case 3:
			StrFormatByteSize(pRecentFile->FileSize, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
			break;
		}
	}

	// Icon
	if (pDispInfo->item.mask & LVIF_IMAGE)
		pDispInfo->item.iImage = pRecentFile->IconID;

	*pResult = 0;
}

void CRecentFilesPane::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (GetSelectedFile()!=-1)
		PostMessage(WM_COMMAND, (WPARAM)IDM_FILE_RECENTPANE_OPEN);
}

void CRecentFilesPane::OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TEXTCOLOR* pTextColor = (NM_TEXTCOLOR*)pNMHDR;

	if (pTextColor->Item!=-1)
		if (!m_RecentFiles[pTextColor->Item].IsValid)
			pTextColor->Color = 0x0000FF;

	*pResult = 0;
}

void CRecentFilesPane::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	*pResult = FALSE;

	if (pTooltipData->Item!=-1)
		if (m_RecentFiles[pTooltipData->Item].IsValid)
		{
			CString SubitemNames[3];
			for (UINT a=1; a<4; a++)
				ENSURE(SubitemNames[a-1].LoadString(IDS_SUBITEM_NAME+a));

			swprintf_s(pTooltipData->Hint, 4096, L"%s: %s\n%s: %s\n%s: %s", SubitemNames[0], m_wndExplorerList.GetItemText(pTooltipData->Item, 1), SubitemNames[1], m_wndExplorerList.GetItemText(pTooltipData->Item, 2), SubitemNames[2], m_wndExplorerList.GetItemText(pTooltipData->Item, 3));

			// Icon
			pTooltipData->hIcon = theApp.m_SystemImageListExtraLarge.ExtractIcon(m_RecentFiles[pTooltipData->Item].IconID);

			*pResult = TRUE;
		}
}


// File commands
//

void CRecentFilesPane::OnFileOpen()
{
	const INT Index = GetSelectedFile();
	if (Index!=-1)
		if (m_RecentFiles[Index].IsValid)
			GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_OPENRECENT);
}

void CRecentFilesPane::OnFileRemove()
{
	const INT Index = GetSelectedFile();
	if (Index!=-1)
	{
		CString Str(m_RecentFiles[Index].Path);

		for (POSITION p=theApp.m_RecentFiles.GetHeadPosition(); p; )
		{
			POSITION pl = p;
			if (theApp.m_RecentFiles.GetNext(p)==Str)
				theApp.m_RecentFiles.RemoveAt(pl);
		}

		UpdateList();
	}
}

void CRecentFilesPane::OnUpdateCommands(CCmdUI* pCmdUI)
{
	const INT Index = GetSelectedFile();

	BOOL bEnable = (Index!=-1);

	if (pCmdUI->m_nID==IDM_FILE_RECENTPANE_OPEN)
		bEnable &= m_RecentFiles[Index].IsValid;

	pCmdUI->Enable(bEnable);
}
