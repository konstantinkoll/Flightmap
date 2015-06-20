
// ChooseDetailsDlg.h: Schnittstelle der Klasse ChooseDetailsDlg
//

#pragma once
#include "CDataGrid.h"
#include "FMCommDlg.h"


// ChooseDetailsDlg
//

class ChooseDetailsDlg : public CDialog
{
public:
	ChooseDetailsDlg(ViewParameters* pViewParameters, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

	CListCtrl m_ShowAttributes;
	ViewParameters* p_ViewParameters;

private:
	void AddAttribute(UINT Attr);
	void SwapItems(INT FocusItem, INT NewPos);
};
