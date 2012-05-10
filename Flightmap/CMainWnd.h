
// CMainWnd.h: Schnittstelle der Klasse CMainWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CItinerary.h"
#include "CKitchen.h"


// CMainWnd
//

#define LoungeView     1
#define DataGrid       2

class CMainWnd : public CMainWindow
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create(CItinerary* pItinerary);


protected:
	HICON m_hIcon;
	CWnd* m_pWndMainView;
	UINT m_CurrentMainView;
	CItinerary* m_pItinerary;

	void UpdateWindowStatus(BOOL AllowLoungeView=FALSE);
	void Open(CString FileName);
	BOOL CloseFile(BOOL AllowLoungeView=FALSE);
	CKitchen* GetKitchen(BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	CBitmap* GetMap(BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportMap(CString Filename, GUID guidFileType, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportCalendar(CString FileName);
	BOOL ExportGoogleEarth(CString FileName, BOOL UseColors=TRUE, BOOL Clamp=FALSE, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportText(CString FileName);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnRequestSubmenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGalleryChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUseBgImagesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFileNew();
	afx_msg void OnFileNewSample1();
	afx_msg void OnFileNewSample2();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenRecent(UINT CmdID);
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSaveICS();
	afx_msg void OnFileSaveTXT();
	afx_msg void OnFileSaveOther();
	afx_msg void OnFileProperties();
	afx_msg void OnFileClose();
	afx_msg void OnFileQuit();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);

	afx_msg void OnMapOpen();
	afx_msg void OnMapMergeMetro();
	afx_msg void OnMapCenterAtlantic();
	afx_msg void OnMapCenterPacific();
	afx_msg void OnMapShowFlightRoutes();
	afx_msg void OnMapStraightLines();
	afx_msg void OnMapArrows();
	afx_msg void OnMapUseColors();
	afx_msg void OnMapShowLocations();
	afx_msg void OnMapShowIATACodes();
	afx_msg void OnUpdateMapCommands(CCmdUI* pCmdUI);

	afx_msg void OnMapExportBMP();
	afx_msg void OnMapExportJPEG();
	afx_msg void OnMapExportPNG();
	afx_msg void OnMapExportTIFF();

	afx_msg void OnGlobeOpen();
	afx_msg void OnGlobeMergeMetro();
	afx_msg void OnUpdateGlobeCommands(CCmdUI* pCmdUI);

	afx_msg void OnGoogleEarthOpen();
	afx_msg void OnGoogleEarthMergeMetro();
	afx_msg void OnGoogleEarthColors();
	afx_msg void OnGoogleEarthClamp();
	afx_msg void OnUpdateGoogleEarthCommands(CCmdUI* pCmdUI);

	afx_msg void OnGoogleEarthExport();
	DECLARE_MESSAGE_MAP()
};
