
// StatisticsDlg.h: Schnittstelle der Klasse StatisticsDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// StatisticsDlg
//

#define WM_UPDATESTATISTICS     WM_USER+101

class StatisticsDlg : public CDialog
{
public:
	StatisticsDlg(CItinerary* pItinerary, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CItinerary* p_Itinerary;
	CImageListTransparent m_SeatIcons;
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CExplorerList m_wndListClass;
	CListCtrl m_wndListRoute;
	CListCtrl m_wndListAirport;
	CListCtrl m_wndListCarrier;
	CListCtrl m_wndListEquipment;

	void UpdateStatistics();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnPostUpdateStatistics();
	afx_msg void OnUpdateStatistics();
	afx_msg void OnSelectIATA();
	afx_msg void OnSortLists(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
