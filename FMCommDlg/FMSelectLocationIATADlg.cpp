
// FMSelectLocationIATADlg.cpp: Implementierung der Klasse FMSelectLocationIATA
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMSelectLocationIATADlg
//

FMSelectLocationIATADlg::FMSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd, CHAR* Airport)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;

	m_LastCountrySelected = FMGetApp()->GetInt(_T("IATALastCountrySelected"), 0);
	m_LastSortColumn = FMGetApp()->GetInt(_T("IATALastSortColumn"), 0);
	m_LastSortDirection = FMGetApp()->GetInt(_T("IATALastSortDirection"), FALSE);

	if (Airport)
	{
		if (!FMIATAGetAirportByCode(Airport, &p_Airport))
			p_Airport = NULL;
	}
	else
	{
		p_Airport = NULL;
	}
}

void FMSelectLocationIATADlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAP_PREVIEW, m_wndMap);
	DDX_Control(pDX, IDC_AIRPORTS, m_wndList);

	if (pDX->m_bSaveAndValidate)
	{
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
		Result = strcmp(m_Airports[n1]->Code, m_Airports[n2]->Code);
		break;
	case 1:
		Result = strcmp(m_Airports[n1]->Name, m_Airports[n2]->Name);
		break;
	}

	if (m_LastSortDirection)
		Result = -Result;

	return Result;
}

void FMSelectLocationIATADlg::Heap(INT wurzel, INT anz)
{
	while (wurzel<=anz/2-1)
	{
		INT idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (Compare(idx, idx+1)<0)
				idx++;
		if (Compare(wurzel, idx)<0)
		{
			std::swap(m_Airports[wurzel], m_Airports[idx]);
			wurzel = idx;
		}
		else
		{
			break;
		}
	}
}

void FMSelectLocationIATADlg::Sort()
{
	if (m_nAirports>1)
	{
		for (INT a=m_nAirports/2-1; a>=0; a--)
			Heap(a, m_nAirports);
		for (INT a=m_nAirports-1; a>0; )
		{
			std::swap(m_Airports[0], m_Airports[a]);
			Heap(0, a--);
		}
	}

	CHeaderCtrl* pHeaderCtrl = m_wndList.GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		HDITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = HDI_FORMAT;

		for (INT a=0; a<2; a++)
		{
			pHeaderCtrl->GetItem(a, &item);

			item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			if (a==(INT)m_LastSortColumn)
				item.fmt |= m_LastSortDirection ? HDF_SORTDOWN : HDF_SORTUP;

			pHeaderCtrl->SetItem(a, &item);
		}
	}

	INT sel = 0;
	if (p_Airport)
	{
		for (INT a=0; a<m_nAirports; a++)
			if (m_Airports[a]==p_Airport)
			{
				sel = a;
				break;
			}
	}
	m_wndList.SetItemState(sel, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndList.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
	m_wndList.EnsureVisible(sel, FALSE);
}

void FMSelectLocationIATADlg::LoadCountry(UINT country)
{
	m_wndList.SetRedraw(FALSE);
	m_wndList.SetItemCount(0);

	m_nAirports = 0;

	INT idx = FMIATAGetNextAirportByCountry(country, -1, &m_Airports[m_nAirports]);
	while ((idx!=-1) && (m_nAirports<MaxAirportsPerCountry))
		idx = FMIATAGetNextAirportByCountry(country, idx, &m_Airports[++m_nAirports]);

	m_wndList.SetItemCount(m_nAirports);
	Sort();

	m_wndList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_wndList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();

	UpdatePreview();
}

void FMSelectLocationIATADlg::UpdatePreview()
{
	INT idx = m_wndList.GetNextItem(-1, LVIS_SELECTED);

	p_Airport = m_Airports[idx];
	m_wndMap.Update(p_Airport);

	CString tmpStr;
	FMGeoCoordinatesToString(p_Airport->Location, tmpStr, false);
	GetDlgItem(IDC_GPSLOCATION)->SetWindowText(tmpStr);
}


