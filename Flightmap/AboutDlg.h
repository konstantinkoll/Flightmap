
// AboutDlg.h: Schnittstelle der Klasse LFAboutDlg
//

#pragma once
#include "FMCommDlg.h"


// AboutDlg
//

class AboutDlg : public FMDialog
{
public:
	AboutDlg(CWnd* pParentWnd=NULL);

	BOOL m_UseStatuteMiles;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);
	virtual BOOL InitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnableAutoUpdate();
	afx_msg void OnUpdateNow();
	afx_msg void OnVersionInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	void CheckLicenseKey();
	void CheckInternetConnection();
	static void AddQuality(CComboBox& wndCombobox, UINT nResID);

	Bitmap* p_Santa;
	Bitmap* p_Logo;
	FMFont m_CaptionFont;
	FMFont m_VersionFont;
	INT m_IconTop;
	INT m_CaptionTop;
	CString m_Version;
	CString m_Copyright;
	CString m_AppName;
	WCHAR m_Build[256];
	CWnd m_wndVersionInfo;
	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;
};
