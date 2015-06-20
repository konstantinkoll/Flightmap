
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// CFileView
//

class CFileView : public CWnd
{
public:
	CFileView();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	void SetData(CWnd* pStatus, CItinerary* pItinerary, AIRX_Flight* pFlight=NULL);

protected:
	CWnd* p_Status;
	CItinerary* p_Itinerary;
	AIRX_Flight* p_Flight;
	CTaskbar m_wndTaskbar;
	CTooltipList m_wndTooltipList;
	CTooltipHeader m_wndHeader;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

	void Reload();
	AIRX_Attachment* GetAttachment(INT Index);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNcPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnAdd();
	afx_msg void OnOpen();
	afx_msg void OnSaveAs();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	UINT* m_pSortArray;
	UINT m_Count;

	void Init();
	INT Compare(INT n1, INT n2);
	void Heap(INT Wurzel, INT Anzahl);
	void Sort();
};
