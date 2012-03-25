
// CMapWnd.h: Schnittstelle der Klasse CMapWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CKitchen.h"
#include "CMapView.h"


// CMapWnd
//

class CMapWnd : public CMainWindow
{
public:
	CMapWnd();
	virtual ~CMapWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create();
	void SetBitmap(CBitmap* pBitmap);

protected:
	HICON m_hIcon;
	CBitmap* m_pBitmap;
	CMapView m_wndMapView;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);

	afx_msg void OnMapWndClose();
	afx_msg void OnUpdateMapWndCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
