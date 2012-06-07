
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

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	AIRX_Flight* p_Flight;
	CMFCMaskedEdit m_wndFromIATA;
	CMFCMaskedEdit m_wndFromTime;
	CMFCMaskedEdit m_wndFromGate;
	CMFCMaskedEdit m_wndToIATA;
	CMFCMaskedEdit m_wndToTime;
	CMFCMaskedEdit m_wndToGate;
	CMFCMaskedEdit m_wndFlighttime;
	CMFCMaskedEdit m_wndComment;

	void SelectAirport(UINT nEditID, CHAR* pIATA, UINT nDisplayID);
	void DisplayLocation(const FMGeoCoordinates Location);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnFromSelect();
	afx_msg void OnToSelect();
	afx_msg void OnWaypoint();
	afx_msg void OnGroundTransportation();
	afx_msg void OnCheckWaypoint();
	DECLARE_MESSAGE_MAP()

private:
	void DisplayAirport(UINT nID, FMAirport* pAirport);
	void DisplayAirport(UINT nID, CHAR* pIATA);
};
