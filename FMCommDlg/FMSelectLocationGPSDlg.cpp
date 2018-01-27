
// FMSelectLocationGPSDlg.cpp: Implementierung der Klasse FMSelectLocationGPSDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMSelectLocationGPSDlg
//

FMSelectLocationGPSDlg::FMSelectLocationGPSDlg(const FMGeoCoordinates& Location, CWnd* pParentWnd)
	: FMDialog(IDD_SELECTLOCATIONGPS, pParentWnd)
{
	m_Location = Location;
}

void FMSelectLocationGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MAP, m_wndMap);

	if (pDX->m_bSaveAndValidate)
	{
		CString strLat;
		GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);
		m_Location.Latitude = StringToCoord(strLat);

		CString strLon;
		GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);
		m_Location.Longitude = StringToCoord(strLon);
	}
}

BOOL FMSelectLocationGPSDlg::InitDialog()
{
	m_wndMap.SetLocation(m_Location);

	return TRUE;
}

DOUBLE FMSelectLocationGPSDlg::StringToCoord(LPCWSTR Str)
{
	INT Deg;
	INT Min;
	INT Sec;
	WCHAR Ch;
	DOUBLE Result = 0.0;

	INT Scanned = swscanf_s(Str, L"%i°%i\'%i\"%c", &Deg, &Min, &Sec, &Ch, 1);

	if (Scanned>=1)
		Result += Deg;

	if (Scanned>=2)
		Result += abs(Min)/60.0;

	if (Scanned>=3)
		Result += abs(Sec)/3600.0;

	if (Scanned>=4)
		if ((Ch==L'N') || (Ch==L'W'))
			Result = -Result;

	if ((Result<-180.0) || (Result>180.0))
		Result = 0.0;

	return Result;
}


BEGIN_MESSAGE_MAP(FMSelectLocationGPSDlg, FMDialog)
	ON_NOTIFY(MAP_UPDATE_LOCATION, IDC_MAP, OnUpdateMap)
	ON_EN_KILLFOCUS(IDC_LATITUDE, OnLatitudeChanged)
	ON_EN_KILLFOCUS(IDC_LONGITUDE, OnLongitudeChanged)

	ON_COMMAND(IDM_MAPCTRL_IATA, OnIATA)
	ON_COMMAND(IDM_MAPCTRL_RESET, OnReset)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPCTRL_IATA, IDM_MAPCTRL_RESET, OnUpdateCommands)
END_MESSAGE_MAP()

void FMSelectLocationGPSDlg::OnUpdateMap(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GPSDATA* pTag = (NM_GPSDATA*)pNMHDR;

	m_Location = *pTag->pLocation;

	GetDlgItem(IDC_LATITUDE)->SetWindowText(FMGeoCoordinateToString(m_Location.Latitude, TRUE, FALSE));
	GetDlgItem(IDC_LONGITUDE)->SetWindowText(FMGeoCoordinateToString(m_Location.Longitude, FALSE, FALSE));

	*pResult = 0;
}

void FMSelectLocationGPSDlg::OnLatitudeChanged()
{
	CString strLat;
	GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);

	m_Location.Latitude = StringToCoord(strLat);

	m_wndMap.SetLocation(m_Location);
}

void FMSelectLocationGPSDlg::OnLongitudeChanged()
{
	CString strLon;
	GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);

	m_Location.Longitude = StringToCoord(strLon);

	m_wndMap.SetLocation(m_Location);
}


void FMSelectLocationGPSDlg::OnIATA()
{
	FMSelectLocationIATADlg dlg(this);
	if (dlg.DoModal()==IDOK)
	{
		m_Location = dlg.p_Airport->Location;

		m_wndMap.SetLocation(m_Location);
	}
}

void FMSelectLocationGPSDlg::OnReset()
{
	m_Location.Latitude = m_Location.Longitude = 0.0;

	m_wndMap.SetLocation(m_Location);
}

void FMSelectLocationGPSDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	if (pCmdUI->m_nID==IDM_MAPCTRL_RESET)
		bEnable &= (m_Location.Latitude!=0) || (m_Location.Longitude!=0);

	pCmdUI->Enable(bEnable);
}
