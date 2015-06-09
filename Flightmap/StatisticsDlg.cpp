
// StatisticsDlg.cpp: Implementierung der Klasse StatisticsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "StatisticsDlg.h"


struct SortParameters
{
	CListCtrl* pList;
	INT Column;
	BOOL ConvertToNumber;
};

INT CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SortParameters* sp = (SortParameters*)lParamSort;

	CString item1= sp->pList->GetItemText((INT)lParam1, sp->Column);
	CString item2= sp->pList->GetItemText((INT)lParam2, sp->Column);

	if (sp->ConvertToNumber)
	{
		LONG i1;
		if (swscanf_s(item1.GetBuffer(), _T("%i"), &i1)<1)
			return 0;

		LONG i2;
		if (swscanf_s(item2.GetBuffer(), _T("%i"), &i2)<1)
			return 0;

		return i2-i1;
	}

	return item1.CompareNoCase(item2);
}

__forceinline void Prepare(CListCtrl& wndList)
{
	wndList.SetExtendedStyle(wndList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
}

void AddColumn(CListCtrl& wndList, INT ID, UINT ResID, BOOL Right=FALSE)
{
	CString tmpStr((LPCSTR)ResID);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = tmpStr.GetBuffer();
	lvc.cx = 65;
	lvc.fmt = Right ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.iSubItem = ID;

	wndList.InsertColumn(ID, &lvc);
}

__forceinline void Start(CListCtrl& wndList)
{
	wndList.SetRedraw(FALSE);
	wndList.DeleteAllItems();
}

__forceinline void Finish(CListCtrl& wndList, INT Count)
{
	for (INT a=0; a<Count; a++)
		wndList.SetColumnWidth(a, LVSCW_AUTOSIZE_USEHEADER);

	CHeaderCtrl* pHeaderCtrl = wndList.GetHeaderCtrl();

	HDITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = HDI_FORMAT;

	pHeaderCtrl->GetItem(0, &item);
	item.fmt |= HDF_SORTDOWN;
	pHeaderCtrl->SetItem(0, &item);

	wndList.SetRedraw(TRUE);
	wndList.Invalidate();
}


// StatisticsDlg
//

struct Airline
{
	UINT FlightCount;
	DOUBLE DistanceNM;
};

typedef CMap<CStringA, LPCSTR, UINT, UINT> CFlightsRoute;
typedef CMap<CStringA, LPCSTR, UINT, UINT> CFlightsAirport;
typedef CMap<CStringW, LPCWSTR, Airline, Airline> CFlightsCarrier;
typedef CMap<CStringW, LPCWSTR, UINT, UINT> CFlightsEquipment;

StatisticsDlg::StatisticsDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_STATISTICS, pParentWnd)
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
	DDX_Control(pDX, IDC_LIST_ROUTE, m_wndListRoute);
	DDX_Control(pDX, IDC_LIST_AIRPORT, m_wndListAirport);
	DDX_Control(pDX, IDC_LIST_CARRIER, m_wndListCarrier);
	DDX_Control(pDX, IDC_LIST_EQUIPMENT, m_wndListEquipment);
}

