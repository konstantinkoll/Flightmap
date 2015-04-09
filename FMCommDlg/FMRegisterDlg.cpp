
// FMRegisterDlg.cpp: Implementierung der Klasse FMRegisterDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMRegisterDlg
//

FMRegisterDlg::FMRegisterDlg(CWnd* pParentWnd)
	: FMDialog(IDD_REGISTER, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(FMRegisterDlg, FMDialog)
	ON_BN_CLICKED(IDC_PURCHASE, OnPurchase)
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

void FMRegisterDlg::OnPurchase()
{
	CCmdUI cmd;
	cmd.m_nID = ID_APP_PURCHASE;

	p_App->OnCmdMsg(ID_APP_PURCHASE, CN_COMMAND, &cmd, NULL);
}

void FMRegisterDlg::OnEnterLicenseKey()
{
	FMLicenseDlg dlg(this);
	dlg.DoModal();

	if (FMIsLicensed())
		EndDialog(IDOK);
}
