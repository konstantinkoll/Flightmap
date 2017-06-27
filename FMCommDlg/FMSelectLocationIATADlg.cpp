
// FMSelectLocationIATADlg.cpp: Implementierung der Klasse FMSelectLocationIATA
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMSelectLocationIATADlg
//

FMSelectLocationIATADlg::FMSelectLocationIATADlg(CWnd* pParentWnd, LPCSTR pAirport)
	: FMDialog(IDD_SELECTIATA, pParentWnd)
{
	m_LastCountrySelected = FMGetApp()->GetInt(_T("IATALastCountrySelected"), 0);
	m_LastSortColumn = FMGetApp()->GetInt(_T("IATALastSortColumn"), 0);
	m_LastSortDirection = FMGetApp()->GetInt(_T("IATALastSortDirection"), FALSE);

	p_Airport = NULL;
	FMIATAGetAirportByCode(pAirport, &p_Airport);
}

void FMSelectLocationIATADlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_AIRPORTS, m_wndAirportList);

	if (pDX->m_bSaveAndValidate)
	{
		const INT Index = m_wndAirportList.GetNextItem(-1, LVIS_SELECTED);
		if (Index!=-1)
			p_Airport = p_Airports[Index];

		FMGetApp()->WriteInt(_T("IATALastCountrySelected"), m_LastCountrySelected);
		FMGetApp()->WriteInt(_T("IATALastSortColumn"), m_LastSortColumn);
		FMGetApp()->WriteInt(_T("IATALastSortDirection"), m_LastSortDirection);
	}
}

INT FMSelectLocationIATADlg::Compare(INT n1, INT n2)
{
	INT Result = 0;

	switch (m_LastSortColumn)
	{
	case 0:
		Result = strcmp(p_Airports[n1]->Code, p_Airports[n2]->Code);
		break;

	case 1:
		Result = strcmp(p_Airports[n1]->Name, p_Airports[n2]->Name);
		break;
	}

	if (m_LastSortDirection)
		Result = -Result;

	return Result;
}

void FMSelectLocationIATADlg::Heap(INT Element, INT Count)
{
	while (Element<=Count/2-1)
	{
		INT Index = (Element+1)*2-1;
		if (Index+1<Count)
			if (Compare(Index, Index+1)<0)
				Index++;

		if (Compare(Element, Index)<0)
		{
			Swap(p_Airports[Element], p_Airports[Index]);
			Element = Index;
		}
		else
		{
			break;
		}
	}
}

void FMSelectLocationIATADlg::Sort()
{
	if (m_AirportCount>1)
	{
		for (INT a=m_AirportCount/2-1; a>=0; a--)
			Heap(a, m_AirportCount);

		for (INT a=m_AirportCount-1; a>0; a--)
		{
			Swap(p_Airports[0], p_Airports[a]);
			Heap(0, a);
		}
	}

	CHeaderCtrl* pHeaderCtrl = m_wndAirportList.GetHeaderCtrl();

	HDITEM Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.mask = HDI_FORMAT;

	for (INT a=0; a<2; a++)
	{
		pHeaderCtrl->GetItem(a, &Item);

		Item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		if (a==(INT)m_LastSortColumn)
			Item.fmt |= m_LastSortDirection ? HDF_SORTDOWN : HDF_SORTUP;

		pHeaderCtrl->SetItem(a, &Item);
	}

	INT Selected = 0;
	if (p_Airport)
		for (INT a=0; a<m_AirportCount; a++)
			if (p_Airports[a]==p_Airport)
			{
				Selected = a;
				break;
			}

	m_wndAirportList.SetItemState(Selected, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndAirportList.SetItemState(Selected, LVIS_SELECTED, LVIS_SELECTED);
	m_wndAirportList.EnsureVisible(Selected, FALSE);
}

void FMSelectLocationIATADlg::LoadCountry(UINT Country)
{
	m_AirportCount = 0;

	INT Index = FMIATAGetNextAirportByCountry(Country, -1, &p_Airports[m_AirportCount]);
	while ((Index!=-1) && (m_AirportCount<MaxAirportsPerCountry))
		Index = FMIATAGetNextAirportByCountry(Country, Index, &p_Airports[++m_AirportCount]);

	m_wndAirportList.SetItemCount(m_AirportCount);
	Sort();

	m_wndAirportList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	CRect rect;
	m_wndAirportList.GetClientRect(rect);
	m_wndAirportList.SetColumnWidth(1, rect.Width()-m_wndAirportList.GetColumnWidth(0));
	m_wndAirportList.SetFocus();
}

BOOL FMSelectLocationIATADlg::InitDialog()
{
	// Combobox f�llen
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COUNTRY);

	UINT cCount = FMIATAGetCountryCount();
	for (UINT a=0; a<cCount; a++)
		pComboBox->AddString(CString(FMIATAGetCountry(a)->Name));

	// Liste konfigurieren
	m_wndAirportList.AddColumn(0, CString((LPCSTR)IDS_AIRPORT_CODE));
	m_wndAirportList.AddColumn(1, CString((LPCSTR)IDS_AIRPORT_LOCATION));

	// Init
	const UINT Country = p_Airport ? p_Airport->CountryID : m_LastCountrySelected;

	pComboBox->SelectString(-1, CString(FMIATAGetCountry(Country)->Name));
	LoadCountry(Country);

	return p_Airport!=NULL;
}


