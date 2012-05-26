
// EditFlightRoutePage.cpp: Implementierung der Klasse EditFlightRoutePage
//

#include "stdafx.h"
#include "EditFlightRoutePage.h"


// EditFlightRoutePage
//

EditFlightRoutePage::EditFlightRoutePage()
	: CPropertyPage(IDD_FLIGHTROUTE_FROMTO)
{
}


BEGIN_MESSAGE_MAP(EditFlightRoutePage, CPropertyPage)
END_MESSAGE_MAP()

BOOL EditFlightRoutePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