BEGIN_MESSAGE_MAP(FMSelectLocationIATADlg, CDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_AIRPORTS, OnCustomDraw)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_AIRPORTS, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, IDC_AIRPORTS, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_AIRPORTS, OnItemChanged)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnSortItems)
	ON_CONTROL(CBN_SELCHANGE, IDC_COUNTRY, OnSelectCountry)
	ON_NOTIFY(NM_CLICK, IDC_REPORTERROR, OnReportError)
END_MESSAGE_MAP()

BOOL FMSelectLocationIATADlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = FMGetApp()->LoadDialogIcon(m_nIDTemplate);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Combobox f�llen
	CComboBox* c = (CComboBox*)GetDlgItem(IDC_COUNTRY);
	UINT cCount = FMIATAGetCountryCount();
	for (UINT a=0; a<cCount; a++)
	{
		CString tmpStr(&FMIATAGetCountry(a)->Name[0]);
		c->AddString(tmpStr);
	}

	// Liste konfigurieren
	m_wndList.SetExtendedStyle(m_wndList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);

	CString tmpStr;

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_CODE));
	lvc.pszText = tmpStr.GetBuffer();
	m_wndList.InsertColumn(0, &lvc);

	ENSURE(tmpStr.LoadString(IDS_AIRPORT_LOCATION));
	lvc.pszText = tmpStr.GetBuffer();
	lvc.iSubItem = 1;
	m_wndList.InsertColumn(1, &lvc);

	// Init
	UINT country = p_Airport ? p_Airport->CountryID : m_LastCountrySelected;
	tmpStr = FMIATAGetCountry(country)->Name;
	c->SelectString(-1, tmpStr);
	LoadCountry(country);

	return TRUE;
}

void FMSelectLocationIATADlg::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT==pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else
		if (CDDS_ITEMPREPAINT==pLVCD->nmcd.dwDrawStage)
		{
			INT idx = (INT)pLVCD->nmcd.dwItemSpec;

			if (strcmp(m_Airports[idx]->Code, m_Airports[idx]->MetroCode)==0)
				pLVCD->clrText = 0xFF0000;
		}
}

void FMSelectLocationIATADlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	INT idx = pItem->iItem;

	if (pItem->mask & LVIF_TEXT)
	{
		CHAR* src = (pItem->iSubItem==0) ? &m_Airports[idx]->Code[0] : &m_Airports[idx]->Name[0];
		MultiByteToWideChar(CP_ACP, 0, src, -1, m_Buffer, 256);
		pItem->pszText = (LPWSTR)m_Buffer;
	}
}

void FMSelectLocationIATADlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void FMSelectLocationIATADlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
		UpdatePreview();
}

void FMSelectLocationIATADlg::OnSortItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pLV = (NMLISTVIEW*)pNMHDR;
	INT col = pLV->iItem;

	if (col!=(INT)m_LastSortColumn)
	{
		m_LastSortColumn = col;
		m_LastSortDirection = FALSE;
	}
	else
	{
		m_LastSortDirection = !m_LastSortDirection;
	}

	m_wndList.SetRedraw(FALSE);

	Sort();

	m_wndList.SetRedraw(TRUE);
	m_wndList.Invalidate();

	*pResult = 0;
}

void FMSelectLocationIATADlg::OnSelectCountry()
{
	m_LastCountrySelected = ((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel();
	LoadCountry(m_LastCountrySelected);
}

void FMSelectLocationIATADlg::OnReportError(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CString Subject = _T("IATA database error");

	INT idx = m_wndList.GetNextItem(-1, LVIS_SELECTED);
	if (idx!=-1)
	{
		CString Code(m_Airports[idx]->Code);
		CString Name(m_Airports[idx]->Name);
		CString Country(&FMIATAGetCountry(m_Airports[idx]->CountryID)->Name[0]);
		Subject += _T(": ")+Code+_T(" (")+Name+_T(", ")+Country+_T(")");
	}

	FMGetApp()->SendMail(Subject);

	*pResult = 0;
}
