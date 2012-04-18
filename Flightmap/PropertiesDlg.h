
// PropertiesDlg.h: Schnittstelle der Klasse PropertiesDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// PropertiesDlg
//

class PropertiesDlg : public CDialog
{
public:
	PropertiesDlg(CItinerary* pItinerary, CWnd* pParent);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CItinerary* p_Itinerary;
};
