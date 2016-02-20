
// AddRouteDlg.h: Schnittstelle der Klasse AddRouteDlg
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// AddRouteDlg
//

class AddRouteDlg : public FMDialog
{
public:
	AddRouteDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	CString m_Route;
	AIRX_Flight m_FlightTemplate;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	CItinerary* p_Itinerary;
	CMFCMaskedEdit m_wndRoute;
	CComboBox m_wndCarrier;
	CMFCMaskedEdit m_wndComment;
	CMFCMaskedEdit m_wndEtixCode;
	CRatingCtrl m_wndRating;
};
