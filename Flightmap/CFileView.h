
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// CFileView
//

class CFileView : public CFrontstageWnd
{
public:
	CFileView();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	void SetItinerary(CItinerary* pItinerary, AIRX_Flight* pFlight=NULL);

protected:
	void Reload();
	AIRX_Attachment* GetAttachment(INT Index);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnAdd();
	afx_msg void OnOpen();
	afx_msg void OnSaveAs();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	AIRX_Flight* p_Flight;
	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CTaskbar m_wndTaskbar;
	CExplorerList m_wndExplorerList;
	CTooltipHeader m_wndHeader;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

private:
	void Init();
	INT Compare(INT n1, INT n2);
	static void Swap(UINT& Eins, UINT& Zwei);
	void Heap(INT Wurzel, INT Anzahl);
	void Sort();

	UINT* m_pSortArray;
	UINT m_Count;
};

inline void CFileView::Swap(UINT& Eins, UINT& Zwei)
{
	UINT Temp = Eins;
	Eins = Zwei;
	Zwei = Temp;
}
