
// CDialogMenuBar.h: Schnittstelle der Klasse CDialogMenuBar
//

#pragma once
#include "FMApplication.h"
#include "CMainWindow.h"
#include "DynArray.h"


// CDialogMenuBar
//

#define CDMB_SMALL      0
#define CDMB_MEDIUM     1
#define CDMB_LARGE      2

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
friend class CMainWindow;

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
	LOGFONT m_NormalLogFont;
	LOGFONT m_CaptionLogFont;
	CFont m_MenuFont;
	CFont m_NormalFont;
	CFont m_CaptionFont;
	INT m_MenuHeight;
};


// CDialogMenuPopup
//

class CDialogMenuItem;

class CDialogMenuPopup : public CWnd
{
public:
	CDialogMenuPopup();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT LargeIconsID=0, UINT SmallIconsID=0);
	void AddCommand(UINT CmdID, INT IconID=-1, UINT PreferredSize=CDMB_SMALL);
	void Track(CPoint pt);

protected:
	UINT m_LargeIconsID;
	UINT m_SmallIconsID;
	CMFCToolBarImages m_LargeIcons;
	CMFCToolBarImages m_SmallIcons;

	void AddItem(CDialogMenuItem* pItem);

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

class CDialogMenuItem
{
public:
	CDialogMenuItem(CDialogMenuPopup* pParentPopup);

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetMinGutter();
	virtual BOOL IsActive();

	virtual void OnPaint(CDC* pDC, LPRECT rect);
	virtual void OnSelect();
	virtual void OnDeselect();
	virtual void OnClick(CPoint point);

protected:
	CDialogMenuPopup* p_ParentPopup;
};


// CDialogMenuCommand
//

class CDialogMenuCommand : public CDialogMenuItem
{
public:
	CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize);

protected:
	UINT m_CmdID;
	INT m_IconID;
	UINT m_PreferredSize;
	CString m_Caption;
	CString m_Hint;
};
