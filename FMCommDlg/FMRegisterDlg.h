
// FMRegisterDlg.h: Schnittstelle der Klasse FMRegisterDlg
//

#pragma once
#include "FMDialog.h"


// FMRegisterDlg
//

class FMRegisterDlg : public FMDialog
{
public:
	FMRegisterDlg(CWnd* pParentWnd=NULL);

protected:
	afx_msg void OnPurchase();
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()
};