BEGIN_MESSAGE_MAP(FMSelectLocationIATADlg, FMDialog)
	ON_CONTROL(CBN_SELCHANGE, IDC_COUNTRY, OnSelectCountry)
	ON_NOTIFY(NM_DBLCLK, IDC_AIRPORTS, OnDoubleClick)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_AIRPORTS, OnGetDispInfo)
	ON_NOTIFY(REQUEST_TEXTCOLOR, IDC_AIRPORTS, OnRequestTextColor)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_AIRPORTS, OnRequestTooltipData)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortItems)
END_MESSAGE_MAP()

void FMSelectLocationIATADlg::OnSelectCountry()
{
	m_LastCountrySelected = ((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel();

	LoadCountry(m_LastCountrySelected);
}

void FMSelectLocationIATADlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void FMSelectLocationIATADlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	const INT Index = pItem->iItem;

	if (pItem->mask & LVIF_TEXT)
	{
		LPCSTR pChar = (pItem->iSubItem==0) ? p_Airports[Index]->Code : p_Airports[Index]->Name;
		MultiByteToWideChar(CP_ACP, 0, pChar, -1, pItem->pszText, 256);
	}
}

void FMSelectLocationIATADlg::OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TEXTCOLOR* pTextColor = (NM_TEXTCOLOR*)pNMHDR;

	if (pTextColor->Item!=-1)
	{
		const FMAirport* pAirport = p_Airports[pTextColor->Item];

		if (strcmp(pAirport->Code, pAirport->MetroCode)==0)
			pTextColor->Color = 0x208040;
	}

	*pResult = 0;
}

void FMSelectLocationIATADlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		FMAirport* pAirport = p_Airports[pTooltipData->Item];

		FMTooltip::AppendAttribute(pTooltipData->Hint, 4096, IDS_AIRPORT_NAME, pAirport->Name);
		FMTooltip::AppendAttribute(pTooltipData->Hint, 4096, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);

		CString tmpStr;
		FMGeoCoordinatesToString(pAirport->Location, tmpStr, FALSE);
		FMTooltip::AppendAttribute(pTooltipData->Hint, 4096, IDS_AIRPORT_LOCATION, tmpStr);

		pTooltipData->hBitmap = FMIATACreateAirportMap(pAirport, 192, 192);

		*pResult = TRUE;
	}
	else
	{
		*pResult = FALSE;
	}
}

void FMSelectLocationIATADlg::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
	INT Column = pLV->iItem;

	if (Column!=(INT)m_LastSortColumn)
	{
		m_LastSortColumn = Column;
		m_LastSortDirection = FALSE;
	}
	else
	{
		m_LastSortDirection = !m_LastSortDirection;
	}

	Sort();
	m_wndAirportList.Invalidate();

	*pResult = 0;
}
