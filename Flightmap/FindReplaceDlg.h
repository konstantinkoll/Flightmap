
// FindReplaceDlg.h: Schnittstelle der Klasse FindReplaceDlg
//

#pragma once
#include "FindReplacePage.h"


// FindReplaceDlg
//

class FindReplaceDlg : public CPropertySheet
{
public:
	FindReplaceDlg(INT iSelectPage, UINT Attr, CWnd* pParentWnd=NULL);

	FindReplaceSettings m_FindReplaceSettings;

protected:
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

	FindReplacePage* m_pPages[2];
};
