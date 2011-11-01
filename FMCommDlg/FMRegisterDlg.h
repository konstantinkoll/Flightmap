
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
	FMRegisterDlg(CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CPictureCtrl m_wndUnregistered;
	CPictureCtrl m_wndRegistered;
};
