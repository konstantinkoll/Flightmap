
// AboutDlg.h: Schnittstelle der Klasse AboutDlg
//

#pragma once
#include "FMCommDlg.h"


// AboutDlg
//

class AboutDlg : public FMDialog
{
public:
	AboutDlg(CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL m_UseStatuteMiles;
	BOOL m_UseBgImages;

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(FMLicense* License=NULL);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnEnableAutoUpdate();
	afx_msg void On3DSettings();
	afx_msg void OnExclusive();
	afx_msg void OnUpdateNow();
	DECLARE_MESSAGE_MAP()

private:
	CString m_Version;
	CString m_Copyright;
};
