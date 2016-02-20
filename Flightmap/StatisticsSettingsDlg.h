
// StatisticsSettingsDlg.h: Schnittstelle der Klasse StatisticsSettingsDlg
//

#pragma once
#include "FMCommDlg.h"


// StatisticsSettingsDlg
//

class StatisticsSettingsDlg : public FMDialog
{
public:
	StatisticsSettingsDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnChangeMergeMetropolitan();
	DECLARE_MESSAGE_MAP()

private:
	CPictureCtrl m_wndMetropolitanPreview;
};
