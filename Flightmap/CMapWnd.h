
// CMapWnd.h: Schnittstelle der Klasse CMapWnd
//

#pragma once
#include "CKitchen.h"
#include "CMapView.h"
#include "FMCommDlg.h"


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
	void SetBitmap(CBitmap* pBitmap, CString DisplayName, CString Title);

protected:
	void ExportMap(CString Filename, GUID guidFileType);
	void ExportMap(DWORD FilterIndex=3);
	void Print(PRINTDLGEX pdex);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnMapWndCopy();
	afx_msg void OnMapWndSaveAs();
	afx_msg void OnMapWndPrint();
	afx_msg void OnMapWndPrintQuick();
	afx_msg void OnMapWndClose();
	afx_msg void OnUpdateMapWndCommands(CCmdUI* pCmdUI);

	afx_msg void OnMapExportBMP();
	afx_msg void OnMapExportJPEG();
	afx_msg void OnMapExportPNG();
	afx_msg void OnMapExportTIFF();
	afx_msg void OnUpdateMapExportCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CBitmap* m_pBitmap;
	CMapView m_wndMapView;

private:
	CString m_Title;
};
