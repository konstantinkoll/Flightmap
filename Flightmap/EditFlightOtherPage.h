
// EditFlightOtherPage.h: Schnittstelle der Klasse EditFlightOtherPage
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// EditFlightOtherPage
//

class EditFlightOtherPage : public CPropertyPage
{
public:
	EditFlightOtherPage();

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
