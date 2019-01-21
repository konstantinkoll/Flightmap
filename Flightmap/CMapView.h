
// CMapView.h: Schnittstelle der Klasse CMapView
//

#pragma once
#include "FMCommDlg.h"


// CMapView
//

#define ZOOMFACTORS     11

static const DOUBLE ZoomFactors[ZOOMFACTORS] = { 0.125, 1.0/6.0, 0.25, 1.0/3.0, 0.5, 2.0/3.0, 0.75, 1.0, 2.0, 3.0, 4.0 };

class CMapView sealed : public CFrontstageScroller
{
public:
	CMapView();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetBitmap(CBitmap* pBitmap, const CString& DisplayName=_T(""));

protected:
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void ShowTooltip(const CPoint& point);
	virtual BOOL DrawNothing() const;
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);

	DOUBLE GetZoomFactor() const;
	void ScaleBitmap();
	void GetCardRect(const CRect& rectClient, CRect& rectCard) const;
	void GetCardRect(CRect& rectCard) const;

	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CBitmap* p_BitmapOriginal;
	CBitmap* m_pBitmapScaled;
	CString m_Title;
	INT m_ZoomFactor;

private:
	void DeleteScaledBitmap();
};

inline DOUBLE CMapView::GetZoomFactor() const
{
	return ZoomFactors[min(m_ZoomFactor, ZOOMFACTORS)];
}

inline void CMapView::GetCardRect(CRect& rectCard) const
{
	CRect rectClient;
	GetClientRect(rectClient);

	GetCardRect(rectClient, rectCard);
}
