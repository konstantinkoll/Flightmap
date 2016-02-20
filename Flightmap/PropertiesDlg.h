
// PropertiesDlg.h: Schnittstelle der Klasse PropertiesDlg
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// PropertiesDlg
//

class PropertiesDlg : public FMDialog
{
public:
	PropertiesDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	CItinerary* p_Itinerary;
};
