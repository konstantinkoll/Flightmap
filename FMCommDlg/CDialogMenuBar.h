
// CDialogMenuBar.h: Schnittstelle der Klasse CDialogMenuBar
//

#pragma once
#include "FMApplication.h"
#include "CMainWindow.h"
#include "DynArray.h"

#define WM_MENULEFT            WM_USER+5
#define WM_MENURIGHT           WM_USER+6
#define WM_MENUUPDATESTATUS    WM_USER+7
#define WM_GALLERYCHANGED      WM_USER+8


// CDialogCmdUI
//

class CDialogCmdUI : public CCmdUI
{
public:
	CDialogCmdUI();

	virtual void Enable(BOOL bOn=TRUE);
	virtual void SetCheck(INT nCheck=1);

	BOOL m_Enabled;
	BOOL m_Checked;
	INT m_CheckedItem;
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
	BOOL Enabled;
	INT IconID;
	INT Left;
	INT Right;
	INT MinWidth;
	WCHAR Name[256];
	UINT Accelerator;
};

class CDialogMenuBar : public CWnd
{
friend class CMainWindow;
friend class CDialogMenuPopup;

public:
	CDialogMenuBar();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
	INT GetMinWidth();
	void AddMenuLeft(UINT nID);
	void AddMenuRight(UINT nCmdID, INT nIconID);
	void AdjustLayout();
	BOOL HasFocus();

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
	void SelectItem(INT idx, BOOL Keyboard);
	void ExecuteItem(INT idx);
	void SetTheme();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnClosePopup();
	afx_msg LRESULT OnPtInRect(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg void OnMenuLeft();
	afx_msg void OnMenuRight();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
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
	BOOL Checked;
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
	void AddGallery(UINT CmdID, UINT IconsID, CSize IconSize, UINT FirstCaptionID, UINT ItemCount, UINT Columns, COLORREF DefaultColor=0xFFFFFF, BOOL CloseOnExecute=TRUE);
	void AddCommand(UINT CmdID, INT IconID=-1, UINT PreferredSize=CDMB_SMALL, BOOL CloseOnExecute=TRUE);
	void AddSubmenu(UINT CmdID, INT IconID=-1, UINT PreferredSize=CDMB_SMALL, BOOL Split=FALSE);
	void AddFileType(UINT CmdID, CString FileType, UINT PreferredSize=CDMB_SMALL, BOOL RetainCaption=FALSE);
	void AddFile(UINT CmdID, CString Path, UINT PreferredSize=CDMB_SMALL);
	void AddCheckbox(UINT CmdID, BOOL Radio=FALSE, BOOL CloseOnExecute=FALSE);
	void AddColor(UINT CmdID, COLORREF* pColor);
	void AddSeparator(BOOL ForBlueArea=FALSE);
	void AddCaption(UINT ResID);
	void GetCheckSize(CSize& sz);
	void SetParentMenu(CWnd* pWnd, BOOL Keyboard);
	void Track(CRect rect, BOOL Down=TRUE);
	void Track(CPoint point);
	BOOL HasItems();
	INT GetGutter();
	INT GetBlueAreaStart();
	CFont* SelectNormalFont(CDC* pDC);
	CFont* SelectCaptionFont(CDC* pDC);
	void DrawBevelRect(CDC& dc, INT x, INT y, INT width, INT height, BOOL Themed);
	void DrawSelectedBackground(CDC* pDC, LPRECT rect, BOOL Enabled=TRUE, BOOL Focused=TRUE);
	void DrawButton(CDC* pDC, LPRECT rect, BOOL Radio, BOOL Checked, BOOL Enabled, BOOL Selected, BOOL Pressed);

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
	BOOL m_Keyboard;
	CMFCToolBarImages m_LargeIcons;
	CMFCToolBarImages m_SmallIcons;
	CDialogMenuPopup* p_SubMenu;
	CWnd* p_ParentMenu;

	void AddItem(CDialogMenuItem* pItem, INT FirstRowOffset=0);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT idx);
	void SelectItem(INT idx, BOOL FromTop=FALSE);
	void TrackSubmenu(CDialogMenuPopup* pPopup, BOOL Keyboard);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnPtInRect(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg void OnMenuLeft();
	afx_msg void OnMenuRight();
	afx_msg LRESULT OnMenuUpdateStatus(WPARAM wParam, LPARAM lParam);
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
	HTHEME hThemeList;
	HTHEME hThemeButton;
	BOOL m_EnableHover;
	CPoint m_LastMove;
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
	virtual BOOL IsChecked();
	virtual BOOL IsSelectable();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);
	virtual void OnSelect(BOOL Keyboard, BOOL FromTop);
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


// CDialogMenuGallery
//

class CDialogMenuGallery : public CDialogMenuItem
{
public:
	CDialogMenuGallery(CDialogMenuPopup* pParentPopup, UINT CmdID, UINT IconsID, CSize IconSize, UINT FirstCaptionID, UINT ItemCount, UINT Columns, COLORREF DefaultColor=0xFFFFFF, BOOL CloseOnExecute=TRUE);
	virtual ~CDialogMenuGallery();

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual BOOL IsEnabled();
	virtual BOOL IsChecked();
	virtual BOOL IsSelectable();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);
	virtual void OnSelect(BOOL Keyboard, BOOL FromTop);
	virtual void OnDeselect();
	virtual BOOL OnButtonDown(CPoint point);
	virtual BOOL OnButtonUp(CPoint point);
	virtual BOOL OnMouseMove(CPoint point);
	virtual BOOL OnMouseLeave();
	virtual BOOL OnKeyDown(UINT nChar);

protected:
	CMFCToolBarImages m_Icons;
	UINT m_CmdID;
	UINT m_ItemCount;
	UINT m_Rows;
	UINT m_Columns;
	UINT m_SelectedItem;
	INT m_HoverItem;
	BOOL m_Enabled;
	BOOL m_Pressed;
	BOOL m_CloseOnExecute;
	CSize m_CheckSize;
	CSize m_IconSize;
	UINT m_ItemHeight;
	UINT m_ItemWidth;
	CString* m_Captions;
	COLORREF m_DefaultColor;

