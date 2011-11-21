
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
friend class CDialogMenuPopup;

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
	DynArray<MenuBarItem> m_Items;
	HTHEME hTheme;
	CMFCToolBarImages m_Icons;
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

struct MenuPopupItem
{
	CDialogMenuItem* pItem;
	RECT Rect;
	BOOL Enabled;
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
	void AddFileType(UINT CmdID, CString FileType, UINT PreferredSize=CDMB_SMALL);
	void AddFile(UINT CmdID, CString Path, UINT PreferredSize=CDMB_SMALL);
	void AddSeparator(BOOL ForBlueArea=FALSE);
	void AddCaption(UINT ResID);
	void Track(CPoint pt);
	INT GetGutter();
	CFont* SelectNormalFont(CDC* pDC);
	CFont* SelectCaptionFont(CDC* pDC);

protected:
	DynArray<MenuPopupItem> m_Items;
	UINT m_LargeIconsID;
	UINT m_SmallIconsID;
	INT m_Gutter;
	INT m_Width;
	INT m_Height;
	INT m_BlueAreaStart;
	INT m_FirstRowOffset;
	CMFCToolBarImages m_LargeIcons;
	CMFCToolBarImages m_SmallIcons;

	void AddItem(CDialogMenuItem* pItem, INT FirstRowOffset=0);

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
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
	virtual INT GetBorder();
	virtual BOOL IsEnabled();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, BOOL Themed);
	virtual void OnSelect();
	virtual void OnDeselect();
	virtual void OnMouseMove(CPoint point);
	virtual void OnClick(CPoint point);
	virtual void OnHover(CPoint point);

protected:
	CDialogMenuPopup* p_ParentPopup;
};


// CDialogMenuCommand
//

class CDialogMenuCommand : public CDialogMenuItem
{
public:
	CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize);

	virtual INT GetMinHeight();
	virtual INT GetMinWidth();
	virtual INT GetMinGutter();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, BOOL Themed);
	virtual void OnDrawIcon(CDC* pDC, CPoint pt);

protected:
	UINT m_CmdID;
	INT m_IconID;
	CSize m_IconSize;
	UINT m_PreferredSize;
	CString m_Caption;
	CString m_Hint;
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
	virtual INT GetBorder();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, BOOL Themed);

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
	virtual INT GetBorder();

	virtual void OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, BOOL Themed);

private:
	CString m_Caption;
};
