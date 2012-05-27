
// EditFlightDlg.h: Schnittstelle der Klasse EditFlightDlg
//

#pragma once
#include "CItinerary.h"
#include "EditFlightAttachmentsPage.h"
#include "EditFlightRoutePage.h"
#include "EditFlightOtherPage.h"
#include "Flightmap.h"


// EditFlightDlg
//

class EditFlightDlg : public CPropertySheet
{
public:
	EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParent);

	virtual void DoDataExchange(CDataExchange* pDX);

	AIRX_Flight m_Flight;

protected:
	EditFlightRoutePage m_Page0;
	EditFlightOtherPage m_Page1;
	EditFlightAttachmentsPage m_Page2;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
