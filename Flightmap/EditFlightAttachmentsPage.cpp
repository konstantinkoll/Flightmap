
// EditFlightAttachmentsPage.cpp: Implementierung der Klasse EditFlightAttachmentsPage
//

#include "stdafx.h"
#include "EditFlightAttachmentsPage.h"


// EditFlightAttachmentsPage
//

EditFlightAttachmentsPage::EditFlightAttachmentsPage(AIRX_Flight* pFlight, CItinerary* pItinerary)
	: CPropertyPage(IDD_EDITFLIGHT_ATTACHMENTS)
{
	ASSERT(pFlight);

	p_Flight = pFlight;
	p_Itinerary = pItinerary;
}

void EditFlightAttachmentsPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_FILEVIEW, m_wndFileView);
}


BEGIN_MESSAGE_MAP(EditFlightAttachmentsPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL EditFlightAttachmentsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
