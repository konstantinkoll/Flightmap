
// CMainWnd.h: Schnittstelle der Klasse CMainWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CKitchen.h"


// CMainWnd
//

class CMainWnd : public CMainWindow
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create();

	BOOL ExportKML(CString FileName, BOOL UseColors=TRUE, BOOL Clamp=FALSE, BOOL Selected=FALSE);

protected:
	HICON m_hIcon;
	CWnd* m_pWndMainView;

	void OpenMainView(BOOL Empty);
	CKitchen* GetKitchen(BOOL Selected=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGalleryChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFileOpen();
	afx_msg void OnFileQuit();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);

	afx_msg void OnMapOpen();
	afx_msg void OnMapCenterAtlantic();
	afx_msg void OnMapCenterPacific();
	afx_msg void OnMapShowFlightRoutes();
	afx_msg void OnMapStraightLines();
	afx_msg void OnMapUseColors();
	afx_msg void OnMapShowLocations();
	afx_msg void OnMapShowIATACodes();
	afx_msg void OnUpdateMapCommands(CCmdUI* pCmdUI);

	afx_msg void OnGlobeOpen();
	afx_msg void OnUpdateGlobeCommands(CCmdUI* pCmdUI);

	afx_msg void OnGoogleEarthOpen();
	afx_msg void OnGoogleEarthExport();
	afx_msg void OnGoogleEarthColors();
	afx_msg void OnGoogleEarthClamp();
	afx_msg void OnUpdateGoogleEarthCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
