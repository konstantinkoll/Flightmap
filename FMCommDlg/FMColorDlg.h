
// FMColorDlg.h: Schnittstelle der Klasse FMColorDlg
//

#pragma once
#include "FMDialog.h"


// FMColorDlg
//

class FMColorDlg : public CColorDialog
{
public:
	FMColorDlg(COLORREF clrInit=0, DWORD dwFlags=0, CWnd* pParentWnd=NULL);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	HICON hIconL;
	HICON hIconS;
};
