
// CDialogMenuBar.h: Schnittstelle der Klasse CDialogMenuBar
//

#pragma once
#include "FMApplication.h"
#include "CMainWindow.h"
#include "DynArray.h"


// CDialogMenuBar
//

struct MenuBarItem
{
	UINT PopupID;
	UINT CmdID;
	INT IconID;
	INT Left;
	INT Right;
	INT MinWidth;
	WCHAR Name[256];
};

class CDialogMenuBar : public CWnd
{
public:
	CDialogMenuBar();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
	INT GetMinWidth();
	void AddMenuLeft(UINT nID, UINT nCaptionResID);
	void AddMenuRight(UINT nCmdID, INT nIconID);
	void AdjustLayout();

protected:
	void SetTheme();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	FMApplication* p_App;
	CMFCToolBarImages m_Icons;
	DynArray<MenuBarItem> m_Items;
	HTHEME hTheme;
	LOGFONT m_MenuLogFont;
	CFont m_MenuFont;
	INT m_MenuHeight;
};


// CDialogMenuPopup
//

class CDialogMenuPopup : public CWnd
{
public:
	CDialogMenuPopup();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT LargeIconsID=0, UINT SmallIconsID=0);
	void Track(CPoint pt);

protected:
	UINT m_LargeIconsID;
	UINT m_SmallIconsID;
	CMFCToolBarImages m_LargeIcons;
	CMFCToolBarImages m_SmallIcons;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwTask);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};


// CDialogMenuItem
//



// CDialogMenuButton
//

#define CDMB_SMALL      0
#define CDMB_MEDIUM     1
#define CDMB_LARGE      2
