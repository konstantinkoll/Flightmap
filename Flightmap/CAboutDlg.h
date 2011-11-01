
// CAboutDlg.h: Schnittstelle der Klasse CAboutDlg
//

#pragma once
#include "FMCommDlg.h"


// CAboutDlg
//

class CAboutDlg : public FMDialog
{
public:
	CAboutDlg(CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL m_UseStatuteMiles;
	BOOL m_ReduceVisuals;

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(FMLicense* License=NULL);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnEnableAutoUpdate();
	afx_msg void OnUpdateNow();
	DECLARE_MESSAGE_MAP()

private:
	CString m_Version;
	CString m_Copyright;
};
