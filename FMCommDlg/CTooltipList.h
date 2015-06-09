
// CTooltipList: Schnittstelle der Klasse CTooltipList
//

#pragma once
#include "CExplorerList.h"
#include "FMTooltip.h"


// CTooltipList
//

#define REQUEST_TOOLTIP_DATA     1

struct NM_TOOLTIPDATA
{
	NMHDR hdr;
	INT Item;
	BOOL Show;
	HICON hIcon;
	HBITMAP hBitmap;
	INT cx;
	INT cy;
	WCHAR Text[1024];
};

class CTooltipList : public CExplorerList
{
public:
	CTooltipList();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void Init();

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	FMTooltip m_TooltipCtrl;
	BOOL m_Hover;
	INT m_HoverItem;
	INT m_TooltipItem;
};
