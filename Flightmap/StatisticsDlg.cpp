
// StatisticsDlg.cpp: Implementierung der Klasse StatisticsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "StatisticsDlg.h"


struct SParameters
{
	CListCtrl* pList;
	INT Column;
	BOOL ConvertToNumber;
};

INT CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SParameters* pSParameters = (SParameters*)lParamSort;

	CString item1= pSParameters->pList->GetItemText((INT)lParam1, pSParameters->Column);
	CString item2= pSParameters->pList->GetItemText((INT)lParam2, pSParameters->Column);

	if (pSParameters->ConvertToNumber)
	{
		LONG i1;
		if (swscanf_s(item1, _T("%i"), &i1)<1)
			return 0;

		LONG i2;
		if (swscanf_s(item2, _T("%i"), &i2)<1)
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

	HDITEM Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.mask = HDI_FORMAT;

	pHeaderCtrl->GetItem(0, &Item);
	Item.fmt |= HDF_SORTDOWN;
	pHeaderCtrl->SetItem(0, &Item);

	wndList.SetRedraw(TRUE);
	wndList.Invalidate();
}


// CClassesList
//

#define CLASSESCOUNT     10

CString CClassesList::m_MaskFlightsSingular;
CString CClassesList::m_MaskFlightsPlural;
CString CClassesList::m_Names[6];
CIcons CClassesList::m_SeatIcons;

CClassesList::CClassesList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING, sizeof(ClassItemData))
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = theApp.LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CClassesList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CClassesList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Strings
	if (m_MaskFlightsSingular.IsEmpty())
	{
		ENSURE(m_MaskFlightsSingular.LoadString(IDS_FLIGHTS_SINGULAR));
		ENSURE(m_MaskFlightsPlural.LoadString(IDS_FLIGHTS_PLURAL));

		for (UINT a=0; a<6; a++)
			ENSURE(m_Names[a].LoadString(IDS_CLASS_F+a));
	}

	// Icons
	m_SeatIcons.Load(IDB_CLASSICONS, 64);

	// Item
	SetItemHeight(m_SeatIcons, 3);

	// Categories
	AddItemCategory(CString((LPCSTR)IDS_REVENUEFLIGHTS));
	AddItemCategory(CString((LPCSTR)IDS_NONREVFLIGHTS));
}


// Layouts

void CClassesList::AdjustLayout()
{
	AdjustLayoutColumns();
}


// Item categories

INT CClassesList::GetItemCategory(INT Index) const
{
	return GetClassItemData(Index)->Category;
}


// Item data

void CClassesList::AddClass(UINT nID, UINT Flights, DOUBLE Distance)
{
	ClassItemData Data;

	Data.DisplayName = m_Names[Data.iIcon=nID%6];
	Data.Flights = Flights;
	Data.Distance = Distance;
	Data.Category = nID<5 ? 0 : 1;

	AddItem(&Data);
}

void CClassesList::SetClasses(UINT* pFlights, DOUBLE* pDistances)
{
	ASSERT(pFlights);
	ASSERT(pDistances);

	// Add Files
	SetItemCount(CLASSESCOUNT, FALSE);

	for (UINT a=0; a<CLASSESCOUNT; a++)
#ifndef _DEBUG
		if (pFlights[a])
#endif
		AddClass(a, pFlights[a], pDistances[a]);

	LastItem();

	AdjustLayout();
}


// Drawing

void CClassesList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	ASSERT(rectItem);

	const ClassItemData* pData = GetClassItemData(Index);

	CString tmpStr;
	tmpStr.Format(pData->Flights==1 ? m_MaskFlightsSingular : m_MaskFlightsPlural, pData->Flights);

	WCHAR Distance[256];
	DistanceToString(Distance, 256, pData->Distance);

	DrawTile(dc, rectItem, m_SeatIcons, pData->iIcon,
		GetDarkTextColor(dc, Index, Themed), 3,
		pData->DisplayName, tmpStr, Distance);
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

