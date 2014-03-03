
// FindReplaceDlg.h: Schnittstelle der Klasse FindReplaceDlg
//

#pragma once
#include "Flightmap.h"
#include "FindReplacePage.h"


// FindReplaceDlg
//

class FindReplaceDlg : public CPropertySheet
{
public:
	FindReplaceDlg(CWnd* pParentWnd, INT iSelectPage, UINT Attr);

	FindReplaceSettings m_FindReplaceSettings;

protected:
	FindReplacePage* m_pPages[2];

	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
