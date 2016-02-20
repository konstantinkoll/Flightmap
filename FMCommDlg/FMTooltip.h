
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

	static void AppendAttribute(CString& Str, const CString& Name, const CString& Value);
	static void AppendAttribute(CString& Str, UINT ResID, const CString& Value);
	static void AppendAttribute(CString& Str, UINT ResID, LPCSTR pValue);
	static void AppendAttribute(WCHAR* pStr, SIZE_T cCount, const CString& Name, const CString& Value);
	static void AppendAttribute(WCHAR* pStr, SIZE_T cCount, UINT ResID, const CString& Value);
	static void AppendAttribute(WCHAR* pStr, SIZE_T cCount, UINT ResID, const CHAR* pValue);

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CRect m_ContentRect;
};
