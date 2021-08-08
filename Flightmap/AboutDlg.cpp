
// AboutDlg.cpp: Implementierung der Klasse AboutDlg
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "Flightmap.h"


// AboutDlg
//

AboutDlg::AboutDlg(CWnd* pParentWnd)
	: FMAboutDialog(0x0009, pParentWnd)
{
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	FMAboutDialog::DoDataExchange(pDX);

	// Units
	DDX_Radio(pDX, IDC_NAUTICALMILES, theApp.m_UseStatuteMiles);

	// 3D settings
	DDX_Control(pDX, IDC_MODELQUALITY, m_wndModelQuality);
	DDX_Control(pDX, IDC_TEXTUREQUALITY, m_wndTextureQuality);
	DDX_Check(pDX, IDC_TEXTURECOMPRESS, theApp.m_TextureCompress);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_ModelQuality = (GLModelQuality)m_wndModelQuality.GetCurSel();
		theApp.m_TextureQuality = (GLTextureQuality)m_wndTextureQuality.GetCurSel();
	}

	// Globe
	DDX_Check(pDX, IDM_GLOBEWND_SHOWLOCATIONS, theApp.m_GlobeShowLocations);
	DDX_Check(pDX, IDM_GLOBEWND_SHOWAIRPORTIATA, theApp.m_GlobeShowAirportIATA);
	DDX_Check(pDX, IDM_GLOBEWND_SHOWAIRPORTNAMES, theApp.m_GlobeShowAirportNames);
	DDX_Check(pDX, IDM_GLOBEWND_SHOWCOORDINATES, theApp.m_GlobeShowCoordinates);
	DDX_Check(pDX, IDM_GLOBEWND_SHOWDESCRIPTIONS, theApp.m_GlobeShowDescriptions);
	DDX_Check(pDX, IDM_GLOBEWND_DARKBACKGROUND, theApp.m_GlobeDarkBackground);
}

BOOL AboutDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!FMTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_ABOUT_GENERAL, pszTabArea);
	AddTab(IDD_ABOUT_VIEWS, pszTabArea);
	AddTab(IDD_ABOUT_UNITS, pszTabArea);

	return TRUE;
}

void AboutDlg::AddQualityString(CComboBox& wndCombobox, UINT nResID)
{
	wndCombobox.AddString(CString((LPCSTR)nResID));
}

BOOL AboutDlg::InitDialog()
{
	// Initialize OpenGL
	theRenderer.Initialize();

	// 3D model
	AddQualityString(m_wndModelQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxModelQuality>=MODELMEDIUM)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxModelQuality>=MODELHIGH)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_HIGH);

	if (theRenderer.m_MaxModelQuality>=MODELULTRA)
		AddQualityString(m_wndModelQuality, IDS_QUALITY_ULTRA);

	m_wndModelQuality.SetCurSel(min((INT)theApp.m_ModelQuality, (INT)theRenderer.m_MaxModelQuality));

	// Texture
	AddQualityString(m_wndTextureQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREMEDIUM)
		AddQualityString(m_wndTextureQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREULTRA)
		AddQualityString(m_wndTextureQuality, IDS_QUALITY_ULTRA);

	m_wndTextureQuality.SetCurSel(min((INT)theApp.m_TextureQuality, (INT)theRenderer.m_MaxTextureQuality));

	// Disabled controls
	GetDlgItem(IDC_TEXTURECOMPRESS)->EnableWindow(theRenderer.m_SupportsTextureCompression);

	return FMAboutDialog::InitDialog();
}
