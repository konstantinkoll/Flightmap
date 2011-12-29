
// FMSelectLocationGPSDlg.h: Schnittstelle der Klasse FMSelectLocationGPSDlg
//

#pragma once
#include "FMCommDlg.h"
#include "CMapSelectionCtrl.h"


// FMSelectLocationGPSDlg
//

class FMSelectLocationGPSDlg : public CDialog
{
public:
	FMSelectLocationGPSDlg(CWnd* pParentWnd, const FMGeoCoordinates Location);

	virtual void DoDataExchange(CDataExchange* pDX);

	FMGeoCoordinates m_Location;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReset();
	afx_msg void OnLatitudeChanged();
	afx_msg void OnLongitudeChanged();
	DECLARE_MESSAGE_MAP()

private:
	CMapSelectionCtrl m_Map;
};
