
// AboutDlg.h: Schnittstelle der Klasse AboutDlg
//

#pragma once
#include "FMCommDlg.h"


// AboutDlg
//

class AboutDlg : public FMAboutDialog
{
public:
	AboutDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;

private:
	static void AddQualityString(CComboBox& wndCombobox, UINT nResID);
};
