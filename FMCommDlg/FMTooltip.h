
// FMTooltip.h: Schnittstelle der Klasse FMTooltip
//

#pragma once


// FMTooltip
//

#define FMHOVERTIME     850

class FMTooltip : public CWnd
{
public:
	FMTooltip();

	virtual BOOL Create(CWnd* pWndParent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Track(CPoint point, HICON hIcon, CSize szIcon, const CString& strCaption, CString strText);
	void Deactivate();

protected:
	void Hide();

	BOOL m_Themed;
	HICON m_Icon;
	CSize m_szIcon;
	CString m_strCaption;
	CString m_strText;
	INT m_TextHeight;

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
