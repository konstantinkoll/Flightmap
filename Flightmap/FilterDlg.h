
// FilterDlg.h: Schnittstelle der Klasse FilterDlg
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


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
	UINT DepartureMonth;
	UINT DepartureYear;
};

class FilterDlg : public FMDialog
{
public:
	FilterDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	FlightFilter m_Filter;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnSelectIATA();
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	CImageList m_SeatIcons;
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CComboBox m_wndFilterMonth;
	CExplorerList m_wndSortAttributes;
	CComboBox m_wndSortDirection;
};
