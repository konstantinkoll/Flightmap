
// CMapView.h: Schnittstelle der Klasse CMapView
//

#pragma once


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
	void AdjustLayout();
	void ResetScrollbars();
	void AdjustScrollbars();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CBitmap* p_Bitmap;
	BOOL m_Hover;
	INT m_ScrollWidth;
	INT m_ScrollHeight;
	INT m_ZoomFactor;
	BOOL m_Autosize;

private:
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_HScrollPos;
	INT m_VScrollPos;
};
