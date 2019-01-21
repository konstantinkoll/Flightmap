
// CDataGrid.h: Schnittstelle der Klasse CDataGrid
//


#pragma once
#include "FMCommDlg.h"
#include "CItinerary.h"


// CDataGrid
//

#define FRS_MATCHCASE           1
#define FRS_MATCHENTIRECELL     2
#define FRS_MATCHCOLUMNONLY     4
#define FRS_REPLACEALL          8

struct FindReplaceSettings
{
	BOOL FirstAction;
	BOOL DoReplace;
	UINT Flags;
	WCHAR SearchTerm[256];
	WCHAR ReplaceTerm[256];
};

struct ViewParameters
{
	INT ColumnOrder[FMAttributeCount];
	INT ColumnWidth[FMAttributeCount];
};

class CDataGrid sealed : public CFrontstageScroller
{
public:
	CDataGrid();

	BOOL Create(CItinerary* pItinerary, CWnd* pParentWnd, UINT nID);
	UINT GetCurrentRow() const;
	void EnsureVisible();
	void SetItinerary(CItinerary* pItinerary);
	BOOL HasSelection(BOOL CurrentLineIfNone=FALSE) const;
	BOOL IsSelected(UINT Index) const;

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void GetHeaderContextMenu(CMenu& Menu);
	virtual BOOL AllowHeaderColumnDrag(UINT Attr) const;
	virtual BOOL AllowHeaderColumnTrack(UINT Attr) const;
	virtual void UpdateHeaderColumnOrder(UINT Attr, INT Position);
	virtual void UpdateHeaderColumnWidth(UINT Attr, INT Width);
	virtual void UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const;
	virtual BOOL GetContextMenu(CMenu& Menu, const CPoint& point);
	virtual void AdjustLayout();
	virtual CPoint PointAtPosition(CPoint point) const;
	virtual void InvalidatePoint(const CPoint& point);
	virtual void ShowTooltip(const CPoint& point);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual void DestroyEdit(BOOL Accept=FALSE);

	void EnsureVisible(const CPoint& Item);
	CRect GetItemRect(const CPoint& Item, BOOL Inflate=TRUE) const;
	INT PartAtPosition(const CPoint& point) const;
	void InvalidateRow(UINT Row);
	void SetFocusItem(const CPoint& FocusItem, BOOL ShiftSelect);
	void SelectItem(UINT Index, BOOL Select=TRUE, BOOL InternalCall=FALSE);
	void EditFlight(const CPoint& Item, INT SelectTab=-1);
	void FindReplace(INT SelectTab=-1);
	void EditCell(BOOL AllowCursor=FALSE, BOOL Delete=FALSE, WCHAR PushChar=L'\0');

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);

	afx_msg void OnCut();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnInsertRow();
	afx_msg void OnDelete();
	afx_msg void OnEditFlight();
	afx_msg void OnAddRoute();
	afx_msg void OnFind();
	afx_msg void OnReplace();
	afx_msg void OnFindReplace();
	afx_msg void OnFindReplaceAgain();
	afx_msg void OnFilter();
	afx_msg void OnSelectAll();
	afx_msg void OnUpdateEditCommands(CCmdUI* pCmdUI);

	afx_msg void OnAutosizeAll();
	afx_msg void OnAutosize();
	afx_msg void OnChooseDetails();
	afx_msg void OnUpdateDetailsCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	BOOL m_EditAllowCursor;
	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	ViewParameters m_ViewParameters;
	CPoint m_FocusItem;
	INT m_HoverPart;
	INT m_SelectionAnchor;
	WORD m_wDay;
	FindReplaceSettings m_FindReplaceSettings;

private:
	INT GetMaxAttributeWidth(UINT Attr) const;
	void AutosizeColumn(UINT Attr);
	void DoDelete();
	void DoCopy(BOOL Cut);
	void DrawCell(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect& rectItem, BOOL Selected);
	void FinishEdit(LPCWSTR pStr, const CPoint& Item);
};

inline UINT CDataGrid::GetCurrentRow() const
{
	return (UINT)m_FocusItem.y;
}

inline void CDataGrid::EnsureVisible()
{
	EnsureVisible(m_FocusItem);
}
