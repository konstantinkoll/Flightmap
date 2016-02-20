
// CColorHistory: Schnittstelle der Klasse CColorHistory
//

#pragma once
#include "CFrontstageWnd.h"


// CColorHistory
//

#define COLORHISTORY_DBLCLK     1

struct NM_COLORDATA
{
	NMHDR hdr;
	COLORREF clr;
};

class CColorHistory : public CFrontstageWnd
{
public:
	CColorHistory();

	virtual void PreSubclassWindow();

protected:
	void SetFocusItem(INT FocusItem);
	INT ItemAtPosition(CPoint point) const;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	COLORREF m_Colors[16];
	INT m_FocusItem;
	INT m_HotItem;
	INT m_ItemWidth;
	BOOL m_Hover;
};
