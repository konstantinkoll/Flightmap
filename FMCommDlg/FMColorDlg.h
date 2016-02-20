
// FMColorDlg.h: Schnittstelle der Klasse FMColorDlg
//

#pragma once
#include "CColorPicker.h"
#include "FMDialog.h"


// FMColorDlg
//

class FMColorDlg : public FMDialog
{
public:
	FMColorDlg(COLORREF* pColor, CWnd* pParentWnd=NULL, BOOL AllowReset=TRUE);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnDoubleClickPicker(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClickHistory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnResetColor(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	COLORREF* p_Color;
	BOOL m_AllowReset;

private:
	CColorPicker m_wndColorPicker;
	CColorHistory m_wndColorHistory;
};
