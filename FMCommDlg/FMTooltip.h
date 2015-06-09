
// FMTooltip.h: Schnittstelle der Klasse FMTooltip
//

#pragma once
#include "IATA.h"


// FMTooltip
//

#define FMHOVERTIME     850

class FMTooltip : public CWnd
{
public:
	FMTooltip();

	virtual BOOL Create(CWnd* pWndParent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Track(CPoint point, HICON hIcon, HBITMAP hBitmap, CSize Size, const CString& strCaption, CString strText, BOOL DrawBorder=FALSE);
	void Track(CPoint point, FMAirport* pAirport, CString strText);
	void Track(CPoint point, CHAR* Code, CString strText);
	void Deactivate();

protected:
	void Hide();

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	BOOL m_Themed;
	BOOL m_Flat;
	HICON m_Icon;
	HBITMAP m_Bitmap;
	CSize m_Size;
	CString m_strCaption;
	CString m_strText;
	INT m_TextHeight;
	BOOL m_DrawBorder;
};
