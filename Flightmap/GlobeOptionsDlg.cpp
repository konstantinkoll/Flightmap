
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(CWnd* pParentWnd)
	: FMDialog(IDD_GLOBEOPTIONS, pParentWnd)
{
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MODELQUALITY, m_wndModelQuality);
	DDX_Control(pDX, IDC_TEXTUREQUALITY, m_wndTextureQuality);

	DDX_Check(pDX, IDC_TEXTURECOMPRESS, theApp.m_TextureCompress);
	DDX_Check(pDX, IDC_SPOTS, theApp.m_GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTIATA, theApp.m_GlobeShowAirportIATA);
	DDX_Check(pDX, IDC_AIRPORTNAMES, theApp.m_GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, theApp.m_GlobeShowGPS);
	DDX_Check(pDX, IDC_MOVEMENTS, theApp.m_GlobeShowMovements);
	DDX_Check(pDX, IDC_DARKBACKGROUND, theApp.m_GlobeDarkBackground);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.m_ModelQuality = (GLModelQuality)m_wndModelQuality.GetCurSel();
		theApp.m_TextureQuality = (GLTextureQuality)m_wndTextureQuality.GetCurSel();

		theApp.Broadcast(WM_3DSETTINGSCHANGED);
	}
}

void GlobeOptionsDlg::AddQuality(CComboBox& wndCombobox, UINT nResID)
{
	CString tmpStr((LPCSTR)nResID);
	wndCombobox.AddString(tmpStr);
}

BOOL GlobeOptionsDlg::InitDialog()
{
	// Initialize OpenGL
	theRenderer.Initialize();

	// 3D model
	AddQuality(m_wndModelQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxModelQuality>=MODELMEDIUM)
		AddQuality(m_wndModelQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxModelQuality>=MODELHIGH)
		AddQuality(m_wndModelQuality, IDS_QUALITY_HIGH);

	if (theRenderer.m_MaxModelQuality>=MODELULTRA)
		AddQuality(m_wndModelQuality, IDS_QUALITY_ULTRA);

	m_wndModelQuality.SetCurSel(min((INT)theApp.m_ModelQuality, (INT)theRenderer.m_MaxModelQuality));

	// Texture
	AddQuality(m_wndTextureQuality, IDS_QUALITY_LOW);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREMEDIUM)
		AddQuality(m_wndTextureQuality, IDS_QUALITY_MEDIUM);

	if (theRenderer.m_MaxTextureQuality>=TEXTUREULTRA)
		AddQuality(m_wndTextureQuality, IDS_QUALITY_ULTRA);

	m_wndTextureQuality.SetCurSel(min((INT)theApp.m_TextureQuality, (INT)theRenderer.m_MaxTextureQuality));

	// Disabled controls
	GetDlgItem(IDC_TEXTURECOMPRESS)->EnableWindow(theRenderer.m_SupportsTextureCompression);

	return TRUE;
}
