
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "FMTooltip.h"


// CTaskButton
//

class CTaskButton : public CButton
{
public:
	CTaskButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	BOOL Create(CString Caption, CString TooltipHeader, CString TooltipHint, CMFCToolBarImages* Icons, INT IconID, CWnd* pParentWnd, UINT nID);
	void SetIconID(INT IconID);
	INT GetPreferredWidth();

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_TooltipHeader;
	CString m_TooltipHint;
	FMTooltip m_TooltipCtrl;
	CMFCToolBarImages* p_Icons;
	INT m_IconID;
	BOOL m_Hover;
};