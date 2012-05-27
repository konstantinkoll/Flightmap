
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
	EditFlightRoutePage(AIRX_Flight* pFlight);

protected:
	AIRX_Flight* p_Flight;

	void SelectAirport(UINT nID, CHAR* pIATA);
	void DisplayLocation(const FMGeoCoordinates Location);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnFromSelect();
	afx_msg void OnToSelect();
	afx_msg void OnWaypoint();
	DECLARE_MESSAGE_MAP()
};
