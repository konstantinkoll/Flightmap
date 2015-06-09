
// FMColorDlg.h: Schnittstelle der Klasse FMColorDlg
//

#pragma once


// FMColorDlg
//

class FMColorDlg : public CColorDialog
{
public:
	FMColorDlg(CWnd* pParentWnd=NULL, COLORREF clrInit=0, DWORD dwFlags=0, CString Caption=_T(""));

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
};
