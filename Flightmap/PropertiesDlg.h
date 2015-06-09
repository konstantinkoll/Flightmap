
// PropertiesDlg.h: Schnittstelle der Klasse PropertiesDlg
//

#pragma once
#include "CItinerary.h"


// PropertiesDlg
//

class PropertiesDlg : public CDialog
{
public:
	PropertiesDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CItinerary* p_Itinerary;
};
