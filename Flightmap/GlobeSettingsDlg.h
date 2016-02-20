
// GlobeSettingsDlg.h: Schnittstelle der Klasse GlobeSettingsDlg
//

#pragma once
#include "FMCommDlg.h"


// GlobeSettingsDlg
//

class GlobeSettingsDlg : public FMDialog
{
public:
	GlobeSettingsDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnChangeMergeMetropolitan();
	DECLARE_MESSAGE_MAP()

private:
	CPictureCtrl m_wndMetropolitanPreview;
};
