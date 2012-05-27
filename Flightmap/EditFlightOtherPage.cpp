
// EditFlightOtherPage.cpp: Implementierung der Klasse EditFlightOtherPage
//

#include "stdafx.h"
#include "EditFlightOtherPage.h"


// EditFlightOtherPage
//

EditFlightOtherPage::EditFlightOtherPage(AIRX_Flight* pFlight)
	: CPropertyPage(IDD_OTHER)
{
	ASSERT(pFlight);

	p_Flight = pFlight;
}


BEGIN_MESSAGE_MAP(EditFlightOtherPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL EditFlightOtherPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
