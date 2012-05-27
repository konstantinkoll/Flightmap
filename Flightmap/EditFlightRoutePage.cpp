
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

void EditFlightRoutePage::SelectAirport(UINT nID, CHAR* pIATA)
{
	ASSERT(nID);
	ASSERT(pIATA);

	CString tmpStr;
	GetDlgItem(nID)->GetWindowText(tmpStr);

	CHAR Code[4];
	WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, Code, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(IDD_SELECTIATA, this, Code);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(pIATA, 4, dlg.p_Airport->Code);

		CString tmpStr(dlg.p_Airport->Code);
		GetDlgItem(nID)->SetWindowText(tmpStr);

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

	DisplayLocation(p_Flight->Waypoint);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightRoutePage::OnFromSelect()
{
	SelectAirport(IDC_FROM_IATA, p_Flight->From.Code);
}

void EditFlightRoutePage::OnToSelect()
{
	SelectAirport(IDC_TO_IATA, p_Flight->To.Code);
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
