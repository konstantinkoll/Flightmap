
// FindReplaceDlg.cpp: Implementierung der Klasse FindReplaceDlg
//

#include "stdafx.h"
#include "FindReplaceDlg.h"


// FindReplaceDlg
//

static UINT LastPageSelected = 0;

FindReplaceDlg::FindReplaceDlg(CWnd* pParentWnd, INT iSelectPage, UINT Attr)
	: CPropertySheet(IDS_FINDREPLACE, pParentWnd, iSelectPage)
{
	m_FindReplaceSettings = theApp.m_FindReplaceSettings;

	m_pPages[0] = new FindReplacePage(&m_FindReplaceSettings, FMAttributes[Attr].Searchable);
	m_pPages[0]->Construct(IDD_FIND);

	m_pPages[1] = new FindReplacePage(&m_FindReplaceSettings, FMAttributes[Attr].Searchable && FMAttributes[Attr].Editable);
	m_pPages[1]->Construct(IDD_REPLACE);

	AddPage(m_pPages[0]);
	AddPage(m_pPages[1]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}


BEGIN_MESSAGE_MAP(FindReplaceDlg, CPropertySheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void FindReplaceDlg::OnDestroy()
{
	for (UINT a=0; a<2; a++)
	{
		m_pPages[a]->DestroyWindow();
		delete m_pPages[a];
	}
}
