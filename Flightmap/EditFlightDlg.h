
// EditFlightDlg.h: Schnittstelle der Klasse EditFlightDlg
//

#pragma once
#include "CItinerary.h"
#include "EditFlightRoutePage.h"
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

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
