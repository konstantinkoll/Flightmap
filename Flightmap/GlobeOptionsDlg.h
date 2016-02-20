
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "FMCommDlg.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public FMDialog
{
public:
	GlobeOptionsDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

private:
	static void AddQuality(CComboBox& wndCombobox, UINT nResID);

	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;
};
