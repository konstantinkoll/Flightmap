
// StatisticsDlg.cpp: Implementierung der Klasse StatisticsDlg
//

#include "stdafx.h"
#include "StatisticsDlg.h"


// StatisticsDlg
//

StatisticsDlg::StatisticsDlg(CItinerary* pItinerary, CWnd* pParent)
	: CDialog(IDD_STATISTICS, pParent)
{
	p_Itinerary = pItinerary;
}

void StatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_MaskedText(pDX, IDC_FILTER_AIRPORT, m_wndFilterAirport, 0);
	DDX_Control(pDX, IDC_FILTER_CARRIER, m_wndFilterCarrier);
	DDX_Control(pDX, IDC_FILTER_EQUIPMENT, m_wndFilterEquipment);
	DDX_Control(pDX, IDC_FILTER_RATING, m_wndFilterRating);
	DDX_Control(pDX, IDC_LIST_CLASS, m_wndListClass);
}

void StatisticsDlg::UpdateStatistics()
{
	ASSERT(p_Itinerary);

	CWaitCursor csr;

	// Reset
	UINT FlightCount = 0;
	DOUBLE DistanceNM = 0.0;
	AIRX_Route Longest = { "", "", -1.0 };
	AIRX_Route Shortest = { "", "", -1.0 };
	ULONG Miles[2][2] = {{ 0, 0 }, { 0, 0 }};
	UINT FlightsByClass[10] = { 0 };
	DOUBLE DistanceNMByClass[10] = { 0.0 };

	// Calculate
	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
	{
		const AIRX_Flight* pFlight = &p_Itinerary->m_Flights.m_Items[a];

		// Filter
		if (FALSE)
			continue;

		// Count
		FlightCount++;
		if (pFlight->Flags & AIRX_DistanceValid)
		{
			DistanceNM += pFlight->DistanceNM;

			if ((Longest.DistanceNM==-1) || (Longest.DistanceNM<pFlight->DistanceNM))
			{
				strcpy_s(Longest.From, 4, pFlight->From.Code);
				strcpy_s(Longest.To, 4, pFlight->To.Code);
				Longest.DistanceNM = pFlight->DistanceNM;
			}

			if ((Shortest.DistanceNM==-1) || (Shortest.DistanceNM>pFlight->DistanceNM))
			{
				strcpy_s(Shortest.From, 4, pFlight->From.Code);
				strcpy_s(Shortest.To, 4, pFlight->To.Code);
				Shortest.DistanceNM = pFlight->DistanceNM;
			}
		}

		BOOL IsAwardFlight = pFlight->Flags & AIRX_AwardFlight;
		Miles[IsAwardFlight ? 1 : 0][0] += pFlight->MilesAward;
		Miles[IsAwardFlight ? 1 : 0][1] += pFlight->MilesStatus;

		if (FALSE)
			IsAwardFlight = FALSE;

		INT Class;
		switch (pFlight->Class)
		{
		case AIRX_First:
			Class = IsAwardFlight ? 6 : 0;
			break;
		case AIRX_Business:
			Class = IsAwardFlight ? 7 : 1;
			break;
		case AIRX_PremiumEconomy:
			Class = FALSE ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 8 : 2;
			break;
		case AIRX_Economy:
			Class = IsAwardFlight ? 9 : 3;
			break;
		case AIRX_Charter:
			Class = FALSE ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 9 : 4;
			break;
		case AIRX_Crew:
			Class = 5;
			break;
		default:
			Class = -1;
		}

		if (Class>=0)
		{
			FlightsByClass[Class]++;
			if (pFlight->Flags & AIRX_DistanceValid)
				DistanceNMByClass[Class] += pFlight->DistanceNM;
		}
	}

	// Display
	CString MaskFlightsSingular;
	CString MaskFlightsPlural;
	ENSURE(MaskFlightsSingular.LoadString(IDS_FLIGHTS_SINGULAR));
	ENSURE(MaskFlightsPlural.LoadString(IDS_FLIGHTS_PLURAL));

	CString tmpStr;
	tmpStr.Format(FlightCount==1 ? MaskFlightsSingular : MaskFlightsPlural, FlightCount);
	GetDlgItem(IDC_FLIGHTCOUNT)->SetWindowText(tmpStr);

	WCHAR tmpBuf[256];
	DistanceToString(tmpBuf, 256, DistanceNM);
	GetDlgItem(IDC_TOTALDISTANCE)->SetWindowText(tmpBuf);

	RouteToString(tmpBuf, 256, Longest);
	GetDlgItem(IDC_LONGESTFLIGHT)->SetWindowText(tmpBuf);

	RouteToString(tmpBuf, 256, Shortest);
	GetDlgItem(IDC_SHORTESTFLIGHT)->SetWindowText(tmpBuf);

	MilesToString(tmpStr, Miles[0][0], Miles[0][1]);
	GetDlgItem(IDC_TOTALMILESEARNED)->SetWindowText(tmpStr);

	MilesToString(tmpStr, Miles[1][0], Miles[1][1]);
	GetDlgItem(IDC_TOTALMILESSPENT)->SetWindowText(tmpStr);

	m_wndListClass.DeleteAllItems();

	UINT Columns[2] = { 1, 2 };
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_GROUPID | LVIF_IMAGE | LVIF_COLUMNS;
	item.cColumns = 2;
	item.puColumns = Columns;

	for (UINT a=0; a<10; a++)
#ifndef _DEBUG
		if (FlightsByClass[a])
#endif
		{
			ENSURE(tmpStr.LoadString(IDS_CLASS_F+a%6));

			item.pszText = tmpStr.GetBuffer();
			item.iGroupId = a<5 ? 0 : 1;
			item.iImage = a%6;

			INT idx = m_wndListClass.InsertItem(&item);

			tmpStr.Format(FlightsByClass[a]==1 ? MaskFlightsSingular : MaskFlightsPlural, FlightsByClass[a]);
			m_wndListClass.SetItemText(idx, 1, tmpStr);

			DistanceToString(tmpBuf, 256, DistanceNMByClass[a]);
			m_wndListClass.SetItemText(idx, 2, tmpBuf);
		}
}


BEGIN_MESSAGE_MAP(StatisticsDlg, CDialog)
END_MESSAGE_MAP()

BOOL StatisticsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_STATISTICS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Filter
	PrepareCarrierCtrl(&m_wndFilterCarrier, p_Itinerary, FALSE);
	PrepareEquipmentCtrl(&m_wndFilterEquipment, p_Itinerary, FALSE);
	m_wndFilterRating.SetRating(0);

	// Class
	CString tmpName;
	CString tmpHint;

	ENSURE(tmpName.LoadString(IDS_REVENUEFLIGHTS));
	m_wndListClass.AddCategory(0, tmpName, _T(""), TRUE);

	ENSURE(tmpName.LoadString(IDS_NONREVFLIGHTS));
	ENSURE(tmpHint.LoadString(IDS_INCLUDESCREW));
	m_wndListClass.AddCategory(1, tmpName, tmpHint, TRUE);

	m_wndListClass.AddColumn(0, _T(""));
	m_wndListClass.AddColumn(1, _T(""));
	m_wndListClass.AddColumn(2, _T(""));

	m_wndListClass.SetFont(&((FMApplication*)AfxGetApp())->m_DefaultFont);
	m_wndListClass.SetView(LV_VIEW_TILE);
	m_wndListClass.EnableGroupView(TRUE);

	UpdateStatistics();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
