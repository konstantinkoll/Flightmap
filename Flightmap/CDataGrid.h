
// CDataGrid.h: Schnittstelle der Klasse CDataGrid
//


#pragma once
#include "CItinerary.h"


// CDataGrid
//

struct ViewParameters
{
	INT ColumnOrder[FMAttributeCount];
	INT ColumnWidth[FMAttributeCount];
};

class CDataGrid : public CWnd
{
public:
	CDataGrid();
	~CDataGrid();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetItinerary(CItinerary* pItinerary);

protected:
	CItinerary* p_Itinerary;
	CEdit* p_Edit;
	CTooltipHeader m_wndHeader;
	ViewParameters m_ViewParameters;
	HTHEME hThemeList;
	HTHEME hThemeButton;
	FMTooltip m_TooltipCtrl;
	UINT m_HeaderHeight;
	UINT m_RowHeight;
	CPoint m_SelectedItem;
	CPoint m_HotItem;
	CPoint m_EditLabel;
	BOOL m_Hover;
	BOOL m_SpacePressed;
	INT m_HeaderItemClicked;
	BOOL m_IgnoreHeaderItemChange;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void AdjustLayout();
	void AdjustHeader();
	void EnsureVisible(CPoint item=CPoint(-1, -1));
	void ResetScrollbars();
	void AdjustScrollbars();
	BOOL HitTest(CPoint point, CPoint* item);
	void InvalidateItem(CPoint Item);
	void SelectItem(CPoint Item);
	void DrawItem(CDC& dc, AIRX_Flight& Flight, UINT Attr, CRect rect);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnAutosizeAll();
	afx_msg void OnAutosize();
	afx_msg void OnChooseDetails();
	afx_msg void OnUpdateDetailsCommands(CCmdUI* pCmdUI);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

private:
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_HScrollPos;
	INT m_VScrollPos;

	void AutosizeColumn(UINT Attr);
	void DestroyEdit(BOOL Accept=FALSE);
};
