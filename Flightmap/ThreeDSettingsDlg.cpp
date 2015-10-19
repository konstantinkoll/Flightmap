
// ThreeDSettingsDlg.cpp: Implementierung der Klasse ThreeDSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "ThreeDSettingsDlg.h"


// ThreeDSettingsDlg
//

ThreeDSettingsDlg::ThreeDSettingsDlg(CWnd* pParentWnd)
	: CDialog(IDD_3DSETTINGS, pParentWnd)
{
}

void ThreeDSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_TEXTURESIZE, m_wndTextureSize);

	DDX_Check(pDX, IDC_ANTIALISING, theApp.m_GlobeAntialising);
	DDX_Check(pDX, IDC_LIGHTING, theApp.m_GlobeLighting);
	DDX_Check(pDX, IDC_ATMOSPHERE, theApp.m_GlobeAtmosphere);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_nTextureSize = m_wndTextureSize.GetCurSel();
		theApp.Broadcast(WM_3DSETTINGSCHANGED);
	}
}


BEGIN_MESSAGE_MAP(ThreeDSettingsDlg, CDialog)
END_MESSAGE_MAP()

BOOL ThreeDSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Texturgr��e
	CString tmpStr((LPCSTR)IDS_AUTOMATIC);
	m_wndTextureSize.AddString(tmpStr);
	m_wndTextureSize.AddString(_T("1024�1024"));
	m_wndTextureSize.AddString(_T("2048�2048"));
	m_wndTextureSize.AddString(_T("4096�4096"));
	m_wndTextureSize.AddString(_T("8192�4096"));
	m_wndTextureSize.SetCurSel(theApp.m_nTextureSize);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
