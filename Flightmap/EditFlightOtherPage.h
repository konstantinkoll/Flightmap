
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

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	AIRX_Flight* p_Flight;
	CMFCMaskedEdit m_wndFlightNo;
	CMFCMaskedEdit m_wndCodeshares;
	CMFCMaskedEdit m_wndRegistration;
	CMFCMaskedEdit m_wndAircraftName;
	CMFCMaskedEdit m_wndEtixCode;
	CMFCMaskedEdit m_wndFare;
	CMFCMaskedEdit m_wndAwardMiles;
	CMFCMaskedEdit m_wndStatusMiles;
	CMFCMaskedEdit m_wndSeat;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnChooseColor();
	DECLARE_MESSAGE_MAP()
};
