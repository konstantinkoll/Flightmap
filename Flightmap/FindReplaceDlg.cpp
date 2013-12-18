
// FindReplaceDlg.cpp: Implementierung der Klasse FindReplaceDlg
//

#include "stdafx.h"
#include "FindReplaceDlg.h"


// FindReplaceDlg
//

static UINT LastPageSelected = 0;

FindReplaceDlg::FindReplaceDlg(CWnd* pParentWnd, INT iSelectPage)
	: CPropertySheet(IDS_FINDREPLACE, pParentWnd, iSelectPage)
{
	m_Pages[0].Construct(IDD_FIND);
	m_Pages[1].Construct(IDD_REPLACE);

	AddPage(&m_Pages[0]);
	AddPage(&m_Pages[1]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}


BEGIN_MESSAGE_MAP(FindReplaceDlg, CPropertySheet)
END_MESSAGE_MAP()
