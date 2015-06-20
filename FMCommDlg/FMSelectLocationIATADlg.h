
// FMSelectLocationIATADlg.h: Schnittstelle der Klasse FMSelectLocationIATADlg
//

#pragma once
#include "CMapPreviewCtrl.h"
#include "IATA.h"


// FMSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class FMSelectLocationIATADlg : public CDialog
{
public:
	FMSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd=NULL, CHAR* Airport=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	FMAirport* p_Airport;
	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

protected:
	void Sort();
	void LoadCountry(UINT country);
	void UpdatePreview();

private:
	INT Compare(INT n1, INT n2);
	void Heap(INT Wurzel, INT Anz);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectCountry();
	afx_msg void OnReportError(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CListCtrl m_wndList;
	CMapPreviewCtrl m_wndMap;
	FMAirport* m_Airports[MaxAirportsPerCountry];
	INT m_nAirports;
	UINT m_nIDTemplate;
	WCHAR m_Buffer[256];
};
