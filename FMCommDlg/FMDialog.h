
// FMDialog.h: Schnittstelle der Klasse FMDialog
//


#pragma once
#include "CGdiPlusBitmap.h"
#include "CGroupBox.h"

struct FMLicenseVersion
{
	UINT Major;
	UINT Minor;
	UINT Release;
};

struct FMLicense
{
	WCHAR PurchaseID[256];
	WCHAR ProductID[256];
	WCHAR PurchaseDate[16];			// Either DD/MM/YYYY or DD.MM.YYYY
	WCHAR Quantity[8];
	WCHAR RegName[256];
	FMLicenseVersion Version;
};


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
	CGdiPlusBitmapResource* m_pLogo;
	UINT m_nIDTemplate;
	UINT m_Design;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(FMLicense* License=NULL);

	CWnd* GetBottomWnd() const;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnEnterLicenseKey();
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
