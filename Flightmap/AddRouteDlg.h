
// AddRouteDlg.h: Schnittstelle der Klasse AddRouteDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// AddRouteDlg
//

class AddRouteDlg : public CDialog
{
public:
	AddRouteDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	CString m_Route;
	AIRX_Flight m_FlightTemplate;

protected:
	CItinerary* p_Itinerary;
	CMFCMaskedEdit m_wndRoute;
	CComboBox m_wndCarrier;
	CMFCMaskedEdit m_wndComment;
	CMFCMaskedEdit m_wndEtixCode;
	CRatingCtrl m_wndRating;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
