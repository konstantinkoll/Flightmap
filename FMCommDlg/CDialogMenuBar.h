
// CDialogMenuBar.h: Schnittstelle der Klasse CDialogMenuBar
//

#pragma once
//#include "CTaskButton.h"
#include "FMApplication.h"
#include <list>


// CDialogMenuBar
//

class CDialogMenuBar : public CWnd
{
public:
	CDialogMenuBar();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
//	CTaskButton* AddButton(UINT nID, INT IconID, BOOL ForceIcon=FALSE, BOOL AddRight=FALSE);
	void AdjustLayout();

protected:
	void SetTheme();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	FMApplication* p_App;
	CMFCToolBarImages Icons;
//	CList<CTaskButton*> m_ButtonsLeft;
//	CList<CTaskButton*> m_ButtonsRight;
	HTHEME hTheme;
};
