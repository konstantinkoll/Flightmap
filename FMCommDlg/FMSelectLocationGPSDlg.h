
// FMSelectLocationGPSDlg.h: Schnittstelle der Klasse FMSelectLocationGPSDlg
//

#pragma once
#include "CMapCtrl.h"


// FMSelectLocationGPSDlg
//

class FMSelectLocationGPSDlg : public CDialog
{
public:
	FMSelectLocationGPSDlg(const FMGeoCoordinates& Location, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	FMGeoCoordinates m_Location;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLatitudeChanged();
	afx_msg void OnLongitudeChanged();

	afx_msg void OnIATA();
	afx_msg void OnReset();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	CMapCtrl m_wndMap;
};
