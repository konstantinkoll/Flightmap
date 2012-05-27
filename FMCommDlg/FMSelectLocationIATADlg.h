
// FMSelectLocationIATADlg.h: Schnittstelle der Klasse FMSelectLocationIATADlg
//

#pragma once
#include "FMCommDlg.h"
#include "CMapPreviewCtrl.h"


// FMSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class FMSelectLocationIATADlg : public CDialog
{
public:
	FMSelectLocationIATADlg(UINT nIDTemplate, CWnd* pParentWnd, CHAR* Airport=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	FMAirport* p_Airport;
	UINT m_LastCountrySelected;

protected:
	void LoadCountry(UINT country, BOOL SelectFirst=TRUE);
	void UpdatePreview();

private:
	CMapPreviewCtrl m_Map;
	FMAirport* m_Airports[MaxAirportsPerCountry];
	INT m_nAirports;
	UINT m_nIDTemplate;
	WCHAR m_Buffer[256];
	FMApplication* p_App;

	INT Compare(INT col, INT n1, INT n2);
	void Heap(INT col, INT wurzel, INT anz);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectCountry();
	afx_msg void OnReportError(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
