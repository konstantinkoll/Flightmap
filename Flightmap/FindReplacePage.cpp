
// FindReplacePage.cpp: Implementierung der Klasse FindReplacePage
//

#include "stdafx.h"
#include "FindReplacePage.h"
#include "Flightmap.h"


// FindReplacePage
//

void FindReplacePage::DoDataExchange(CDataExchange* pDX)
{
}


BEGIN_MESSAGE_MAP(FindReplacePage, CPropertyPage)
END_MESSAGE_MAP()

BOOL FindReplacePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
