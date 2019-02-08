

// CRecentFilesPane.cpp: Implementierung der Klasse CRecentFilesPane
//

#include "stdafx.h"
#include "CRecentFilesPane.h"
#include "Flightmap.h"


// CRecentFilesList
//

#define MAXRECENTFILES     20

CString CRecentFilesList::m_SubitemNames[3];

CRecentFilesList::CRecentFilesList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(FileItemData))
{
	// Subitem names
	if (m_SubitemNames[0].IsEmpty())
		for (UINT a=1; a<4; a++)
			ENSURE(m_SubitemNames[a-1].LoadString(IDS_SUBITEM_NAME+a));

	// Item
	SetItemHeight(theApp.m_SystemImageListExtraLarge, 3);
}

void CRecentFilesList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const FileItemData* pData = GetFileItemData(m_HoverItem);

	if (pData->IsValid)
	{
		// Hint
		WCHAR strSize[256];
		StrFormatByteSize(pData->FileSize, strSize, 256);

		WCHAR Hint[4096];
		swprintf_s(Hint, 4096, L"%s: %s\n%s: %s\n%s: %s",
			m_SubitemNames[0], (LPCWSTR)FMTimeToString(pData->Created),
			m_SubitemNames[1], (LPCWSTR)FMTimeToString(pData->Modified),
			m_SubitemNames[2], strSize);

		// Show tooltip
		theApp.ShowTooltip(this, point, pData->DisplayName, Hint,
			theApp.m_SystemImageListExtraLarge.ExtractIcon(pData->iIcon), NULL);
	}
}

BOOL CRecentFilesList::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index>=0)
		Menu.LoadMenu(IDM_FILE_RECENTPANE);

	return TRUE;
}

void CRecentFilesList::AddFile(LPCWSTR Path, const WIN32_FIND_DATA& FindFileData, INT iIcon, BOOL IsValid)
{
	FileItemData Data;

	wcscpy_s(Data.Path, MAX_PATH, Path);

	const LPCWSTR DisplayName = wcsrchr(Data.Path, L'\\');
	wcscpy_s(Data.DisplayName, MAX_PATH, DisplayName ? DisplayName+1 : Data.Path);

	Data.Created = FindFileData.ftCreationTime;
	Data.Modified = FindFileData.ftLastWriteTime;
	Data.FileSize = (((INT64)FindFileData.nFileSizeHigh) << 32) | FindFileData.nFileSizeLow;
	Data.iIcon = iIcon;
	Data.IsValid = IsValid;

	AddItem(&Data);
}

void CRecentFilesList::SetFiles()
{
	// Add Files
	SetItemCount(MAXRECENTFILES, FALSE);

	if (!theApp.m_RecentFiles.IsEmpty())
		for (POSITION p=theApp.m_RecentFiles.GetHeadPosition(); p; )
		{
			// Name
			CString Path = theApp.m_RecentFiles.GetNext(p);

			// Basic file attributes
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind = FindFirstFile(Path, &FindFileData);

			// Icon via extension
			CString tmpStr = Path;
			tmpStr.Delete(0, tmpStr.ReverseFind(L'\\')+1);

			const INT Pos = tmpStr.ReverseFind(L'.');

			CString Ext = (Pos==-1) ? _T("*") : tmpStr.Mid(Pos);

			SHFILEINFO ShellFileInfo;
			AddFile(Path, FindFileData, SUCCEEDED(SHGetFileInfo(Ext, 0, &ShellFileInfo, sizeof(ShellFileInfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? ShellFileInfo.iIcon : 3, hFind!=INVALID_HANDLE_VALUE);

			if (hFind!=INVALID_HANDLE_VALUE)
				FindClose(hFind);
		}

	LastItem();

	AdjustLayout();
}

LPCWSTR CRecentFilesList::GetSelectedFilePath(BOOL OnlyValid) const
{
	const INT Index = GetSelectedItem();
	if (Index>=0)
	{
		const FileItemData* pData = GetFileItemData(Index);

		return !OnlyValid || pData->IsValid ? pData->Path : L"";
	}

	return FALSE;
}

void CRecentFilesList::AdjustLayout()
{
	AdjustLayoutColumns();
}

COLORREF CRecentFilesList::GetItemTextColor(INT Index, BOOL /*Themed*/) const
{
	return GetFileItemData(Index)->IsValid ? (COLORREF)-1 : 0x2020FF;
}

void CRecentFilesList::FireSelectedItem() const
{
	GetOwner()->SendMessage(WM_COMMAND, IDM_FILE_RECENTPANE_OPEN);
}

void CRecentFilesList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const FileItemData* pData = GetFileItemData(Index);

	if (pData->IsValid)
	{
		WCHAR strSize[256];
		StrFormatByteSize(pData->FileSize, strSize, 256);

		DrawTile(dc, rectItem, theApp.m_SystemImageListExtraLarge, pData->iIcon,
			ILD_TRANSPARENT,
			GetDarkTextColor(dc, Index, Themed), 3,
			pData->DisplayName, FMTimeToString(pData->Modified), strSize);
	}
	else
	{
		DrawTile(dc, rectItem, theApp.m_SystemImageListExtraLarge, pData->iIcon,
			ILD_BLEND50,
			GetDarkTextColor(dc, Index, Themed), 1,
			pData->DisplayName);
	}
}


// CRecentFilesPane
//

void CRecentFilesPane::AdjustLayout(CRect rectLayout)
{
	m_wndFileList.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), rectLayout.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CRecentFilesPane, CFrontstagePane)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()

	ON_COMMAND(IDM_FILE_RECENTPANE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_RECENTPANE_VIEWINFOLDER, OnFileViewInFolder)
	ON_COMMAND(IDM_FILE_RECENTPANE_REMOVE, OnFileRemove)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_RECENTPANE_OPEN, IDM_FILE_RECENTPANE_REMOVE, OnUpdateCommands)
END_MESSAGE_MAP()

INT CRecentFilesPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstagePane::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_wndFileList.Create(this, 1))
		return -1;

	m_wndFileList.SetFiles();

	return 0;
}

void CRecentFilesPane::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndFileList.SetFocus();
}


// File commands
//

void CRecentFilesPane::OnFileOpen()
{
	GetOwner()->PostMessage(WM_COMMAND, IDM_FILE_OPENRECENT);
}

void CRecentFilesPane::OnFileViewInFolder()
{
	LPCWSTR Path = GetSelectedFilePath(FALSE);
	ASSERT(Path);

	theApp.OpenFolderAndSelectItem(Path);
}

void CRecentFilesPane::OnFileRemove()
{
	LPCWSTR Path = GetSelectedFilePath(FALSE);
	ASSERT(Path);

	if (Path[0]!=L'\0')
	{
		for (POSITION p=theApp.m_RecentFiles.GetHeadPosition(); p; )
		{
			POSITION pl = p;

			if (theApp.m_RecentFiles.GetNext(p)==Path)
				theApp.m_RecentFiles.RemoveAt(pl);
		}

		m_wndFileList.SetFiles();
	}
}

void CRecentFilesPane::OnUpdateCommands(CCmdUI* pCmdUI)
{
	LPCWSTR Path = GetSelectedFilePath();
	ASSERT(Path);

	pCmdUI->Enable((Path[0]!=L'\0') || (pCmdUI->m_nID==IDM_FILE_RECENTPANE_REMOVE));
}
