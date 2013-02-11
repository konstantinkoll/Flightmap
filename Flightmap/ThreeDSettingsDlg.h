
// ThreeDSettingsDlg.h: Schnittstelle der Klasse ThreeDSettingsDlg
//

#pragma once
#include "Flightmap.h"


// ThreeDSettingsDlg
//

class ThreeDSettingsDlg : public CDialog
{
public:
	ThreeDSettingsDlg(CWnd* pParentWnd);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_wndTextureSize;
};
