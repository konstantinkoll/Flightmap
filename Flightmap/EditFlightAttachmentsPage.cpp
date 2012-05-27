
// EditFlightAttachmentsPage.cpp: Implementierung der Klasse EditFlightAttachmentsPage
//

#include "stdafx.h"
#include "EditFlightAttachmentsPage.h"


// EditFlightAttachmentsPage
//

EditFlightAttachmentsPage::EditFlightAttachmentsPage(AIRX_Flight* pFlight)
	: CPropertyPage(IDD_ATTACHMENTS)
{
	ASSERT(pFlight);

	p_Flight = pFlight;
}


BEGIN_MESSAGE_MAP(EditFlightAttachmentsPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL EditFlightAttachmentsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
