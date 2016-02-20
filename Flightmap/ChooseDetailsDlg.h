
// ChooseDetailsDlg.h: Schnittstelle der Klasse ChooseDetailsDlg
//

#pragma once
#include "CDataGrid.h"
#include "FMCommDlg.h"


// ChooseDetailsDlg
//

class ChooseDetailsDlg : public FMDialog
{
public:
	ChooseDetailsDlg(ViewParameters* pViewParameters, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

	CExplorerList m_wndAttributes;
	ViewParameters* p_ViewParameters;

private:
	void AddAttribute(UINT Attr);
	void SwapItems(INT FocusItem, INT NewPos);
};
