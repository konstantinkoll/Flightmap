
// ThreeDSettingsDlg.h: Schnittstelle der Klasse ThreeDSettingsDlg
//

#pragma once


// ThreeDSettingsDlg
//

class ThreeDSettingsDlg : public CDialog
{
public:
	ThreeDSettingsDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_wndTextureSize;
};
