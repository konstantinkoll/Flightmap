
// FilterDlg.h: Schnittstelle der Klasse FilterDlg
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// CSortList
//

struct AttributeItemData
{
	ItemData Hdr;
	UINT Attr;
	WCHAR Name[256];
};

class CSortList sealed : public CFrontstageItemView
{
public:
	CSortList();

	void SetAttributes();
	INT GetSelectedSortAttribute() const;

protected:
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	AttributeItemData* GetAttributeItemData(INT Index) const;
	void AddAttribute(UINT Attr, LPCWSTR Name);
};

inline AttributeItemData* CSortList::GetAttributeItemData(INT Index) const
{
	return (AttributeItemData*)GetItemData(Index);
}


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

	INT GetSelectedSortAttribute() const;

	afx_msg void OnSelectIATA();
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CComboBox m_wndFilterMonth;
	CSortList m_wndSortAttributes;
	CComboBox m_wndSortDirection;
};

inline INT FilterDlg::GetSelectedSortAttribute() const
{
	return m_wndSortAttributes.GetSelectedSortAttribute();
}
