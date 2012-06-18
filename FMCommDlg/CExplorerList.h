
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "FMApplication.h"


// CExplorerList
//

class CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();

	void AddCategory(INT ID, CString Name, CString Hint=_T(""), BOOL Collapsable=FALSE);
	void AddColumn(INT ID, CString Name);
	void SetMenus(UINT ItemMenuID=0, BOOL HighlightFirst=FALSE, UINT BackgroundMenuID=0);

protected:
	FMApplication* p_App;
	HTHEME hTheme;
	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;

	virtual void Init();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()
};
