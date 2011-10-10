
// CLoungeView.h: Schnittstelle der Klasse CLoungeView
//


#pragma once
#include "FMCommDlg.h"


// CLoungeView
//

class CLoungeView : public CWnd
{
public:
	CLoungeView();

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CGdiPlusBitmapResource* m_pBackdrop;
	CGdiPlusBitmapResource* m_pLogo;
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};