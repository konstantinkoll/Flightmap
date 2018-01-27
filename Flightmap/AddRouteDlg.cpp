
// AddRouteDlg.cpp: Implementierung der Klasse AddRouteDlg
//

#include "stdafx.h"
#include "AddRouteDlg.h"
#include "Flightmap.h"


// AddRouteDlg
//

AddRouteDlg::AddRouteDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: FMDialog(IDD_ADDROUTE, pParentWnd)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;

	CItinerary::ResetFlight(m_FlightTemplate);
}

void AddRouteDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ROUTE, m_wndRoute);
	DDX_Control(pDX, IDC_CARRIER, m_wndCarrier);
	DDX_MaskedText(pDX, IDC_COMMENT, m_wndComment, 22, &m_FlightTemplate);
	DDX_MaskedText(pDX, IDC_ETIXCODE, m_wndEtixCode, 16, &m_FlightTemplate);
	DDX_Control(pDX, IDC_RATING, m_wndRating);

	if (pDX->m_bSaveAndValidate)
	{
		// Route
		m_wndRoute.GetWindowText(m_Route);

		// Template
		CString tmpStr;
		m_wndCarrier.GetWindowText(tmpStr);

		StringToAttribute(tmpStr, m_FlightTemplate, 7);

		INT Class = 0;
		DDX_Radio(pDX, IDC_CLASS_Y, Class);
		m_FlightTemplate.Class = (Class==0) ? AIRX_Economy : (Class==1) ? AIRX_PremiumEconomy : (Class==2) ? AIRX_Business : (Class==3) ? AIRX_First : (Class==4) ? AIRX_Crew : (Class==5) ? AIRX_Charter : AIRX_Unknown;

		m_FlightTemplate.Flags |= m_wndRating.GetRating()<<FMAttributes[21].DataParameter;

		if (((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_AwardFlight;

		if (((CButton*)GetDlgItem(IDC_LEISURETRIP))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_LeisureTrip;

		if (((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_BusinessTrip;

		if (((CButton*)GetDlgItem(IDC_CANCELLED))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_Cancelled;
	}
}

BOOL AddRouteDlg::InitDialog()
{
	// Route
	m_wndRoute.SetCueBanner(CString((LPCSTR)IDS_CUEBANNER_ROUTE));

	// Carrier
	p_Itinerary->PrepareCarrierCtrl(m_wndCarrier);

	// Rating
	m_wndRating.SetRating(0);

	return TRUE;
}
