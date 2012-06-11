
// CMainWindow: Schnittstelle der Klasse CMainWindow
//

#pragma once
#include "FMApplication.h"
#include "CDialogMenuBar.h"


// CMainWindow
//

#define WM_REQUESTSUBMENU         WM_USER+1
#define WM_PTINRECT               WM_USER+2
#define WM_CLOSEPOPUP             WM_USER+3
#define WM_USEBGIMAGESCHANGED     WM_USER+4

class CMainWindow : public CWnd
{
friend class CDialogMenuBar;
friend class CDialogMenuPopup;

public:
	CMainWindow();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();
	virtual void PostNcDestroy();

	BOOL Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd=NULL, UINT nID=0);
	void RegisterPopupWindow(CWnd* pPopupWnd);

protected:
	FMApplication* p_App;
	CWnd* p_PopupWindow;
	CDialogMenuBar* m_pDialogMenuBar;
	BOOL m_Active;
	HACCEL hAccelerator;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnClosePopup();
	afx_msg void OnUseBgImagesChanged();
	DECLARE_MESSAGE_MAP()
};
