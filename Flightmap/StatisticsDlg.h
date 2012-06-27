
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
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CExplorerList m_wndListClass;

	void UpdateStatistics();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
