
// CDialogMenuBar.h: Schnittstelle der Klasse CDialogMenuBar
//

#pragma once
#include "FMApplication.h"
#include "CMainWindow.h"
#include "DynArray.h"

#define WM_MENULEFT      WM_USER+4
#define WM_MENURIGHT     WM_USER+5


// CDialogCmdUI
//

class CDialogCmdUI : public CCmdUI
{
public:
	CDialogCmdUI();

	virtual void Enable(BOOL bOn=TRUE);

	BOOL m_Enabled;
};


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
friend class CDialogMenuPopup;

public:
	CDialogMenuBar();

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
	INT GetMinWidth();
	void AddMenuLeft(UINT nID);
	void AddMenuRight(UINT nCmdID, INT nIconID);
	void AdjustLayout();

protected:
	FMApplication* p_App;
	DynArray<MenuBarItem> m_Items;
	INT m_SelectedItem;
	INT m_HoverItem;
	BOOL m_Hover;
	BOOL m_UseDropdown;
	CMFCToolBarImages m_Icons;
	LOGFONT m_MenuLogFont;
	LOGFONT m_NormalLogFont;
	LOGFONT m_CaptionLogFont;
	CFont m_MenuFont;
	CFont m_NormalFont;
	CFont m_CaptionFont;

	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT idx);
	void SelectItem(INT idx);
	void SetTheme();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnClosePopup();
	afx_msg LRESULT OnPtInRect(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg LRESULT OnMenuLeft(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMenuRight(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pKillWnd);
	DECLARE_MESSAGE_MAP()

private:
	HTHEME hTheme;
	INT m_MenuHeight;
	CPoint m_LastMove;
	CDialogMenuPopup* m_pPopup;
};


// CDialogMenuPopup
//

class CDialogMenuItem;

struct MenuPopupItem
{
	CDialogMenuItem* pItem;
	RECT Rect;
	BOOL Enabled;
	BOOL Selectable;
	UINT Accelerator;
};

class CDialogMenuPopup : public CWnd
{
friend class CDialogMenuCommand;

public:
	CDialogMenuPopup();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT LargeIconsID=0, UINT SmallIconsID=0);
	void AddCommand(UINT CmdID, INT IconID=-1, UINT PreferredSize=CDMB_SMALL);
	void AddSubmenu(UINT CmdID, INT IconID=-1, UINT PreferredSize=CDMB_SMALL, BOOL Split=FALSE);
	void AddFileType(UINT CmdID, CString FileType, UINT PreferredSize=CDMB_SMALL);
	void AddFile(UINT CmdID, CString Path, UINT PreferredSize=CDMB_SMALL);
	void AddSeparator(BOOL ForBlueArea=FALSE);
	void AddCaption(UINT ResID);
	void SetParentMenu(CWnd* pWnd);
	void Track(CPoint point);
	BOOL HasItems();
	INT GetGutter();
	INT GetBlueAreaStart();
	CFont* SelectNormalFont(CDC* pDC);
	CFont* SelectCaptionFont(CDC* pDC);
	void DrawSelectedBackground(CDC* pDC, LPRECT rect, BOOL Enabled=TRUE, BOOL Focused=TRUE);

protected:
	FMApplication* p_App;
	DynArray<MenuPopupItem> m_Items;
	UINT m_LargeIconsID;
	UINT m_SmallIconsID;
	INT m_Gutter;
	INT m_Width;
	INT m_Height;
	INT m_BlueAreaStart;
	INT m_FirstRowOffset;
	INT m_SelectedItem;
	INT m_LastSelectedItem;
	BOOL m_Hover;
	CMFCToolBarImages m_LargeIcons;
	CMFCToolBarImages m_SmallIcons;
	CDialogMenuPopup* p_SubMenu;
	CWnd* p_ParentMenu;

	void AddItem(CDialogMenuItem* pItem, INT FirstRowOffset=0);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT idx);
	void SelectItem(INT idx);
	void TrackSubmenu(CDialogMenuPopup* pPopup);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnPtInRect(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg LRESULT OnMenuLeft(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMenuRight(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg INT OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	DECLARE_MESSAGE_MAP()

private:
	HTHEME hThemeButton;
	HTHEME hThemeList;
	BOOL m_EnableHover;
	CPoint m_LastMove;

	void FixShadow();
};


// CDialogMenuItem
//

class CDialogMenuItem
{
public:
	CDialogMenuItem(CDialogMenuPopup* pParentPopup);
	virtual ~CDialogMenuItem();

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetMinGutter();
	virtual INT GetOuterBorder();
	virtual UINT GetAccelerator();
	virtual BOOL IsEnabled();
	virtual BOOL IsSelectable();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);
	virtual void OnDeselect();
	virtual BOOL OnButtonDown(CPoint point);
	virtual BOOL OnButtonUp(CPoint point);
	virtual BOOL OnMouseMove(CPoint point);
	virtual BOOL OnMouseLeave();
	virtual BOOL OnHover(CPoint point);
	virtual BOOL OnKeyDown(UINT nChar);

protected:
	CDialogMenuPopup* p_ParentPopup;
};


// CDialogMenuCommand
//

class CDialogMenuCommand : public CDialogMenuItem
{
public:
	CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize, BOOL Submenu=FALSE, BOOL Split=FALSE);
	virtual ~CDialogMenuCommand();

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetMinGutter();
	virtual UINT GetAccelerator();
	virtual BOOL IsEnabled();
	virtual BOOL IsSelectable();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);
	virtual void OnDrawIcon(CDC* pDC, CPoint pt);
	virtual void OnDeselect();
	virtual BOOL OnButtonUp(CPoint point);
	virtual BOOL OnMouseMove(CPoint point);
	virtual BOOL OnMouseLeave();
	virtual BOOL OnHover(CPoint point);
	virtual BOOL OnKeyDown(UINT nChar);