	virtual void Execute();
};


// CDialogMenuCommand
//

class CDialogMenuCommand : public CDialogMenuItem
{
public:
	CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize, BOOL Submenu=FALSE, BOOL Split=FALSE, BOOL CloseOnExecute=TRUE);
	virtual ~CDialogMenuCommand();

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetMinGutter();
	virtual UINT GetAccelerator();
	virtual BOOL IsEnabled();
	virtual BOOL IsSelectable();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed);
	virtual void OnDrawIcon(CDC* pDC, CPoint pt, BOOL Selected, BOOL Themed);
	virtual void OnSelect(BOOL Keyboard, BOOL FromTop);
	virtual void OnDeselect();
	virtual BOOL OnButtonDown(CPoint point);
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
	BOOL m_CloseOnExecute;
	CString m_Caption;
	CString m_Hint;

	virtual void Execute();

	INT GetInnerBorder();

private:
	BOOL PtOnSubmenuArrow(CPoint point);
	BOOL TrackSubmenu(BOOL Select);

	CDialogMenuPopup* m_pSubmenu;
	BOOL m_HoverOverCommand;
};


// CDialogMenuFileType
//

class CDialogMenuFileType : public CDialogMenuCommand
{
public:
	CDialogMenuFileType(CDialogMenuPopup* pParentPopup, UINT CmdID, CString FileType, UINT PreferredSize, BOOL RetainCaption);

	virtual void OnDrawIcon(CDC* pDC, CPoint pt, BOOL Selected, BOOL Themed);

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


// CDialogMenuCheckbox
//

class CDialogMenuCheckbox : public CDialogMenuCommand
{
public:
	CDialogMenuCheckbox(CDialogMenuPopup* pParentPopup, UINT CmdID, BOOL Radio, BOOL CloseOnExecute);

	virtual INT GetMinHeight();
	virtual BOOL IsChecked();

	virtual void OnDrawIcon(CDC* pDC, CPoint pt, BOOL Selected, BOOL Themed);
	virtual void OnDeselect();
	virtual BOOL OnButtonDown(CPoint point);
	virtual BOOL OnButtonUp(CPoint point);

protected:
	BOOL m_Checked;
	BOOL m_Pressed;
	BOOL m_Radio;
};


// CDialogMenuColor
//

class CDialogMenuColor : public CDialogMenuCommand
{
public:
	CDialogMenuColor(CDialogMenuPopup* pParentPopup, UINT CmdID, COLORREF* pColor);

	virtual void OnDrawIcon(CDC* pDC, CPoint pt, BOOL Selected, BOOL Themed);

protected:
	COLORREF* p_Color;

	virtual void Execute();
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
