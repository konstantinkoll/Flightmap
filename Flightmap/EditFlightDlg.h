
// EditFlightDlg.h: Schnittstelle der Klasse EditFlightDlg
//

#pragma once
#include "CItinerary.h"


// EditFlightDlg
//

#define EDITFLIGHTPAGES     3

class EditFlightDlg : public CPropertySheet
{
public:
	EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParentWnd=NULL, CItinerary* pItinerary=NULL, INT iSelectPage=-1);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	AIRX_Flight m_Flight;

protected:
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

	CPropertyPage* m_pPages[EDITFLIGHTPAGES];
};
