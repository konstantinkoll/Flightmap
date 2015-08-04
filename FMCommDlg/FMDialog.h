
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
	virtual void AdjustLayout();

	void GetLayoutRect(LPRECT lpRect) const;

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	CWnd* GetBottomWnd() const;
	void SetBottomLeftControl(CWnd* pChildWnd);
	void SetBottomLeftControl(UINT nID);
	void AddBottomRightControl(CWnd* pChildWnd);
	void AddBottomRightControl(UINT nID);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
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
	CWnd* p_BottomLeftControl;
	CList<CWnd*> m_BottomRightControls;
	CPoint m_LastSize;
};
