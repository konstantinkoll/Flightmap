
// FilterDlg.cpp: Implementierung der Klasse FilterDlg
//

#include "stdafx.h"
#include "FilterDlg.h"
#include "Flightmap.h"


// FilterDlg
//

FilterDlg::FilterDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_FILTER, pParentWnd)
{
	p_Itinerary = pItinerary;

	ZeroMemory(&m_Filter, sizeof(m_Filter));
}

void FilterDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_MaskedText(pDX, IDC_FILTER_AIRPORT, m_wndFilterAirport, 0);
	DDX_Control(pDX, IDC_FILTER_CARRIER, m_wndFilterCarrier);
	DDX_Control(pDX, IDC_FILTER_EQUIPMENT, m_wndFilterEquipment);
	DDX_Control(pDX, IDC_FILTER_RATING, m_wndFilterRating);
	DDX_Control(pDX, IDC_FILTER_MONTH, m_wndFilterMonth);
	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_wndSortAttributes);
	DDX_Control(pDX, IDC_ASCENDING, m_wndAscending);

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

		INT Index = m_wndSortAttributes.GetNextItem(-1, LVIS_SELECTED);
		m_Filter.SortBy = (Index==-1) ? -1 : (INT)m_wndSortAttributes.GetItemData(Index);

		m_Filter.Descending = m_wndAscending.GetCheck()>0;
	}
}


BEGIN_MESSAGE_MAP(FilterDlg, CDialog)
	ON_BN_CLICKED(IDD_SELECTIATA, OnSelectIATA)
END_MESSAGE_MAP()

BOOL FilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadDialogIcon(IDD_FILTER);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Filter
	PrepareCarrierCtrl(&m_wndFilterCarrier, p_Itinerary, FALSE);
	PrepareEquipmentCtrl(&m_wndFilterEquipment, p_Itinerary, FALSE);
	m_wndFilterRating.SetRating(0);

	// Sort
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS;
	m_wndSortAttributes.SetExtendedStyle(m_wndSortAttributes.GetExtendedStyle() | dwExStyle);

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

	m_wndAscending.SetCheck(1);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FilterDlg::OnSelectIATA()
{
	CString tmpStr;
	m_wndFilterAirport.GetWindowText(tmpStr);

	CHAR Code[4];
	WideCharToMultiByte(CP_ACP, 0, tmpStr.GetBuffer(), -1, Code, 4, NULL, NULL);

	FMSelectLocationIATADlg dlg(this, Code);
	if (dlg.DoModal()==IDOK)
	{
		tmpStr = dlg.p_Airport->Code;
		m_wndFilterAirport.SetWindowText(tmpStr);
	}
}
