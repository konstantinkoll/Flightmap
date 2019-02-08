
// EditFlightDlg.cpp: Implementierung der Klasse EditFlightDlg
//

#include "stdafx.h"
#include "EditFlightDlg.h"
#include "Flightmap.h"


// EditFlightDlg
//

UINT EditFlightDlg::m_LastTab = 0;

EditFlightDlg::EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParentWnd, CItinerary* pItinerary, INT SelectTab)
	: FMTabbedDialog(IDS_EDITFLIGHT, pParentWnd, &m_LastTab)
{
	if (pFlight)
	{
		m_Flight = *pFlight;
	}
	else
	{
		CItinerary::ResetFlight(m_Flight);
	}

	p_Itinerary = pItinerary;

	if (SelectTab!=-1)
		m_LastTab = SelectTab;
}

void EditFlightDlg::DoDataExchange(CDataExchange* pDX)
{
	FMTabbedDialog::DoDataExchange(pDX);

	// Tab 0
	DDX_MaskedText(pDX, IDC_FROM_IATA, m_wndFromIATA, 0, &m_Flight);
	DDX_MaskedText(pDX, IDC_FROM_TIME, m_wndFromTime, 1, &m_Flight);
	DDX_MaskedText(pDX, IDC_FROM_GATE, m_wndFromGate, 2, &m_Flight);
	DDX_MaskedText(pDX, IDC_TO_IATA, m_wndToIATA, 3, &m_Flight);
	DDX_MaskedText(pDX, IDC_TO_TIME, m_wndToTime, 4, &m_Flight);
	DDX_MaskedText(pDX, IDC_TO_GATE, m_wndToGate, 5, &m_Flight);
	DDX_MaskedText(pDX, IDC_FLIGHTTIME, m_wndFlighttime, 23, &m_Flight);
	DDX_MaskedText(pDX, IDC_COMMENT, m_wndComment, 22, &m_Flight);

	// Tab 1
	INT Class;
	if (!pDX->m_bSaveAndValidate && (m_Flight.Class!=AIRX_Unknown))
	{
		Class = (m_Flight.Class==AIRX_Economy) ? 0 : (m_Flight.Class==AIRX_PremiumEconomy) ? 1 : (m_Flight.Class==AIRX_Business) ? 2 : (m_Flight.Class==AIRX_First) ? 3 : (m_Flight.Class==AIRX_Crew) ? 4 : 5;
		DDX_Radio(pDX, IDC_CLASS_Y, Class);
	}

	DDX_Control(pDX, IDC_CARRIER, m_wndCarrier);
	DDX_Control(pDX, IDC_EQUIPMENT, m_wndEquipment);
	DDX_MaskedText(pDX, IDC_FLIGHTNO, m_wndFlightNo, 8, &m_Flight);
	DDX_MaskedText(pDX, IDC_CODESHARES, m_wndCodeshares, 9, &m_Flight);
	DDX_MaskedText(pDX, IDC_REGISTRATION, m_wndRegistration, 11, &m_Flight);
	DDX_MaskedText(pDX, IDC_AIRCRAFTNAME, m_wndAircraftName, 12, &m_Flight);
	DDX_MaskedText(pDX, IDC_ETIXCODE, m_wndEtixCode, 16, &m_Flight);
	DDX_MaskedText(pDX, IDC_FARE, m_wndFare, 17, &m_Flight);
	DDX_MaskedText(pDX, IDC_AWARDMILES, m_wndAwardMiles, 18, &m_Flight);
	DDX_MaskedText(pDX, IDC_STATUSMILES, m_wndStatusMiles, 19, &m_Flight);
	DDX_MaskedText(pDX, IDC_UPGRADEVOUCHER, m_wndUpgradeVoucher, 24, &m_Flight);
	DDX_Control(pDX, IDC_COLORINDICATOR, m_wndColorIndicator);
	DDX_Control(pDX, IDC_RATING, m_wndRating);
	DDX_MaskedText(pDX, IDC_SEAT, m_wndSeat, 14, &m_Flight);

	// Tab 2
	DDX_Control(pDX, IDC_FILEVIEW, m_wndFileView);

	if (pDX->m_bSaveAndValidate)
	{
		// Tab 0
		m_Flight.Flags &= ~AIRX_GroundTransportation;

		if (((CButton*)GetDlgItem(IDC_GROUNDTRANSPORTATION))->GetCheck())
			m_Flight.Flags |= AIRX_GroundTransportation;

		// Tab 1
		CString tmpStr;
		m_wndCarrier.GetWindowText(tmpStr);
		StringToAttribute(tmpStr, m_Flight, 7);

		m_wndEquipment.GetWindowText(tmpStr);
		StringToAttribute(tmpStr, m_Flight, 10);

		DDX_Radio(pDX, IDC_CLASS_Y, Class);
		m_Flight.Class = (Class==0) ? AIRX_Economy : (Class==1) ? AIRX_PremiumEconomy : (Class==2) ? AIRX_Business : (Class==3) ? AIRX_First : (Class==4) ? AIRX_Crew : (Class==5) ? AIRX_Charter : AIRX_Unknown;

		m_Flight.Flags &= ~((0xF<<FMAttributes[21].DataParameter) | AIRX_LeisureTrip | AIRX_BusinessTrip | AIRX_AwardFlight | AIRX_Upgrade | AIRX_Cancelled);
		m_Flight.Flags |= m_wndRating.GetRating()<<FMAttributes[21].DataParameter;

		if (((CButton*)GetDlgItem(IDC_LEISURETRIP))->GetCheck())
			m_Flight.Flags |= AIRX_LeisureTrip;

		if (((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->GetCheck())
			m_Flight.Flags |= AIRX_BusinessTrip;

		if (((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->GetCheck())
			m_Flight.Flags |= AIRX_AwardFlight;

		if (((CButton*)GetDlgItem(IDC_UPGRADE))->GetCheck())
			m_Flight.Flags |= AIRX_Upgrade;

		if (((CButton*)GetDlgItem(IDC_CANCELLED))->GetCheck())
			m_Flight.Flags |= AIRX_Cancelled;
	}
}

void EditFlightDlg::GetIATACode(UINT nID, LPSTR pIATA)
{
	ASSERT(pIATA);

	CString tmpStr;
	GetDlgItem(nID)->GetWindowText(tmpStr);

	WideCharToMultiByte(CP_ACP, 0, tmpStr, -1, pIATA, 4, NULL, NULL);
}

void EditFlightDlg::DisplayAirport(UINT nID, LPCAIRPORT lpcAirport)
{
	ASSERT(lpcAirport);

	CString tmpStr1(lpcAirport->Name);
	CString tmpStr2(FMIATAGetCountry(lpcAirport->CountryID)->Name);

	tmpStr1.Append(_T(", "));
	tmpStr1.Append(tmpStr2);

	GetDlgItem(nID)->SetWindowText(tmpStr1);
}

void EditFlightDlg::DisplayAirport(UINT nID, LPCSTR pIATA)
{
	ASSERT(pIATA);

	LPCAIRPORT lpcAirport;
	if (FMIATAGetAirportByCode(pIATA, lpcAirport))
	{
		DisplayAirport(nID, lpcAirport);
	}
	else
	{
		GetDlgItem(nID)->SetWindowText(_T(""));
	}
}

void EditFlightDlg::DisplayAirport(UINT nDisplayID, UINT nEditID)
{
	CHAR Code[4];
	GetIATACode(nEditID, Code);

	DisplayAirport(nDisplayID, Code);
}

void EditFlightDlg::SelectAirport(UINT nEditID, LPSTR pIATA)
{
	ASSERT(nEditID);
	ASSERT(pIATA);

	CHAR Code[4];
	GetIATACode(nEditID, Code);

	FMSelectLocationIATADlg dlg(this, Code);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(pIATA, 4, dlg.p_Airport->Code);
		GetDlgItem(nEditID)->SetWindowText(CString(dlg.p_Airport->Code));

		OnCheckAirports();
	}
}

void EditFlightDlg::DisplayLocation(const FMGeoCoordinates& Location)
{
	GetDlgItem(IDC_WAYPOINT_DISPLAY)->SetWindowText(FMGeoCoordinatesToString(Location));
}

BOOL EditFlightDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!FMTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_EDITFLIGHT_ROUTE, pszTabArea);
	AddTab(IDD_EDITFLIGHT_OTHER, pszTabArea);
	AddTab(IDD_EDITFLIGHT_ATTACHMENTS, pszTabArea);

	return TRUE;
}

BOOL EditFlightDlg::InitDialog()
{
	// Airports
	DisplayLocation(m_Flight.Waypoint);
	OnCheckAirports();

	// Flight
	((CButton*)GetDlgItem(IDC_GROUNDTRANSPORTATION))->SetCheck(m_Flight.Flags & AIRX_GroundTransportation);

	// Carrier
	p_Itinerary->PrepareCarrierCtrl(m_wndCarrier);

	if (m_wndCarrier.SelectString(-1, m_Flight.Carrier)==-1)
		m_wndCarrier.SetWindowText(m_Flight.Carrier);

	// Equipment
	p_Itinerary->PrepareEquipmentCtrl(m_wndEquipment);

	if (m_wndEquipment.SelectString(-1, m_Flight.Equipment)==-1)
		m_wndEquipment.SetWindowText(m_Flight.Equipment);

	// Flight
	((CButton*)GetDlgItem(IDC_LEISURETRIP))->SetCheck(m_Flight.Flags & AIRX_LeisureTrip);
	((CButton*)GetDlgItem(IDC_BUSINESSTRIP))->SetCheck(m_Flight.Flags & AIRX_BusinessTrip);
	((CButton*)GetDlgItem(IDC_AWARDFLIGHT))->SetCheck(m_Flight.Flags & AIRX_AwardFlight);
	((CButton*)GetDlgItem(IDC_UPGRADE))->SetCheck(m_Flight.Flags & AIRX_Upgrade);
	((CButton*)GetDlgItem(IDC_CANCELLED))->SetCheck(m_Flight.Flags & AIRX_Cancelled);

	// Color
	m_wndColorIndicator.SetColor(m_Flight.Color);

	// Rating
	m_wndRating.SetInitialRating((m_Flight.Flags>>FMAttributes[21].DataParameter) & 0xF);

	// Attachments
	m_wndFileView.SetAttachments(p_Itinerary, &m_Flight);

	return FMTabbedDialog::InitDialog();
}


BEGIN_MESSAGE_MAP(EditFlightDlg, FMTabbedDialog)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 3, OnRequestTooltipData)

	ON_BN_CLICKED(IDC_FROM_SELECT, OnFromSelect)
	ON_BN_CLICKED(IDC_TO_SELECT, OnToSelect)
	ON_BN_CLICKED(IDC_WAYPOINT_BTN, OnWaypoint)
	ON_EN_KILLFOCUS(IDC_FROM_IATA, OnCheckAirports)
	ON_EN_KILLFOCUS(IDC_TO_IATA, OnCheckAirports)

	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChooseColor)
