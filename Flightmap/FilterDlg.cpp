
// FilterDlg.cpp: Implementierung der Klasse FilterDlg
//

#include "stdafx.h"
#include "FilterDlg.h"
#include "Flightmap.h"


// FilterDlg
//

FilterDlg::FilterDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: FMDialog(IDD_FILTER, pParentWnd)
{
	p_Itinerary = pItinerary;

	ZeroMemory(&m_Filter, sizeof(m_Filter));
}

void FilterDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_MaskedText(pDX, IDC_FILTER_AIRPORT, m_wndFilterAirport, 0);
	DDX_Control(pDX, IDC_FILTER_CARRIER, m_wndFilterCarrier);
	DDX_Control(pDX, IDC_FILTER_EQUIPMENT, m_wndFilterEquipment);
	DDX_Control(pDX, IDC_FILTER_RATING, m_wndFilterRating);
	DDX_Control(pDX, IDC_FILTER_MONTH, m_wndFilterMonth);
	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_wndSortAttributes);
	DDX_Control(pDX, IDC_SORTDIRECTION, m_wndSortDirection);

	if (pDX->m_bSaveAndValidate)
	{
		CString tmpStr;
		m_wndFilterAirport.GetWindowText(tmpStr);
		WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, m_Filter.Airport, 4, NULL, NULL);

		m_wndFilterCarrier.GetWindowText(m_Filter.Carrier, 256);
		m_wndFilterEquipment.GetWindowText(m_Filter.Equipment, 256);

		m_Filter.Business = ((CButton*)GetDlgItem(IDC_FILTER_BUSINESSTRIP))->GetCheck();
		m_Filter.Leisure = ((CButton*)GetDlgItem(IDC_FILTER_LEISURETRIP))->GetCheck();
		m_Filter.Rating = m_wndFilterRating.GetRating();

		m_Filter.DepartureMonth = m_wndFilterMonth.GetCurSel();
		if (m_Filter.DepartureMonth==CB_ERR)
			m_Filter.DepartureMonth = 0;

		GetDlgItem(IDC_FILTER_YEAR)->GetWindowText(tmpStr);
		if (swscanf_s(tmpStr.GetBuffer(), L"%u", &m_Filter.DepartureYear)!=1)
			m_Filter.DepartureYear = 0;

		const INT Index = m_wndSortAttributes.GetNextItem(-1, LVIS_SELECTED);
		m_Filter.SortBy = (Index==-1) ? -1 : (INT)m_wndSortAttributes.GetItemData(Index);

		m_Filter.Descending = m_wndSortDirection.GetCurSel();
	}
}

BOOL FilterDlg::InitDialog()
{
	GetDlgItem(IDC_INSTRUCTIONS)->SetFont(&FMGetApp()->m_DefaultFont);

	// Filter
	PrepareCarrierCtrl(&m_wndFilterCarrier, p_Itinerary, FALSE);
	PrepareEquipmentCtrl(&m_wndFilterEquipment, p_Itinerary, FALSE);
	m_wndFilterRating.SetRating(0);

	// Sort attributes
	m_wndSortAttributes.AddColumn(0);

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_PARAM;

	for (UINT a=0; a<FMAttributeCount; a++)
		if (FMAttributes[a].Sortable)
		{
			CString tmpStr((LPCSTR)FMAttributes[a].nNameID);

			lvi.lParam = (LPARAM)a;
			lvi.pszText = tmpStr.GetBuffer();
			lvi.iItem = m_wndSortAttributes.GetItemCount();
			m_wndSortAttributes.InsertItem(&lvi);
		}

	// Sort direction
	m_wndSortDirection.SetCurSel(0);

	return TRUE;
}


BEGIN_MESSAGE_MAP(FilterDlg, FMDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChange)
	ON_BN_CLICKED(IDD_SELECTIATA, OnSelectIATA)
END_MESSAGE_MAP()

void FilterDlg::OnSelectIATA()
{
	WCHAR CodeW[4];
	m_wndFilterAirport.GetWindowText(CodeW, 4);

	CHAR CodeA[4];
	WideCharToMultiByte(CP_ACP, 0, CodeW, -1, CodeA, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(this, CodeA);
	if (dlg.DoModal()==IDOK)
		m_wndFilterAirport.SetWindowText(CString(dlg.p_Airport->Code));
}

void FilterDlg::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged & LVIF_STATE)
		m_wndSortDirection.EnableWindow(pNMListView->uNewState & LVIS_SELECTED);

	*pResult = 0;
}
