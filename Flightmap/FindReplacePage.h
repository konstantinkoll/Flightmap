
// FindReplacePage.h: Schnittstelle der Klasse FindReplacePage
//

#pragma once
#include "CDataGrid.h"


// FindReplacePage
//

class FindReplacePage : public CPropertyPage
{
public:
	FindReplacePage(FindReplaceSettings* pFindReplaceSettings, BOOL AllowColumnOnly);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	FindReplaceSettings* p_FindReplaceSettings;
	BOOL m_AllowColumnOnly;
	CComboBox m_wndSearchTerm;
	CComboBox m_wndReplaceTerm;
	CButton m_wndMatchCase;
	CButton m_wndMatchEntireCell;
	CButton m_wndMatchColumnOnly;
	CButton m_wndReplaceAll;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
