
// InspectDlg.h: Schnittstelle der Klasse InspectDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// InspectDlg
//

class InspectDlg : public CDialog
{
public:
	InspectDlg(CItinerary* pItinerary, CWnd* pParent=NULL);

protected:
	CItinerary* p_Itinerary;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
