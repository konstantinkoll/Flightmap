
// EditFlightOtherPage.cpp: Implementierung der Klasse EditFlightOtherPage
//

#include "stdafx.h"
#include "EditFlightOtherPage.h"


// EditFlightOtherPage
//

EditFlightOtherPage::EditFlightOtherPage(AIRX_Flight* pFlight, CItinerary* pItinerary)
	: CPropertyPage(IDD_OTHER)
{
	ASSERT(pFlight);

	p_Flight = pFlight;
	p_Itinerary = pItinerary;
}

void EditFlightOtherPage::DoDataExchange(CDataExchange* pDX)
{
	INT Class;
	if (!pDX->m_bSaveAndValidate && (p_Flight->Class!=AIRX_Unknown))
	{
		Class = (p_Flight->Class==AIRX_Economy) ? 0 : (p_Flight->Class==AIRX_PremiumEconomy) ? 1 : (p_Flight->Class==AIRX_Business) ? 2 : (p_Flight->Class==AIRX_First) ? 3 : 4;
		DDX_Radio(pDX, IDC_CLASS_Y, Class);
	}

	DDX_Control(pDX, IDC_CARRIER, m_wndCarrier);
	DDX_Control(pDX, IDC_EQUIPMENT, m_wndEquipment);
	DDX_MaskedText(pDX, IDC_FLIGHTNO, m_wndFlightNo, 8, p_Flight);
	DDX_MaskedText(pDX, IDC_CODESHARES, m_wndCodeshares, 9, p_Flight);
	DDX_MaskedText(pDX, IDC_REGISTRATION, m_wndRegistration, 11, p_Flight);
	DDX_MaskedText(pDX, IDC_AIRCRAFTNAME, m_wndAircraftName, 12, p_Flight);
	DDX_MaskedText(pDX, IDC_ETIXCODE, m_wndEtixCode, 16, p_Flight);
	DDX_MaskedText(pDX, IDC_FARE, m_wndFare, 17, p_Flight);
	DDX_MaskedText(pDX, IDC_AWARDMILES, m_wndAwardMiles, 18, p_Flight);
	DDX_MaskedText(pDX, IDC_STATUSMILES, m_wndStatusMiles, 19, p_Flight);
	DDX_MaskedText(pDX, IDC_SEAT, m_wndSeat, 14, p_Flight);
	DDX_Control(pDX, IDC_RATING, m_wndRating);

	if (pDX->m_bSaveAndValidate)
	{
		CString tmpStr;
		m_wndCarrier.GetWindowText(tmpStr);
		StringToAttribute(tmpStr.GetBuffer(), *p_Flight, 7);

		m_wndEquipment.GetWindowText(tmpStr);
		StringToAttribute(tmpStr.GetBuffer(), *p_Flight, 10);

		DDX_Radio(pDX, IDC_CLASS_Y, Class);
		p_Flight->Class = (Class==0) ? AIRX_Economy : (Class==1) ? AIRX_PremiumEconomy : (Class==2) ? AIRX_Business : (Class==3) ? AIRX_First : (Class==4) ? AIRX_Crew : AIRX_Unknown;

		p_Flight->Flags &= ~((0xF<<FMAttributes[21].DataParameter) | AIRX_LeisureTrip | AIRX_BusinessTrip | AIRX_AwardFlight);
		p_Flight->Flags |= m_wndRating.GetRating()<<FMAttributes[21].DataParameter;

		if (((CButton*)GetDlgItem(IDC_LEISURETRIP))->GetCheck())
			p_Flight->Flags |= AIRX_LeisureTrip;
		if (((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->GetCheck())
			p_Flight->Flags |= AIRX_BusinessTrip;
		if (((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->GetCheck())
			p_Flight->Flags |= AIRX_AwardFlight;
	}
}


BEGIN_MESSAGE_MAP(EditFlightOtherPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChooseColor)
END_MESSAGE_MAP()

BOOL EditFlightOtherPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Carrier
	PrepareCarrierCtrl(&m_wndCarrier, p_Itinerary);

	if (m_wndCarrier.SelectString(-1, p_Flight->Carrier)==-1)
		m_wndCarrier.SetWindowText(p_Flight->Carrier);

	// Equipment
	PrepareEquipmentCtrl(&m_wndEquipment, p_Itinerary);

	if (m_wndEquipment.SelectString(-1, p_Flight->Equipment)==-1)
		m_wndEquipment.SetWindowText(p_Flight->Equipment);

	// Flug
	((CButton*)GetDlgItem(IDC_LEISURETRIP))->SetCheck(p_Flight->Flags & AIRX_LeisureTrip);
	((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->SetCheck(p_Flight->Flags & AIRX_BusinessTrip);
	((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->SetCheck(p_Flight->Flags & AIRX_AwardFlight);

	// Rating
	m_wndRating.SetRating((p_Flight->Flags>>FMAttributes[21].DataParameter) & 0xF);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightOtherPage::OnChooseColor()
{
	theApp.ChooseColor(&p_Flight->Color, this);
}
