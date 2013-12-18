
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
	FindReplaceDlg(CWnd* pParentWnd, INT iSelectPage);

protected:
	FindReplacePage m_Pages[2];

	DECLARE_MESSAGE_MAP()
};
