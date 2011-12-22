
// CGlobeWnd.h: Schnittstelle der Klasse CGlobeWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CGlobeView.h"


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
	void AddFlight(CHAR* From, CHAR* To, COLORREF Color);
	void CalcFlights();

protected:
	HICON m_hIcon;
	CGlobeView m_wndGlobeView;

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
