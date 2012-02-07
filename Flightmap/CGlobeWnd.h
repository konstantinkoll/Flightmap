
// CGlobeWnd.h: Schnittstelle der Klasse CGlobeWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CGlobeView.h"
#include "CKitchen.h"


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
	void SetFlights(CKitchen* pKitchen);

protected:
	HICON m_hIcon;
	CGlobeView m_wndGlobeView;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);
	afx_msg void On3DSettingsChanged();

	afx_msg void OnGlobeWndClose();
	afx_msg void OnUpdateGlobeWndCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
