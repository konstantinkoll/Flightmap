
// CLoungeView.h: Schnittstelle der Klasse CLoungeView
//


#pragma once


// CLoungeView
//

class CLoungeView : public CWnd
{
public:
	CLoungeView();

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
