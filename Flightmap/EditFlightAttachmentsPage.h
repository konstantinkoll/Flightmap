
// EditFlightAttachmentsPage.h: Schnittstelle der Klasse EditFlightAttachmentsPage
//

#pragma once
#include "CFileView.h"
#include "CItinerary.h"


// EditFlightAttachmentsPage
//

class EditFlightAttachmentsPage : public CPropertyPage
{
public:
	EditFlightAttachmentsPage(AIRX_Flight* pFlight, CItinerary* pItinerary);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	AIRX_Flight* p_Flight;
	CFileView m_wndFileView;
	CItinerary* p_Itinerary;
};
