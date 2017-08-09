
// FMSelectLocationGPSDlg.h: Schnittstelle der Klasse FMSelectLocationGPSDlg
//

#pragma once
#include "CMapCtrl.h"
#include "FMDialog.h"


// FMSelectLocationGPSDlg
//

class FMSelectLocationGPSDlg : public FMDialog
{
public:
	FMSelectLocationGPSDlg(const FMGeoCoordinates& Location, CWnd* pParentWnd=NULL);

	FMGeoCoordinates m_Location;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLatitudeChanged();
	afx_msg void OnLongitudeChanged();

	afx_msg void OnIATA();
	afx_msg void OnReset();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CMapCtrl m_wndMap;

private:
	static DOUBLE StringToCoord(LPCWSTR Str);
};
