
// AddRouteDlg.cpp: Implementierung der Klasse AddRouteDlg
//

#include "stdafx.h"
#include "AddRouteDlg.h"
#include "Flightmap.h"


// AddRouteDlg
//

AddRouteDlg::AddRouteDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_ADDROUTE, pParentWnd)
{
	p_Itinerary = pItinerary;
	ResetFlight(m_FlightTemplate);
}

void AddRouteDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_CARRIER, m_wndCarrier);
	DDX_Control(pDX, IDC_ROUTE, m_wndRoute);
	DDX_MaskedText(pDX, IDC_COMMENT, m_wndComment, 22, &m_FlightTemplate);
	DDX_MaskedText(pDX, IDC_ETIXCODE, m_wndEtixCode, 16, &m_FlightTemplate);
	DDX_Control(pDX, IDC_RATING, m_wndRating);

	if (pDX->m_bSaveAndValidate)
	{
		m_wndRoute.GetWindowText(m_Route);

		CString tmpStr;
		m_wndCarrier.GetWindowText(tmpStr);
		StringToAttribute(tmpStr.GetBuffer(), m_FlightTemplate, 7);

		INT Class = 0;
		DDX_Radio(pDX, IDC_CLASS_Y, Class);
		m_FlightTemplate.Class = (Class==0) ? AIRX_Economy : (Class==1) ? AIRX_PremiumEconomy : (Class==2) ? AIRX_Business : (Class==3) ? AIRX_First : (Class==4) ? AIRX_Crew : (Class==5) ? AIRX_Charter : AIRX_Unknown;

		m_FlightTemplate.Flags &= ~((0xF<<FMAttributes[21].DataParameter) | AIRX_LeisureTrip | AIRX_BusinessTrip | AIRX_AwardFlight);
		m_FlightTemplate.Flags |= m_wndRating.GetRating()<<FMAttributes[21].DataParameter;

		if (((CButton*)GetDlgItem(IDC_LEISURETRIP))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_LeisureTrip;
		if (((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_BusinessTrip;
		if (((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->GetCheck())
			m_FlightTemplate.Flags |= AIRX_AwardFlight;
	}
}


BEGIN_MESSAGE_MAP(AddRouteDlg, CDialog)
END_MESSAGE_MAP()

BOOL AddRouteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadDialogIcon(IDI_ADDROUTE);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Route
	CString tmpStr((LPCSTR)IDS_CUEBANNER_ROUTE);
	m_wndRoute.SetCueBanner(tmpStr);

	// Carrier
	PrepareCarrierCtrl(&m_wndCarrier, p_Itinerary);

	// Rating
	m_wndRating.SetRating(0);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
