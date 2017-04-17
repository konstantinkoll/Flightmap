
// FMSelectLocationIATADlg.h: Schnittstelle der Klasse FMSelectLocationIATADlg
//

#pragma once
#include "CExplorerList.h"
#include "FMDialog.h"


// FMSelectLocationIATADlg
//

#define MaxAirportsPerCountry   2500

class FMSelectLocationIATADlg : public FMDialog
{
public:
	FMSelectLocationIATADlg(CWnd* pParentWnd=NULL, LPCSTR pAirport=NULL);

	FMAirport* p_Airport;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	void Sort();
	void LoadCountry(UINT Country);

	afx_msg void OnSelectCountry();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTextColor(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItems(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	UINT m_LastCountrySelected;
	UINT m_LastSortColumn;
	BOOL m_LastSortDirection;

	CExplorerList m_wndAirportList;
	FMAirport* p_Airports[MaxAirportsPerCountry];
	INT m_AirportCount;

private:
	INT Compare(INT n1, INT n2);
	static void Swap(FMAirport*& Eins, FMAirport*& Zwei);
	void Heap(INT Wurzel, INT Anzahl);
};

inline void FMSelectLocationIATADlg::Swap(FMAirport*& Eins, FMAirport*& Zwei)
{
	FMAirport* Temp = Eins;
	Eins = Zwei;
	Zwei = Temp;
}
