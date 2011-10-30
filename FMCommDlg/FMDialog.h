
// FMDialog.h: Schnittstelle der Klasse FMDialog
//


#pragma once
#include "CGdiPlusBitmap.h"
#include "CGroupBox.h"
#include "FMApplication.h"


// FMDialog
//

#define FMDS_Blue         1
#define FMDS_White        2

class FMDialog : public CDialog
{
public:
	FMDialog(UINT nIDTemplate, UINT Design, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	void GetLayoutRect(LPRECT lpRect) const;
	UINT GetDesign() const;

protected:
	FMApplication* p_App;
	CGdiPlusBitmapResource* m_pLogo;
	UINT m_nIDTemplate;
	UINT m_Design;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	CWnd* GetBottomWnd() const;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CGdiPlusBitmapResource* m_pBackdrop;
	CGroupBox m_GroupBox[4];
	HICON hIconL;
	HICON hIconS;
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
