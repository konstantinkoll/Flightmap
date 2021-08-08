
// FMAboutDialog.h: Schnittstelle der Klasse FMAboutDialog
//

#pragma once
#include "FMTabbedDialog.h"
#include "FMFont.h"


// FMAboutDialog
//

class FMAboutDialog : public FMTabbedDialog
{
public:
	FMAboutDialog(USHORT BackgroundTabMask, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnVersionInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnableAutoUpdate();
	afx_msg void OnUpdateNow();
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

	static UINT m_LastTab;
	SHORT m_BackgroundTabMask;
	CWnd m_wndVersionInfo;

private:
	BOOL ShowBackgroundOnTab(UINT Index) const;
	void CheckInternetConnection();

	Bitmap* p_AppLogo;
	Bitmap* p_SantaHat;
	CPoint m_ptAppLogo;

	FMFont m_CaptionFont;
	FMFont m_VersionFont;

	CString m_AppName;
	CString m_Version;
	WCHAR m_BuildInfo[256];
	CString m_Copyright;

	CButton m_wndAutoUpdate;
};

inline BOOL FMAboutDialog::ShowBackgroundOnTab(UINT Index) const
{
	return (m_BackgroundTabMask & (1<<Index));
}
