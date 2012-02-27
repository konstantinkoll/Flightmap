
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

protected:
	FMApplication* p_App;
	HTHEME hTheme;

	virtual void Init();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	DECLARE_MESSAGE_MAP()
};
