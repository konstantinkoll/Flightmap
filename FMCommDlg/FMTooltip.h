
// FMTooltip.h: Schnittstelle der Klasse FMTooltip
//

#pragma once


// FMTooltip
//

#define FMHOVERTIME     850

class FMTooltip : public CWnd
{
public:
	BOOL Create();

	void ShowTooltip(CPoint point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void HideTooltip();
};
