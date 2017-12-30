
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

class CDataGrid : public CFrontstageWnd
{
public:
	CDataGrid();
	~CDataGrid();

	BOOL Create(CItinerary* pItinerary, CWnd* pParentWnd, UINT nID);
	void SetItinerary(CItinerary* pItinerary);
	BOOL HasSelection(BOOL CurrentLineIfNone=FALSE) const;
	BOOL IsSelected(UINT Index) const;
	UINT GetCurrentRow() const;
	void EnsureVisible();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CPoint PointAtPosition(CPoint point) const;
	virtual void InvalidatePoint(const CPoint& Point);
	virtual void ShowTooltip(const CPoint& point);

	void AdjustLayout();
	void AdjustHeader();
	void EditCell(BOOL AllowCursor=FALSE, BOOL Delete=FALSE, WCHAR PushChar=L'\0', CPoint Item=CPoint(-1, -1));
	void EditFlight(CPoint Item=CPoint(-1, -1), INT SelectTab=-1);
	void EnsureVisible(CPoint Item);
	INT PartAtPosition(const CPoint& point) const;
	void InvalidatePoint(UINT Row, UINT Attr);
	void InvalidateRow(UINT Row);
	void SetFocusItem(const CPoint& FocusItem, BOOL ShiftSelect);
	void SelectItem(UINT Index, BOOL Select=TRUE, BOOL InternalCall=FALSE);
	void FindReplace(INT SelectTab=-1);
	void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

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

	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	BOOL m_EditAllowCursor;
	CTooltipHeader m_wndHeader;
	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	ViewParameters m_ViewParameters;
	UINT m_HeaderHeight;
	UINT m_RowHeight;
	CPoint m_FocusItem;
	INT m_HoverPart;
	INT m_HeaderItemClicked;
	BOOL m_IgnoreHeaderItemChange;
	INT m_SelectionAnchor;
	WORD m_wDay;
	FindReplaceSettings m_FindReplaceSettings;

private:
	void ResetScrollbars();
	void AdjustScrollbars();
	void DoCopy(BOOL Cut);
	void DoDelete();
	void DrawCell(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect& rectItem, BOOL Selected);
	INT GetMaxAttributeWidth(UINT Attr) const;
	void AutosizeColumn(UINT Attr);
	void FinishEdit(LPWSTR pStr, const CPoint& Item);
	void DestroyEdit(BOOL Accept=FALSE);

	CMFCMaskedEdit* m_pWndEdit;
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_HScrollPos;
	INT m_VScrollPos;
};

inline void CDataGrid::EnsureVisible()
{
	EnsureVisible(CPoint(-1, -1));
}