void StatisticsDlg::UpdateStatistics()
{
	ASSERT(p_Itinerary);

	CWaitCursor csr;

	// Reset
	UINT FlightCount = 0;
	DOUBLE DistanceNM = 0.0;
	LONG FlightTime = 0;
	AIRX_Route Longest = { "", "", -1.0 };
	AIRX_Route Shortest = { "", "", -1.0 };
	ULONG Spent = 0;
	ULONG Miles[2][2] = {{ 0, 0 }, { 0, 0 }};
	UINT FlightsByClass[10] = { 0 };
	DOUBLE DistanceNMByClass[10] = { 0.0 };
	CFlightsRoute Route;
	CFlightsAirport Airport;
	CFlightsCarrier Carrier;
	CFlightsEquipment Equipment;

	// Filter
	CString tmpStr;
	m_wndFilterAirport.GetWindowText(tmpStr);

	CHAR FilterAirport[4];
	WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, FilterAirport, 4, NULL, NULL);

	CString FilterCarrier;
	m_wndFilterCarrier.GetWindowText(FilterCarrier);

	CString FilterEquipment;
	m_wndFilterEquipment.GetWindowText(FilterEquipment);

	BOOL FilterBusiness = ((CButton*)GetDlgItem(IDC_FILTER_BUSINESSTRIP))->GetCheck();
	BOOL FilterLeisure = ((CButton*)GetDlgItem(IDC_FILTER_LEISURETRIP))->GetCheck();

	UCHAR FilterRating = m_wndFilterRating.GetRating();

	// Calculate
	UINT Count = p_Itinerary->m_Flights.m_ItemCount;
	if (!FMIsLicensed())
		Count = min(Count, 10);

	for (UINT a=0; a<Count; a++)
	{
		const AIRX_Flight* pFlight = &p_Itinerary->m_Flights.m_Items[a];

		// Cancelled
		if (pFlight->Flags & AIRX_Cancelled)
			continue;

		// Empty lines
		if ((strlen(pFlight->From.Code)!=3) || (strlen(pFlight->To.Code)!=3))
			continue;

		// Filter
		if (strlen(FilterAirport)==3)
			if ((strcmp(FilterAirport, pFlight->From.Code)!=0) && (strcmp(FilterAirport, pFlight->To.Code)!=0))
				continue;
		if (!FilterCarrier.IsEmpty())
			if (wcscmp(FilterCarrier, pFlight->Carrier)!=0)
				continue;
		if (!FilterEquipment.IsEmpty())
			if (wcscmp(FilterEquipment, pFlight->Equipment)!=0)
				continue;
		if (FilterBusiness)
			if ((pFlight->Flags & AIRX_BusinessTrip)==0)
				continue;
		if (FilterLeisure)
			if ((pFlight->Flags & AIRX_LeisureTrip)==0)
				continue;
		if (pFlight->Flags>>28<FilterRating)
			continue;

		// Count
		FlightCount++;
		FlightTime += pFlight->FlightTime;
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

		if (theApp.m_MergeAwards)
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
			Class = theApp.m_MergeClasses ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 8 : 2;
			break;
		case AIRX_Economy:
			Class = IsAwardFlight ? 9 : 3;
			break;
		case AIRX_Charter:
			Class = theApp.m_MergeClasses ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 9 : 4;
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

		Spent += pFlight->Fare;

		if ((strlen(pFlight->From.Code)==3) && (strlen(pFlight->To.Code)==3))
		{
			BOOL Swap = (strcmp(pFlight->To.Code, pFlight->From.Code)<0) && theApp.m_MergeDirections;
			CHAR Key[7];
			strcpy_s(Key, 7, Swap ? pFlight->To.Code : pFlight->From.Code);
			strcat_s(Key, 7, Swap ? pFlight->From.Code : pFlight->To.Code);

			Route[Key]++;
		}

		if (strlen(pFlight->From.Code)==3)
			Airport[pFlight->From.Code]++;
		if (strlen(pFlight->To.Code)==3)
			Airport[pFlight->To.Code]++;

		if (pFlight->Carrier[0]!=L'\0')
		{
			if (pFlight->Flags & AIRX_DistanceValid)
				Carrier[pFlight->Carrier].DistanceNM += pFlight->DistanceNM;

			Carrier[pFlight->Carrier].FlightCount++;
		}

		if (pFlight->Equipment[0]!=L'\0')
			Equipment[pFlight->Equipment]++;
	}

	// Display
	CString MaskFlightsSingular((LPCSTR)IDS_FLIGHTS_SINGULAR);
	CString MaskFlightsPlural((LPCSTR)IDS_FLIGHTS_PLURAL);

	tmpStr.Format(FlightCount==1 ? MaskFlightsSingular : MaskFlightsPlural, FlightCount);
	GetDlgItem(IDC_FLIGHTCOUNT)->SetWindowText(tmpStr);

	WCHAR tmpBuf[256];
	DistanceToString(tmpBuf, 256, DistanceNM);
	GetDlgItem(IDC_TOTALDISTANCE)->SetWindowText(tmpBuf);

	swprintf(tmpBuf, 256, L"%ud %02u:%02u", FlightTime/1440, (FlightTime/60)%24 ,FlightTime%60);
	GetDlgItem(IDC_TOTALFLIGHTTIME)->SetWindowText(FlightTime ? tmpBuf : L"—");

	RouteToString(tmpBuf, 256, Longest);
	GetDlgItem(IDC_LONGESTFLIGHT)->SetWindowText(tmpBuf);

	RouteToString(tmpBuf, 256, Shortest);
	GetDlgItem(IDC_SHORTESTFLIGHT)->SetWindowText(tmpBuf);

	swprintf(tmpBuf, 256, L"%lu", Spent);
	GetDlgItem(IDC_TOTALMONEYSPENT)->SetWindowText(Spent ? tmpBuf : L"—");

	MilesToString(tmpStr, Miles[0][0], Miles[0][1]);
	GetDlgItem(IDC_TOTALMILESEARNED)->SetWindowText(tmpStr);

	MilesToString(tmpStr, Miles[1][0], Miles[1][1]);
	GetDlgItem(IDC_TOTALMILESSPENT)->SetWindowText(tmpStr);

	m_wndListClass.SetRedraw(FALSE);
	m_wndListClass.DeleteAllItems();

	UINT Columns1[1] = { 1 };
	UINT Columns2[2] = { 1, 2 };
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_GROUPID | LVIF_IMAGE | LVIF_COLUMNS;
	item.cColumns = 2;
	item.puColumns = Columns2;

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

	m_wndListClass.SetRedraw(TRUE);
	m_wndListClass.Invalidate();

	// Routes
	Start(m_wndListRoute);

	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	item.cColumns = 2;
	item.puColumns = Columns2;

	CFlightsRoute::CPair* pPair1 = Route.PGetFirstAssoc();
	while (pPair1)
	{
		CHAR Key[7];
		strcpy_s(Key, 7, pPair1->key);

		CHAR Rt[8];
		strncpy_s(Rt, 8, Key, 3);
		strcat_s(Rt, 8, "–");
		strncat_s(Rt, 8, &Key[3], 3);

		tmpStr.Format(_T("%5d"), pPair1->value);
		item.pszText = tmpStr.GetBuffer();

		INT idx = m_wndListRoute.InsertItem(&item);

		tmpStr = Rt;
		m_wndListRoute.SetItemText(idx, 1, tmpStr);

		m_wndListRoute.SetItemData(idx, idx);
		pPair1 = Route.PGetNextAssoc(pPair1);
	}

	// Airports
	Start(m_wndListAirport);

	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	item.cColumns = 2;
	item.puColumns = Columns2;

	CFlightsAirport::CPair* pPair2 = Airport.PGetFirstAssoc();
	while (pPair2)
	{
		CHAR Code[4];
		strcpy_s(Code, 4, pPair2->key);

		FMAirport* pAirport;
		if (FMIATAGetAirportByCode(Code, &pAirport))
		{
			tmpStr.Format(_T("%5d"), pPair2->value);
			item.pszText = tmpStr.GetBuffer();

			INT idx = m_wndListAirport.InsertItem(&item);

			tmpStr = pAirport->Code;
			m_wndListAirport.SetItemText(idx, 1, tmpStr);

			tmpStr = pAirport->Name;
			tmpStr += _T(", ");
			tmpStr += FMIATAGetCountry(pAirport->CountryID)->Name;
			m_wndListAirport.SetItemText(idx, 2, tmpStr);
			m_wndListAirport.SetItemData(idx, idx);
		}

		pPair2 = Airport.PGetNextAssoc(pPair2);
	}

	// Carrier
	Start(m_wndListCarrier);

	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	item.cColumns = 2;
	item.puColumns = Columns2;

	CFlightsCarrier::CPair* pPair3 = Carrier.PGetFirstAssoc();
	while (pPair3)
	{
		tmpStr.Format(_T("%5d"), pPair3->value.FlightCount);
		item.pszText = tmpStr.GetBuffer();

		INT idx = m_wndListCarrier.InsertItem(&item);

		DistanceToString(tmpBuf, 256, pPair3->value.DistanceNM);
		m_wndListCarrier.SetItemText(idx, 1, tmpBuf);

		tmpStr = pPair3->key;
		m_wndListCarrier.SetItemText(idx, 2, tmpStr);

		m_wndListCarrier.SetItemData(idx, idx);
		pPair3 = Carrier.PGetNextAssoc(pPair3);
	}

	// Equipment
	Start(m_wndListEquipment);

	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	item.cColumns = 1;
	item.puColumns = Columns1;

	CFlightsEquipment::CPair* pPair4 = Equipment.PGetFirstAssoc();
	while (pPair4)
	{
		tmpStr.Format(_T("%5d"), pPair4->value);
		item.pszText = tmpStr.GetBuffer();

		INT idx = m_wndListEquipment.InsertItem(&item);
		m_wndListEquipment.SetItemText(idx, 1, pPair4->key);

		m_wndListCarrier.SetItemData(idx, idx);
		pPair4 = Equipment.PGetNextAssoc(pPair4);
	}

	// Finish
	Finish(m_wndListRoute, 2);
	Finish(m_wndListAirport, 3);
	Finish(m_wndListCarrier, 3);
	Finish(m_wndListEquipment, 2);
}


