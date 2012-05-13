
// CTooltipHeader: Schnittstelle der Klasse CTooltipHeader
//

#pragma once
#include "FMTooltip.h"


// CTooltipHeader
//

class CTooltipHeader : public CHeaderCtrl
{
public:
	CTooltipHeader();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

protected:
	CImageListTransparent m_SortIndicators;
	FMTooltip m_TooltipCtrl;

private:
	BOOL m_Hover;
	INT m_PressedItem;
	INT m_TooltipItem;
};
