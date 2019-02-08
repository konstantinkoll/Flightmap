
// FilterDlg.cpp: Implementierung der Klasse FilterDlg
//

#include "stdafx.h"
#include "FilterDlg.h"
#include "Flightmap.h"


// CSortList
//

CSortList::CSortList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(AttributeItemData))
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CSortList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CSortList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Item
	CFrontstageScroller::SetItemHeight(FMGetApp()->m_DialogFont.GetFontHeight()+2*ITEMCELLPADDINGY);
}


// Layouts

void CSortList::AdjustLayout()
{
	AdjustLayoutColumns();
}


// Item data

void CSortList::AddAttribute(UINT Attr, LPCWSTR Name)
{
	AttributeItemData Data;

	Data.Attr = Attr;
	wcscpy_s(Data.Name, 256, Name);

	AddItem(&Data);
}

void CSortList::SetAttributes()
{
	SetItemCount(FMAttributeCount, FALSE);

	for (UINT a=0; a<FMAttributeCount; a++)
		if (FMAttributes[a].Sortable)
			AddAttribute(a, CString((LPCSTR)FMAttributes[a].nNameID));

	LastItem();
	AdjustLayout();
}


// Item selection

INT CSortList::GetSelectedSortAttribute() const
{
	const INT Index = GetSelectedItem();

	return (Index<0) ? -1 : GetAttributeItemData(Index)->Attr;
}


// Drawing

void CSortList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL /*Themed*/)
{
	const AttributeItemData* pData = GetAttributeItemData(Index);

	CRect rect(rectItem);
	rect.DeflateRect(ITEMCELLPADDINGX, ITEMCELLPADDINGY);

	// Text
	CFont* pOldFont = dc.SelectObject(&FMGetApp()->m_DialogFont);
	dc.DrawText(pData->Name, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(pOldFont);
}


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
		WideCharToMultiByte(CP_ACP, 0, tmpStr, -1, m_Filter.Airport, 4, NULL, NULL);

		m_wndFilterCarrier.GetWindowText(m_Filter.Carrier, 256);
		m_wndFilterEquipment.GetWindowText(m_Filter.Equipment, 256);

		m_Filter.Business = ((CButton*)GetDlgItem(IDC_FILTER_BUSINESSTRIP))->GetCheck();
		m_Filter.Leisure = ((CButton*)GetDlgItem(IDC_FILTER_LEISURETRIP))->GetCheck();
		m_Filter.Rating = m_wndFilterRating.GetRating();

		if ((m_Filter.DepartureMonth=m_wndFilterMonth.GetCurSel())==CB_ERR)
			m_Filter.DepartureMonth = 0;

		GetDlgItem(IDC_FILTER_YEAR)->GetWindowText(tmpStr);
		if (swscanf_s(tmpStr, L"%u", &m_Filter.DepartureYear)!=1)
			m_Filter.DepartureYear = 0;

		m_Filter.SortBy = GetSelectedSortAttribute();
		m_Filter.Descending = m_wndSortDirection.GetCurSel();
	}
}

BOOL FilterDlg::InitDialog()
{
	GetDlgItem(IDC_INSTRUCTIONS)->SetFont(&theApp.m_DefaultFont);

	// Filter
	p_Itinerary->PrepareCarrierCtrl(m_wndFilterCarrier, FALSE);
	p_Itinerary->PrepareEquipmentCtrl(m_wndFilterEquipment, FALSE);

	m_wndFilterRating.SetInitialRating(0);

	// Sorting
	m_wndSortAttributes.SetAttributes();
	m_wndSortDirection.SetCurSel(0);

	return TRUE;
}


BEGIN_MESSAGE_MAP(FilterDlg, FMDialog)
	ON_NOTIFY(IVN_SELECTIONCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChanged)
	ON_BN_CLICKED(IDD_SELECTLOCATIONIATA, OnSelectIATA)
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

void FilterDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_wndSortDirection.EnableWindow(GetSelectedSortAttribute()!=-1);

	*pResult = 0;
}
