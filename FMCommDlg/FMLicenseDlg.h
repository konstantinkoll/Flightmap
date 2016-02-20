
// FMLicenseDlg.h: Schnittstelle der Klasse FMLicenseDlg
//

#pragma once
#include "FMDialog.h"


// FMLicenseDlg
//

class FMLicenseDlg : public FMDialog
{
public:
	FMLicenseDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnLoadLicense();
	afx_msg void OnChange();
	DECLARE_MESSAGE_MAP()
};
