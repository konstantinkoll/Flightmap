
// StatisticsDlg.h: Schnittstelle der Klasse StatisticsDlg
//

#pragma once
#include "CItinerary.h"


// CClassesList
//

struct ClassItemData
{
	ItemData Hdr;
	LPCWSTR DisplayName;
	UINT Flights;
	DOUBLE Distance;
	INT Category;
	INT iIcon;
};

class CClassesList sealed : public CFrontstageItemView
{
public:
	CClassesList();

	void SetClasses(UINT* pFlights, DOUBLE* pDistances);

protected:
	virtual void AdjustLayout();
	virtual INT GetItemCategory(INT Index) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	ClassItemData* GetClassItemData(INT Index) const;
	void AddClass(UINT nID, UINT Flights, DOUBLE Distance);

	static CString m_MaskFlightsSingular;
	static CString m_MaskFlightsPlural;
	static CString m_Names[6];
	static CIcons m_SeatIcons;
};

inline ClassItemData* CClassesList::GetClassItemData(INT Index) const
{
	return (ClassItemData*)GetItemData(Index);
}


// StatisticsDlg
//

#define WM_UPDATESTATISTICS     WM_USER+101

class StatisticsDlg : public CDialog
{
public:
	StatisticsDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	void UpdateStatistics();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnPostUpdateStatistics();
	afx_msg void OnUpdateStatistics();
	afx_msg void OnSelectIATA();
	afx_msg void OnSortLists(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	CMFCMaskedEdit m_wndFilterAirport;
	CComboBox m_wndFilterCarrier;
	CComboBox m_wndFilterEquipment;
	CRatingCtrl m_wndFilterRating;
	CClassesList m_wndListClass;
	CListCtrl m_wndListRoute;
	CListCtrl m_wndListAirport;
	CListCtrl m_wndListCarrier;
	CListCtrl m_wndListEquipment;
};
