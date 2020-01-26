
#pragma once
#include "FMCommDlg.h"


// CRecentFilesList
//

struct FileItemData
{
	ItemData Hdr;
	WCHAR Path[MAX_PATH];
	WCHAR DisplayName[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	INT64 FileSize;
	INT iIcon;
	BOOL IsValid;
};

class CRecentFilesList sealed : public CFrontstageItemView
{
public:
	CRecentFilesList();

	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	LPCWSTR GetSelectedFilePath(BOOL OnlyValid=TRUE) const;
	void SetFiles();

protected:
	virtual void ShowTooltip(const CPoint& point);
	virtual void AdjustLayout();
	virtual COLORREF GetItemTextColor(INT Index, BOOL Themed) const;
	virtual void FireSelectedItem();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	FileItemData* GetFileItemData(INT Index) const;
	void AddFile(LPCWSTR Path, const WIN32_FIND_DATA& FindFileData, INT iIcon, BOOL IsValid);

	static CString m_SubitemNames[3];
};

inline FileItemData* CRecentFilesList::GetFileItemData(INT Index) const
{
	return (FileItemData*)GetItemData(Index);
}


// CRecentFilesPane
//

class CRecentFilesPane : public CFrontstagePane
{
public:
	virtual void AdjustLayout(CRect rectLayout);

	LPCWSTR GetSelectedFilePath(BOOL OnlyValid=TRUE) const;

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnFileOpen();
	afx_msg void OnFileViewInFolder();
	afx_msg void OnFileRemove();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CRecentFilesList m_wndFileList;
};

inline LPCWSTR CRecentFilesPane::GetSelectedFilePath(BOOL OnlyValid) const
{
	return m_wndFileList.GetSelectedFilePath(OnlyValid);
}
