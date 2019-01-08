
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// CAttachmentList
//

struct AttachmentItemData
{
	ItemData Hdr;
	UINT Index;
	AIRX_Attachment* pAttachment;
};

#define FILEVIEWCOLUMNS     4

class CAttachmentList sealed : public CFrontstageItemView
{
public:
	CAttachmentList();

	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void SetAttachments(CItinerary* pItinerary, AIRX_Flight* pFlight=NULL);
	AIRX_Attachment* GetSelectedAttachment() const;
	INT GetSelectedAttachmentIndex() const;
	void EditLabel(INT Index);
	BOOL IsEditing() const;

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ShowTooltip(const CPoint& point);
	virtual COLORREF GetItemTextColor(INT Index) const;
	virtual void UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const;
	virtual void HeaderColumnClicked(UINT Attr);
	virtual void AdjustLayout();
	virtual void FireSelectedItem() const;
	virtual void DrawItemCell(CDC& dc, CRect& rectCell, INT Index, UINT Attr, BOOL Themed);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	void SortItems();

	RECT GetLabelRect(INT Index) const;
	void DestroyEdit(BOOL Accept=FALSE);

	afx_msg void OnDestroy();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;

private:
	static INT __stdcall CompareItems(AttachmentItemData* pData1, AttachmentItemData* pData2, const SortParameters& Parameters);
	AIRX_Attachment* GetAttachment(INT Index) const;
	UINT GetAttachmentIndex(INT Index) const;
	void AddAttachment(UINT Index, AIRX_Attachment& Attachment);
	void UpdateHeader();

	static CString m_SubitemNames[FILEVIEWCOLUMNS];
	static UINT m_SortAttribute;
	static BOOL m_SortDescending;
	CEdit* m_pWndEdit;
	INT m_ColumnOrder[FILEVIEWCOLUMNS];
	INT m_ColumnWidth[FILEVIEWCOLUMNS];
};

inline AIRX_Attachment* CAttachmentList::GetAttachment(INT Index) const
{
	return ((AttachmentItemData*)GetItemData(Index))->pAttachment;
}

inline UINT CAttachmentList::GetAttachmentIndex(INT Index) const
{
	return ((AttachmentItemData*)GetItemData(Index))->Index;
}

inline void CAttachmentList::SortItems()
{
	CFrontstageItemView::SortItems((PFNCOMPARE)CompareItems, HasHeader() ? m_SortAttribute : 0, HasHeader() ? m_SortDescending : FALSE);
}

inline BOOL CAttachmentList::IsEditing() const
{
	return m_pWndEdit!=NULL;
}

inline void CAttachmentList::UpdateHeader()
{
	CFrontstageItemView::UpdateHeader(m_ColumnOrder, m_ColumnWidth);
}


// CFileView
//

class CFileView : public CFrontstageWnd
{
public:
	CFileView();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	void SetAttachments(CItinerary* pItinerary, AIRX_Flight* pFlight=NULL);

protected:
	void Reload();

	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);

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
	CAttachmentList m_wndAttachmentList;
};
