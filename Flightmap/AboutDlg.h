
// AboutDlg.h: Schnittstelle der Klasse AboutDlg
//

#pragma once
#include "FMCommDlg.h"


// AboutDlg
//

class AboutDlg : public FMDialog
{
public:
	AboutDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL m_UseStatuteMiles;
	BOOL m_UseBgImages;

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnableAutoUpdate();
	afx_msg void On3DSettings();
	afx_msg void OnUpdateNow();
	afx_msg void OnVersionInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	void CheckLicenseKey();
	void CheckInternetConnection();

	Bitmap* p_Logo;
	Bitmap* p_Santa;
	FMFont m_CaptionFont;
	FMFont m_VersionFont;
	INT m_CaptionTop;
	INT m_IconTop;
	CString m_Version;
	CString m_Copyright;
	CString m_AppName;
	WCHAR m_Build[256];
	CWnd m_wndVersionInfo;
};
