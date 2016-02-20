
// CMapView.h: Schnittstelle der Klasse CMapView
//

#pragma once
#include "FMCommDlg.h"


// CMapView
//

#define ZOOMFACTORS     11

static const DOUBLE ZoomFactors[ZOOMFACTORS] = { 0.125, 1.0/6.0, 0.25, 1.0/3.0, 0.5, 2.0/3.0, 0.75, 1.0, 2.0, 3.0, 4.0 };

class CMapView : public CFrontstageWnd
{
public:
	CMapView();
	~CMapView();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetBitmap(CBitmap* pBitmap);

protected:
	DOUBLE GetZoomFactor() const;
	void ScaleBitmap();
	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CBitmap* p_BitmapOriginal;
	CBitmap* m_pBitmapScaled;
	BOOL m_Hover;
	INT m_ScrollWidth;
	INT m_ScrollHeight;
	INT m_ZoomFactor;

private:
	void DeleteScaledBitmap();
	void ResetScrollbars();
	void AdjustScrollbars();

	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_HScrollPos;
	INT m_VScrollPos;
};

inline DOUBLE CMapView::GetZoomFactor() const
{
	return ZoomFactors[min(m_ZoomFactor, ZOOMFACTORS)];
}
