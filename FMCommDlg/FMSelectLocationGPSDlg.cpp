
// FMSelectLocationGPSDlg.cpp: Implementierung der Klasse FMSelectLocationGPSDlg
//

#include "StdAfx.h"
#include "FMSelectLocationGPSDlg.h"
#include "Resource.h"


DOUBLE StringToCoord(CString str)
{
	INT Deg;
	INT Min;
	INT Sec;
	WCHAR Ch;
	DOUBLE Res = 0.0;

	INT Scanned = swscanf_s(str.GetBuffer(), L"%d°%d\'%d\"%c", &Deg, &Min, &Sec, &Ch, 1);

	if (Scanned>=1)
		Res += Deg;
	if (Scanned>=2)
		Res += abs(Min)/60.0;
	if (Scanned>=3)
		Res += abs(Sec)/3600.0;
	if (Scanned>=4)
		if ((Ch==L'N') || (Ch==L'W'))
			Res = -Res;

	if ((Res<-180.0) || (Res>180.0))
		Res = 0.0;

	return Res;
}


// FMSelectLocationGPSDlg
//

FMSelectLocationGPSDlg::FMSelectLocationGPSDlg(CWnd* pParentWnd, const FMGeoCoordinates Location)
	: CDialog(IDD_SELECTGPS, pParentWnd)
{
	m_Location = Location;
}

void FMSelectLocationGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAP_SELECTION, m_Map);

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


BEGIN_MESSAGE_MAP(FMSelectLocationGPSDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(MAP_UPDATE_LOCATION, IDC_MAP_SELECTION, OnUpdateEdit)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_EN_KILLFOCUS(IDC_LATITUDE, OnLatitudeChanged)
	ON_EN_KILLFOCUS(IDC_LONGITUDE, OnLongitudeChanged)
END_MESSAGE_MAP()

BOOL FMSelectLocationGPSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_SELECTGPS));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	m_Map.SetGeoCoordinates(m_Location);
	SetTimer(1, 500, NULL);

	return TRUE;
}

void FMSelectLocationGPSDlg::OnDestroy()
{
	KillTimer(1);
	CDialog::OnDestroy();
}

void FMSelectLocationGPSDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		m_Map.OnBlink();

	CDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void FMSelectLocationGPSDlg::OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	tagGPSDATA* pTag = (tagGPSDATA*)pNMHDR;
	m_Location = *pTag->pCoord;

	CHAR Buffer[16];
	FMGeoCoordinateToString(m_Location.Latitude, Buffer, 16, TRUE, FALSE);
	GetDlgItem(IDC_LATITUDE)->SetWindowText(CString(Buffer));
	FMGeoCoordinateToString(m_Location.Longitude, Buffer, 16, TRUE, FALSE);
	GetDlgItem(IDC_LONGITUDE)->SetWindowText(CString(Buffer));

	*pResult = 0;
}

void FMSelectLocationGPSDlg::OnReset()
{
	m_Location.Latitude = m_Location.Longitude = 0.0;
	EndDialog(IDOK);
}

void FMSelectLocationGPSDlg::OnLatitudeChanged()
{
	CString strLat;
	GetDlgItem(IDC_LATITUDE)->GetWindowText(strLat);
	m_Location.Latitude = StringToCoord(strLat);

	m_Map.SetGeoCoordinates(m_Location);
}

void FMSelectLocationGPSDlg::OnLongitudeChanged()
{
	CString strLon;
	GetDlgItem(IDC_LONGITUDE)->GetWindowText(strLon);
	m_Location.Longitude = StringToCoord(strLon);

	m_Map.SetGeoCoordinates(m_Location);
}
