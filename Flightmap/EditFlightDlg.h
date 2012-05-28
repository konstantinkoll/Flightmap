
// EditFlightDlg.h: Schnittstelle der Klasse EditFlightDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// EditFlightDlg
//

#define EditFlightPages     3

class EditFlightDlg : public CPropertySheet
{
public:
	EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParent);

	AIRX_Flight m_Flight;

protected:
	CPropertyPage* m_Pages[EditFlightPages];

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
