
// FMRegisterDlg.h: Schnittstelle der Klasse FMRegisterDlg
//

#pragma once
#include "CPictureCtrl.h"
#include "FMDialog.h"


// FMRegisterDlg
//

class FMRegisterDlg : public FMDialog
{
public:
	FMRegisterDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual INT_PTR DoModal();

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnPurchase();
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	CPictureCtrl m_wndUnregistered;
	CPictureCtrl m_wndRegistered;
	CFont m_fntWingdings;
};