BEGIN_MESSAGE_MAP(StatisticsDlg, CDialog)
	ON_MESSAGE_VOID(WM_UPDATESTATISTICS, OnUpdateStatistics)
	ON_EN_KILLFOCUS(IDC_FILTER_AIRPORT, OnPostUpdateStatistics)
	ON_BN_CLICKED(IDD_SELECTIATA, OnSelectIATA)
	ON_CBN_SELENDOK(IDC_FILTER_CARRIER, OnPostUpdateStatistics)
	ON_CBN_SELENDOK(IDC_FILTER_EQUIPMENT, OnPostUpdateStatistics)
	ON_BN_CLICKED(IDC_FILTER_BUSINESSTRIP, OnPostUpdateStatistics)
	ON_BN_CLICKED(IDC_FILTER_LEISURETRIP, OnPostUpdateStatistics)
	ON_MESSAGE_VOID(WM_RATINGCHANGED, OnPostUpdateStatistics)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortLists)
END_MESSAGE_MAP()

BOOL StatisticsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadDialogIcon(IDD_STATISTICS);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Filter
	PrepareCarrierCtrl(&m_wndFilterCarrier, p_Itinerary, FALSE);
	PrepareEquipmentCtrl(&m_wndFilterEquipment, p_Itinerary, FALSE);
	m_wndFilterCarrier.InsertString(0, _T(""));
	m_wndFilterEquipment.InsertString(0, _T(""));
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

	m_SeatIcons.Create(IDB_SEATICONS, 32, 32);
	m_wndListClass.SetImageList(&m_SeatIcons, LVSIL_NORMAL);

	m_wndListClass.SetView(LV_VIEW_TILE);
	m_wndListClass.EnableGroupView(TRUE);

	// Route
	Prepare(m_wndListRoute);
	AddColumn(m_wndListRoute, 0, IDS_FLIGHTS, TRUE);
	AddColumn(m_wndListRoute, 1, IDS_ROUTE);

	// Airports
	Prepare(m_wndListAirport);
	AddColumn(m_wndListAirport, 0, IDS_FLIGHTS, TRUE);
	AddColumn(m_wndListAirport, 1, IDS_AIRPORT_CODE);
	AddColumn(m_wndListAirport, 2, IDS_AIRPORT_LOCATION);

	// Carrier
	Prepare(m_wndListCarrier);
	AddColumn(m_wndListCarrier, 0, IDS_FLIGHTS, TRUE);
	AddColumn(m_wndListCarrier, 1, IDS_COLUMN6, TRUE);
	AddColumn(m_wndListCarrier, 2, IDS_COLUMN7);

	// Equipment
	Prepare(m_wndListEquipment);
	AddColumn(m_wndListEquipment, 0, IDS_FLIGHTS, TRUE);
	AddColumn(m_wndListEquipment, 1, IDS_COLUMN10);

	UpdateStatistics();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void StatisticsDlg::OnPostUpdateStatistics()
{
	PostMessage(WM_UPDATESTATISTICS);
}

