
// EditFlightAttachmentsPage.h: Schnittstelle der Klasse EditFlightAttachmentsPage
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// EditFlightAttachmentsPage
//

class EditFlightAttachmentsPage : public CPropertyPage
{
public:
	EditFlightAttachmentsPage();

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
