
// GoogleEarthSettingsDlg.h: Schnittstelle der Klasse GoogleEarthSettingsDlg
//

#pragma once
#include "FMCommDlg.h"


// GoogleEarthSettingsDlg
//

class GoogleEarthSettingsDlg : public FMDialog
{
public:
	GoogleEarthSettingsDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnChangeMergeMetropolitan();
	DECLARE_MESSAGE_MAP()

private:
	CPictureCtrl m_wndMetropolitanPreview;
};
