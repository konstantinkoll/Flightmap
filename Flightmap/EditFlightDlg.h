
// EditFlightDlg.h: Schnittstelle der Klasse EditFlightDlg
//

#pragma once
#include "CFileView.h"
#include "CItinerary.h"
#include "FMCommDlg.h"


// EditFlightDlg
//

class EditFlightDlg : public FMTabbedDialog
{
public:
	EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParentWnd=NULL, CItinerary* pItinerary=NULL, INT SelectTab=-1);

	AIRX_Flight m_Flight;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	void SelectAirport(UINT nEditID, LPSTR pIATA);
	void DisplayLocation(const FMGeoCoordinates& Location);

	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnFromSelect();
	afx_msg void OnToSelect();
	afx_msg void OnWaypoint();
	afx_msg void OnCheckAirports();

	afx_msg void OnChooseColor();
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	static UINT m_LastTab;

	CMFCMaskedEdit m_wndFromIATA;
	CMFCMaskedEdit m_wndFromTime;
	CMFCMaskedEdit m_wndFromGate;
	CMFCMaskedEdit m_wndToIATA;
	CMFCMaskedEdit m_wndToTime;
	CMFCMaskedEdit m_wndToGate;
	CMFCMaskedEdit m_wndFlighttime;
	CMFCMaskedEdit m_wndComment;

	CComboBox m_wndCarrier;
	CComboBox m_wndEquipment;
	CMFCMaskedEdit m_wndFlightNo;
	CMFCMaskedEdit m_wndCodeshares;
	CMFCMaskedEdit m_wndRegistration;
	CMFCMaskedEdit m_wndAircraftName;
	CMFCMaskedEdit m_wndEtixCode;
	CMFCMaskedEdit m_wndFare;
	CMFCMaskedEdit m_wndAwardMiles;
	CMFCMaskedEdit m_wndStatusMiles;
	CMFCMaskedEdit m_wndUpgradeVoucher;
	CColorIndicator m_wndColorIndicator;
	CRatingCtrl m_wndRating;
	CMFCMaskedEdit m_wndSeat;

	CFileView m_wndFileView;

private:
	void GetIATACode(UINT nID, LPSTR pIATA);
	void DisplayAirport(UINT nID, FMAirport* pAirport);
	void DisplayAirport(UINT nID, LPCSTR pIATA);
	void DisplayAirport(UINT nDisplayID, UINT nEditID);
};
