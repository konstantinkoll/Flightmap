
// EditFlightRoutePage.cpp: Implementierung der Klasse EditFlightRoutePage
//

#include "stdafx.h"
#include "EditFlightRoutePage.h"



// EditFlightRoutePage
//

EditFlightRoutePage::EditFlightRoutePage(AIRX_Flight* pFlight)
	: CPropertyPage(IDD_ROUTE)
{
	ASSERT(pFlight);

	p_Flight = pFlight;
}

void EditFlightRoutePage::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		CalcDistance(*p_Flight, TRUE);
	}
}

void EditFlightRoutePage::DisplayAirport(UINT nID, FMAirport* pAirport)
{
	CString tmpStr1(pAirport->Name);
	CString tmpStr2(FMIATAGetCountry(pAirport->CountryID)->Name);

	tmpStr1.Append(_T(" ("));
	tmpStr1.Append(tmpStr2);
	tmpStr1.Append(_T(")"));

	GetDlgItem(nID)->SetWindowText(tmpStr1);
}

void EditFlightRoutePage::DisplayAirport(UINT nID, CHAR* pIATA)
{
	ASSERT(pIATA);

	FMAirport* pAirport = NULL;
	if (FMIATAGetAirportByCode(pIATA, &pAirport))
	{
		DisplayAirport(nID, pAirport);
	}
	else
	{
		GetDlgItem(nID)->SetWindowText(_T(""));
	}
}

void EditFlightRoutePage::SelectAirport(UINT nEditID, CHAR* pIATA, UINT nDisplayID)
{
	ASSERT(nEditID);
	ASSERT(pIATA);
	ASSERT(nDisplayID);

	CString tmpStr;
	GetDlgItem(nEditID)->GetWindowText(tmpStr);

	CHAR Code[4];
	WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, Code, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(IDD_SELECTIATA, this, Code);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(pIATA, 4, dlg.p_Airport->Code);

		CString tmpStr(dlg.p_Airport->Code);
		GetDlgItem(nEditID)->SetWindowText(tmpStr);

		DisplayAirport(nDisplayID, dlg.p_Airport);
		SetModified(TRUE);
	}
}

void EditFlightRoutePage::DisplayLocation(const FMGeoCoordinates Location)
{
	CString tmpStr;
	FMGeoCoordinatesToString(Location, tmpStr);

	GetDlgItem(IDC_WAYPOINT_DISPLAY)->SetWindowText(tmpStr);
}


BEGIN_MESSAGE_MAP(EditFlightRoutePage, CPropertyPage)
	ON_BN_CLICKED(IDC_FROM_SELECT, OnFromSelect)
	ON_BN_CLICKED(IDC_TO_SELECT, OnToSelect)
	ON_BN_CLICKED(IDC_WAYPOINT_BTN, OnWaypoint)
END_MESSAGE_MAP()

BOOL EditFlightRoutePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	DisplayAirport(IDC_FROM_NAME, p_Flight->From.Code);
	DisplayAirport(IDC_TO_NAME, p_Flight->To.Code);
	DisplayLocation(p_Flight->Waypoint);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightRoutePage::OnFromSelect()
{
	SelectAirport(IDC_FROM_IATA, p_Flight->From.Code, IDC_FROM_NAME);
}

void EditFlightRoutePage::OnToSelect()
{
	SelectAirport(IDC_TO_IATA, p_Flight->To.Code, IDC_TO_NAME);
}

void EditFlightRoutePage::OnWaypoint()
{
	FMSelectLocationGPSDlg dlg(this, p_Flight->Waypoint);
	if (dlg.DoModal()==IDOK)
	{
		p_Flight->Waypoint = dlg.m_Location;
		DisplayLocation(dlg.m_Location);

		SetModified(TRUE);
	}
}
