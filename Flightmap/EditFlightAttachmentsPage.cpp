
// EditFlightAttachmentsPage.cpp: Implementierung der Klasse EditFlightAttachmentsPage
//

#include "stdafx.h"
#include "EditFlightAttachmentsPage.h"


// EditFlightAttachmentsPage
//

EditFlightAttachmentsPage::EditFlightAttachmentsPage()
	: CPropertyPage(IDD_ATTACHMENTS)
{
}


BEGIN_MESSAGE_MAP(EditFlightAttachmentsPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL EditFlightAttachmentsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
