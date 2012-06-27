
// StatisticsDlg.h: Schnittstelle der Klasse StatisticsDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// StatisticsDlg
//

class StatisticsDlg : public CDialog
{
public:
	StatisticsDlg(CItinerary* pItinerary, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

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