END_MESSAGE_MAP()

void EditFlightDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	const UINT nTab = pTooltipData->Item-IDD_TABBEDDIALOG;
	if (nTab==2)
	{
		INT64 FileSize = 0;
		for (UINT a=0; a<m_Flight.AttachmentCount; a++)
			FileSize += p_Itinerary->m_Attachments[m_Flight.Attachments[a]].FileSize;

		WCHAR tmpBuf[256];
		StrFormatByteSize(FileSize, tmpBuf, 256);

		CString tmpStr;
		tmpStr.Format(m_Flight.AttachmentCount==1 ? IDS_FILESTATUS_SINGULAR : IDS_FILESTATUS_PLURAL, m_Flight.AttachmentCount, tmpBuf);

		wcscpy_s(pTooltipData->Hint, 4096, tmpStr);

		*pResult = TRUE;
	}
	else
	{
		FMTabbedDialog::OnRequestTooltipData(pNMHDR, pResult);
	}
}


void EditFlightDlg::OnFromSelect()
{
	SelectAirport(IDC_FROM_IATA, m_Flight.From.Code);
}

void EditFlightDlg::OnToSelect()
{
	SelectAirport(IDC_TO_IATA, m_Flight.To.Code);
}

void EditFlightDlg::OnWaypoint()
{
	FMSelectLocationGPSDlg dlg(m_Flight.Waypoint, this);
	if (dlg.DoModal()==IDOK)
	{
		m_Flight.Waypoint = dlg.m_Location;
		DisplayLocation(dlg.m_Location);
	}
}

void EditFlightDlg::OnCheckAirports()
{
	BOOL bActive = FALSE;

	// Airports
	DisplayAirport(IDC_FROM_NAME, IDC_FROM_IATA);
	DisplayAirport(IDC_TO_NAME, IDC_TO_IATA);

	// Waypoint
	CHAR Code[4];

	LPCAIRPORT pFrom;
	GetIATACode(IDC_FROM_IATA, Code);
	if (!FMIATAGetAirportByCode(Code, pFrom))
		goto SetActive;

	LPCAIRPORT pTo;
	GetIATACode(IDC_TO_IATA, Code);
	if (!FMIATAGetAirportByCode(Code, pTo))
		goto SetActive;

	bActive = (pFrom==pTo) && (pFrom!=NULL);

SetActive:
	GetDlgItem(IDC_WAYPOINT_BTN)->EnableWindow(bActive);
	GetDlgItem(IDC_WAYPOINT_DISPLAY)->EnableWindow(bActive);
}


void EditFlightDlg::OnChooseColor()
{
	theApp.ChooseColor(&m_Flight.Color, this);

	m_wndColorIndicator.SetColor(m_Flight.Color);
}
