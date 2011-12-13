
// CGlobeWnd.h: Schnittstelle der Klasse CGlobeWnd
//

#pragma once
#include "FMCommDlg.h"


// CGlobeWnd
//

class CGlobeWnd : public CMainWindow
{
public:
	CGlobeWnd();
	virtual ~CGlobeWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create();

protected:
	HICON m_hIcon;
	CWnd* m_pWndMainView;

	void OpenMainView(BOOL Empty);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFileOpen();
	afx_msg void OnFileQuit();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);

	afx_msg void OnGlobeOpen();
	afx_msg void OnUpdateGlobeCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
