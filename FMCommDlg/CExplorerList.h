
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once


// CExplorerList
//

class CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();

	void AddCategory(INT ID, CString Name, CString Hint=_T(""), BOOL Collapsible=FALSE);
	void AddColumn(INT ID, CString Name, BOOL Right=FALSE);
	void SetMenus(UINT ItemMenuID=0, BOOL HighlightFirst=FALSE, UINT BackgroundMenuID=0);

protected:
	virtual void Init();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	HTHEME hTheme;

private:
	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;
};
