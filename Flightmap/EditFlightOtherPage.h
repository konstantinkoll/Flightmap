
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
	EditFlightOtherPage(AIRX_Flight* pFlight);

protected:
	AIRX_Flight* p_Flight;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
