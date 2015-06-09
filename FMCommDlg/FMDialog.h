
// FMDialog.h: Schnittstelle der Klasse FMDialog
//


#pragma once
#include "CGroupBox.h"


// FMDialog
//

class FMDialog : public CDialog
{
public:
	FMDialog(UINT nIDTemplate, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	void GetLayoutRect(LPRECT lpRect) const;

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	CWnd* GetBottomWnd() const;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

	UINT m_nIDTemplate;

private:
	CGroupBox m_GroupBox[4];
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
