
#pragma once
#include "FMCommDlg.h"


// CRecentFilesPane
//

#define MAXRECENTFILES     20

struct RecentFile
{
	WCHAR Path[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	INT64 FileSize;
	INT IconID;
	BOOL IsValid;
};

class CRecentFilesPane : public CFrontstagePane
{
public:
	CRecentFilesPane();

	virtual void AdjustLayout(CRect rectLayout);

	CString GetSelectedRecentFilePath() const;

protected:
	void UpdateList();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnFileOpen();
	afx_msg void OnFileRemove();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CExplorerList m_wndExplorerList;

private:
	RecentFile m_RecentFiles[MAXRECENTFILES];
	UINT m_RecentFileCount;
};