CIcons SeatIcons;

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

	CWaitCursor WaitCursor;

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
	WideCharToMultiByte(CP_ACP, 0, tmpStr, -1, FilterAirport, 4, NULL, NULL);

	CString FilterCarrier;
	m_wndFilterCarrier.GetWindowText(FilterCarrier);

	CString FilterEquipment;
	m_wndFilterEquipment.GetWindowText(FilterEquipment);

	BOOL FilterBusiness = ((CButton*)GetDlgItem(IDC_FILTER_BUSINESSTRIP))->GetCheck();
	BOOL FilterLeisure = ((CButton*)GetDlgItem(IDC_FILTER_LEISURETRIP))->GetCheck();

	UCHAR FilterRating = m_wndFilterRating.GetRating();

	// Calculate
	for (UINT a=0; a<p_Itinerary->m_Flights.m_ItemCount; a++)
	{
		const AIRX_Flight* pFlight = &p_Itinerary->m_Flights[a];

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

		if (!FilterCarrier.IsEmpty() && (wcscmp(FilterCarrier, pFlight->Carrier)!=0))
			continue;

		if (!FilterEquipment.IsEmpty() && (wcscmp(FilterEquipment, pFlight->Equipment)!=0))
			continue;

		if (FilterBusiness && ((pFlight->Flags & AIRX_BusinessTrip)==0))
			continue;

		if (FilterLeisure && ((pFlight->Flags & AIRX_LeisureTrip)==0))
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

		if (theApp.m_StatisticsMergeAwards)
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
			Class = theApp.m_StatisticsMergeClasses ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 8 : 2;
			break;

		case AIRX_Economy:
			Class = IsAwardFlight ? 9 : 3;
			break;

		case AIRX_Charter:
			Class = theApp.m_StatisticsMergeClasses ? IsAwardFlight ? 9 : 3 : IsAwardFlight ? 9 : 4;
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

		CHAR From[4];
		strcpy_s(From, 4, pFlight->From.Code);

		CHAR To[4];
		strcpy_s(To, 4, pFlight->To.Code);

		if (theApp.m_StatisticsMergeMetro)
		{
			LPCAIRPORT lpcAirport;

			if (FMIATAGetAirportByCode(From, lpcAirport))
				if ((lpcAirport->MetroCode[0]!='\0') && (strcmp(lpcAirport->Code, lpcAirport->MetroCode)!=0))
					strcpy_s(From, 4, lpcAirport->MetroCode);

			if (FMIATAGetAirportByCode(To, lpcAirport))
				if ((lpcAirport->MetroCode[0]!='\0') && (strcmp(lpcAirport->Code, lpcAirport->MetroCode)!=0))
					strcpy_s(To, 4, lpcAirport->MetroCode);
		}

		if ((strlen(From)==3) && (strlen(To)==3))
		{
			const BOOL Swap = theApp.m_StatisticsMergeDirections && (strcmp(To, From)<0);

			CHAR Key[7];
			strcpy_s(Key, 7, Swap ? To : From);
			strcat_s(Key, 7, Swap ? From : To);

			Route[Key]++;
		}

		if (strlen(From)==3)
			Airport[From]++;

		if (strlen(To)==3)
			Airport[To]++;

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
	tmpStr.Format(FlightCount==1 ? IDS_FLIGHTS_SINGULAR :IDS_FLIGHTS_PLURAL, FlightCount);
	GetDlgItem(IDC_FLIGHTCOUNT)->SetWindowText(tmpStr);

	WCHAR tmpBuf[256];
	DistanceToString(tmpBuf, 256, DistanceNM);
	GetDlgItem(IDC_TOTALDISTANCE)->SetWindowText(tmpBuf);

	swprintf_s(tmpBuf, 256, L"%ud %02u:%02u", FlightTime/1440, (FlightTime/60)%24 ,FlightTime%60);
	GetDlgItem(IDC_TOTALFLIGHTTIME)->SetWindowText(FlightTime ? tmpBuf : L"—");

	RouteToString(tmpBuf, 256, Longest);
	GetDlgItem(IDC_LONGESTFLIGHT)->SetWindowText(tmpBuf);

	RouteToString(tmpBuf, 256, Shortest);
	GetDlgItem(IDC_SHORTESTFLIGHT)->SetWindowText(tmpBuf);

	swprintf_s(tmpBuf, 256, L"%lu", Spent);
	GetDlgItem(IDC_TOTALMONEYSPENT)->SetWindowText(Spent ? tmpBuf : L"—");

	MilesToString(tmpBuf, 256, Miles[0][0], Miles[0][1]);
	GetDlgItem(IDC_TOTALMILESEARNED)->SetWindowText(tmpBuf);

	MilesToString(tmpBuf, 256, Miles[1][0], Miles[1][1]);
	GetDlgItem(IDC_TOTALMILESSPENT)->SetWindowText(tmpBuf);

	m_wndListClass.SetClasses(FlightsByClass, DistanceNMByClass);

	// Routes
	Start(m_wndListRoute);

	LVITEM Item;
	UINT Columns1[1] = { 1 };
	UINT Columns2[2] = { 1, 2 };

	ZeroMemory(&Item, sizeof(Item));
	Item.mask = LVIF_TEXT | LVIF_COLUMNS;
	Item.cColumns = 2;
	Item.puColumns = Columns2;

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
		Item.pszText = tmpStr.GetBuffer();

		const INT Index = m_wndListRoute.InsertItem(&Item);

		tmpStr = Rt;
		m_wndListRoute.SetItemText(Index, 1, tmpStr);

		m_wndListRoute.SetItemData(Index, Index);
		pPair1 = Route.PGetNextAssoc(pPair1);
	}

	// Airports
	Start(m_wndListAirport);

	ZeroMemory(&Item, sizeof(Item));
	Item.mask = LVIF_TEXT | LVIF_COLUMNS;
	Item.cColumns = 2;
	Item.puColumns = Columns2;

	CFlightsAirport::CPair* pPair2 = Airport.PGetFirstAssoc();
	while (pPair2)
	{
		CHAR Code[4];
		strcpy_s(Code, 4, pPair2->key);

		LPCAIRPORT lpcAirport;
		if (FMIATAGetAirportByCode(Code, lpcAirport))
		{
			tmpStr.Format(_T("%5d"), pPair2->value);
			Item.pszText = tmpStr.GetBuffer();

			const INT Index = m_wndListAirport.InsertItem(&Item);

			tmpStr = lpcAirport->Code;
			m_wndListAirport.SetItemText(Index, 1, tmpStr);

			tmpStr = lpcAirport->Name;
			tmpStr += _T(", ");
			tmpStr += FMIATAGetCountry(lpcAirport->CountryID)->Name;
			m_wndListAirport.SetItemText(Index, 2, tmpStr);
			m_wndListAirport.SetItemData(Index, Index);
		}

		pPair2 = Airport.PGetNextAssoc(pPair2);
	}

	// Carrier
	Start(m_wndListCarrier);

	ZeroMemory(&Item, sizeof(Item));
	Item.mask = LVIF_TEXT | LVIF_COLUMNS;
	Item.cColumns = 2;
	Item.puColumns = Columns2;

	CFlightsCarrier::CPair* pPair3 = Carrier.PGetFirstAssoc();
	while (pPair3)
	{
		tmpStr.Format(_T("%5d"), pPair3->value.FlightCount);
		Item.pszText = tmpStr.GetBuffer();

		const INT Index = m_wndListCarrier.InsertItem(&Item);

		DistanceToString(tmpBuf, 256, pPair3->value.DistanceNM);
		m_wndListCarrier.SetItemText(Index, 1, tmpBuf);

		tmpStr = pPair3->key;
		m_wndListCarrier.SetItemText(Index, 2, tmpStr);

		m_wndListCarrier.SetItemData(Index, Index);
		pPair3 = Carrier.PGetNextAssoc(pPair3);
	}

	// Equipment
	Start(m_wndListEquipment);

	ZeroMemory(&Item, sizeof(Item));
	Item.mask = LVIF_TEXT | LVIF_COLUMNS;
	Item.cColumns = 1;
	Item.puColumns = Columns1;

	CFlightsEquipment::CPair* pPair4 = Equipment.PGetFirstAssoc();
	while (pPair4)
	{
		tmpStr.Format(_T("%5d"), pPair4->value);
		Item.pszText = tmpStr.GetBuffer();

		const INT Index = m_wndListEquipment.InsertItem(&Item);
		m_wndListEquipment.SetItemText(Index, 1, pPair4->key);

		m_wndListCarrier.SetItemData(Index, Index);
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
	ON_BN_CLICKED(IDD_SELECTLOCATIONIATA, OnSelectIATA)
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

	// Filter
	p_Itinerary->PrepareCarrierCtrl(m_wndFilterCarrier, FALSE);
	p_Itinerary->PrepareEquipmentCtrl(m_wndFilterEquipment, FALSE);
	m_wndFilterCarrier.InsertString(0, _T(""));
	m_wndFilterEquipment.InsertString(0, _T(""));
	m_wndFilterRating.SetInitialRating(0);

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
	WideCharToMultiByte(CP_ACP, 0, tmpStr, -1, Code, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(this, Code);
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

	SParameters sp;
	sp.pList = (CListCtrl*)CListCtrl::FromHandle(pLV->hdr.hwndFrom)->GetParent();
	sp.Column = pLV->iItem;
	sp.ConvertToNumber = (sp.Column==0) || ((sp.Column==1) && (sp.pList==&m_wndListCarrier));

	sp.pList->SortItemsEx(SortFunc, (DWORD_PTR)&sp);

	CHeaderCtrl* pHeaderCtrl = sp.pList->GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		HDITEM Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = HDI_FORMAT;

		for (INT a=0; a<pHeaderCtrl->GetItemCount(); a++)
		{
			pHeaderCtrl->GetItem(a, &Item);

			Item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			if (a==(INT)sp.Column)
				Item.fmt |= sp.ConvertToNumber ? HDF_SORTDOWN : HDF_SORTUP;

			pHeaderCtrl->SetItem(a, &Item);
		}
	}

	*pResult = 0;
}
