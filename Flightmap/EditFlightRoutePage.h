
// EditFlightRoutePage.h: Schnittstelle der Klasse EditFlightRoutePage
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// EditFlightRoutePage
//

class EditFlightRoutePage : public CPropertyPage
{
public:
	EditFlightRoutePage();

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
