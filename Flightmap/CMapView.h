
// CMapView.h: Schnittstelle der Klasse CMapView
//

#pragma once
#include "FMCommDlg.h"


// CMapView
//

class CMapView : public CWnd
{
public:
	CMapView();
	~CMapView();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetBitmap(CBitmap* pBitmap);

protected:
	FMTooltip m_TooltipCtrl;
	CBitmap* p_Bitmap;
	BOOL m_Hover;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void AdjustLayout();
	void ResetScrollbars();
	void AdjustScrollbars();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

private:
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_HScrollPos;
	INT m_VScrollPos;
};
