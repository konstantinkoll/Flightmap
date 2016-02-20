
// CMainWnd.h: Schnittstelle der Klasse CMainWnd
//

#pragma once
#include "FMCommDlg.h"
#include "CDataGrid.h"
#include "CItinerary.h"
#include "CKitchen.h"


// CMainWnd
//

#define LOUNGEVIEW     1
#define DATAGRID       2

class CMainWnd : public CBackstageWnd
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL GetLayoutRect(LPRECT lpRect) const;

	BOOL Create(CItinerary* pItinerary);

protected:
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);

	void UpdateWindowStatus();
	void SetItinerary(CItinerary* pItinerary);
	void Open(const CString& FileName);
	BOOL CloseFile();
	CKitchen* GetKitchen(BOOL Limit, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	CBitmap* GetMap(BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportMap(const CString& FileName, GUID guidFileType, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportExcel(const CString& FileName);
	void ExportCalendar(const CString& FileName);
	BOOL ExportGoogleEarth(const CString& FileName, BOOL UseCount=FALSE, BOOL UseColors=TRUE, BOOL ClampHeight=FALSE, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void ExportText(const CString& FileName);
	void ExportMap(DWORD FilterIndex=3, BOOL Selected=FALSE, BOOL MergeMetro=FALSE);
	void SaveAs(DWORD FilterIndex=1);
	void Print(PRINTDLGEX pdex);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnDistanceSettingChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFileNew();
	afx_msg void OnFileNewSample1();
	afx_msg void OnFileNewSample2();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenRecent(UINT CmdID);
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSaveCSV();
	afx_msg void OnFileSaveICS();
	afx_msg void OnFileSaveTXT();
	afx_msg void OnFileSaveOther();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintQuick();
	afx_msg void OnFileProperties();
	afx_msg void OnFileInspect();
	afx_msg void OnFileAttachments();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);

	afx_msg void OnItinerary();
	afx_msg void OnMapOpen();
	afx_msg void OnMapSettings();
	afx_msg void OnGlobeOpen();
	afx_msg void OnGlobeSettings();
	afx_msg void OnGoogleEarthOpen();
	afx_msg void OnGoogleEarthSettings();
	afx_msg void OnStatisticsOpen();
	afx_msg void OnStatisticsSettings();
	afx_msg void OnUpdateSidebarCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIconsSidebar;
	static CIcons m_SmallIconsSidebar;
	static CIcons m_LargeIconsTaskbar;
	static CIcons m_SmallIconsTaskbar;
	CBackstageSidebar m_wndSidebar;
	CTaskbar m_wndTaskbar;
	CItinerary* m_pItinerary;

private:
	CDataGrid* m_pDataGridWnd;
};
