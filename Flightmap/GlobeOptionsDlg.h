
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "Flightmap.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public CDialog
{
public:
	GlobeOptionsDlg(CWnd* pParent);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewport();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_wndTextureSize;
	CButton m_wndViewport;
};