void StatisticsDlg::OnUpdateStatistics()
{
	UpdateStatistics();
}

void StatisticsDlg::OnSelectIATA()
{
	CString tmpStr;
	m_wndFilterAirport.GetWindowText(tmpStr);

	CHAR Code[4];
	WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, Code, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(IDD_SELECTIATA, this, Code);
	if (dlg.DoModal()==IDOK)
	{
		tmpStr = dlg.p_Airport->Code;
		m_wndFilterAirport.SetWindowText(tmpStr);

		OnPostUpdateStatistics();
	}
}

void StatisticsDlg::OnSortLists(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;

	SortParameters sp;
	sp.pList = (CListCtrl*)CListCtrl::FromHandle(pLV->hdr.hwndFrom)->GetParent();
	sp.Column = pLV->iItem;
	sp.ConvertToNumber = (sp.Column==0) || ((sp.Column==1) && (sp.pList==&m_wndListCarrier));

	sp.pList->SortItemsEx(SortFunc, (DWORD_PTR)&sp);

	CHeaderCtrl* pHeaderCtrl = sp.pList->GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		HDITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = HDI_FORMAT;

		for (INT a=0; a<pHeaderCtrl->GetItemCount(); a++)
		{
			pHeaderCtrl->GetItem(a, &item);

			item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			if (a==(INT)sp.Column)
				item.fmt |= sp.ConvertToNumber ? HDF_SORTDOWN : HDF_SORTUP;

			pHeaderCtrl->SetItem(a, &item);
		}
	}

	*pResult = 0;
}
