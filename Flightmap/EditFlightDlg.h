
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
	EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParentWnd, CItinerary* pItinerary=NULL, INT iSelectPage=-1);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	AIRX_Flight m_Flight;

protected:
	CPropertyPage* m_pPages[EditFlightPages];

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
