
// FindReplaceDlg.h: Schnittstelle der Klasse FindReplaceDlg
//

#pragma once
#include "CDataGrid.h"
#include "FMCommDlg.h"


// FindReplaceDlg
//

class FindReplaceDlg : public FMTabbedDialog
{
public:
	FindReplaceDlg(UINT Attr, CWnd* pParentWnd=NULL, INT SelectTab=-1);

	FindReplaceSettings m_FindReplaceSettings;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void ShowTab(UINT Index);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);

	static UINT m_LastTab;

	CComboBox m_wndSearchTerm;
	CComboBox m_wndReplaceTerm;
	CButton m_wndMatchCase;
	CButton m_wndMatchEntireCell;
	CButton m_wndMatchColumnOnly;
	CButton m_wndReplaceAll;

private:
	BOOL m_AllowColumnOnly[2];
};
