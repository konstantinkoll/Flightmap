
// FMLicenseDlg.h: Schnittstelle der Klasse FMLicenseDlg
//

#pragma once
#include "FMDialog.h"


// FMLicenseDlg
//

class FMLicenseDlg : public FMDialog
{
public:
	FMLicenseDlg(CWnd* pParent=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnDestroy();
	afx_msg void OnLoadLicense();
	DECLARE_MESSAGE_MAP()
};
