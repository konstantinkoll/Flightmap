
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
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual void ShowTooltip(const CPoint& point);

	void SetFocusItem(INT FocusItem);
	void UpdateCursor();

	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
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
	INT m_ItemWidth;

private:
	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
};