protected:
	UINT m_CmdID;
	INT m_IconID;
	INT m_Width;
	CSize m_IconSize;
	UINT m_PreferredSize;
	BOOL m_Submenu;
	BOOL m_Split;
	BOOL m_Enabled;
	CString m_Caption;
	CString m_Hint;

	INT GetInnerBorder();

private:
	BOOL PtOnSubmenuArrow(CPoint point);
	BOOL TrackSubmenu();
	void Execute();

	CDialogMenuPopup* m_pSubmenu;
	BOOL m_HoverOverCommand;
};


// CDialogMenuFileType
//

class CDialogMenuFileType : public CDialogMenuCommand
{
public:
	CDialogMenuFileType(CDialogMenuPopup* pParentPopup, UINT CmdID, CString FileType, UINT PreferredSize);

	virtual INT GetMinHeight();
	virtual INT GetMinGutter();

	virtual void OnDrawIcon(CDC* pDC, CPoint pt);

private:
	CImageList* p_Icons;
};


// CDialogMenuFile
//

class CDialogMenuFile : public CDialogMenuFileType
{
public:
	CDialogMenuFile(CDialogMenuPopup* pParentPopup, UINT CmdID, CString Path, UINT PreferredSize);
};


// CDialogMenuSeparator
//

class CDialogMenuSeparator : public CDialogMenuItem
{
public:
	CDialogMenuSeparator(CDialogMenuPopup* pParentPopup, BOOL ForBlueArea);

	virtual INT GetMinHeight();
	virtual INT GetOuterBorder();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);

private:
	BOOL m_ForBlueArea;
};


// CDialogMenuCaption
//

class CDialogMenuCaption : public CDialogMenuItem
{
public:
	CDialogMenuCaption(CDialogMenuPopup* pParentPopup, UINT ResID);

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetOuterBorder();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);

private:
	CString m_Caption;
};
