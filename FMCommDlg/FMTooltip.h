
// FMTooltip.h: Schnittstelle der Klasse FMTooltip
//

#pragma once


// FMTooltip
//

#define HOVERTIME     850

class FMTooltip : public CWnd
{
public:
	FMTooltip();

	BOOL Create();

	void ShowTooltip(const CPoint& point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void HideTooltip();

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CRect m_ContentRect;
};
