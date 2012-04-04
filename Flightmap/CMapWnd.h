
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
	void SetBitmap(CBitmap* pBitmap, CString DisplayName=_T(""));

protected:
	HICON m_hIcon;
	CBitmap* m_pBitmap;
	CMapView m_wndMapView;

	void ExportMap(CString Filename, GUID guidFileType);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnMapWndCopy();
	afx_msg void OnMapWndSaveAs();
	afx_msg void OnMapWndClose();
	afx_msg void OnUpdateMapWndCommands(CCmdUI* pCmdUI);

	afx_msg void OnMapExportBMP();
	afx_msg void OnMapExportJPEG();
	afx_msg void OnMapExportPNG();
	afx_msg void OnMapExportTIFF();
	afx_msg void OnUpdateMapExportCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
