
// FilterDlg.h: Schnittstelle der Klasse FilterDlg
//

#pragma once
#include "CItinerary.h"
#include "Flightmap.h"


// FilterDlg
//

struct FlightFilter
{
	CHAR Airport[4];
	WCHAR Carrier[256];
	WCHAR Equipment[256];
	BOOL Business;
	BOOL Leisure;
	UCHAR Rating;
	INT SortBy;
	BOOL Descending;
};

class FilterDlg : public CDialog
{
public:
	FilterDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	FlightFilter m_Filter;

protected:
	CItinerary* p_Itinerary;
	CImageListTransparent m_SeatIcons;
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CListCtrl m_wndSortAttributes;
	CButton m_wndAscending;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectIATA();
	DECLARE_MESSAGE_MAP()
};
