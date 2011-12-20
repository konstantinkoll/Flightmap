
// GlobeOptionsDlg.cpp: Implementierung der Klasse GlobeOptionsDlg
//

#include "stdafx.h"
#include "GlobeOptionsDlg.h"


// GlobeOptionsDlg
//

GlobeOptionsDlg::GlobeOptionsDlg(CWnd* pParent)
	: CDialog(IDD_GLOBEOPTIONS, pParent)
{
}

void GlobeOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_TEXTURESIZE, m_wndTextureSize);
	DDX_Control(pDX, IDC_VIEWPORT, m_wndViewport);

	DDX_Check(pDX, IDC_HQMODEL, theApp.m_GlobeHQModel);
	DDX_Check(pDX, IDC_LIGHTING, theApp.m_GlobeLighting);
	DDX_Check(pDX, IDC_ATMOSPHERE, theApp.m_GlobeAtmosphere);
	DDX_Check(pDX, IDC_SHADOWS, theApp.m_GlobeShadows);
	DDX_Check(pDX, IDC_SPOTS, theApp.m_GlobeShowSpots);
	DDX_Check(pDX, IDC_AIRPORTNAMES, theApp.m_GlobeShowAirportNames);
	DDX_Check(pDX, IDC_GPSCOORDINATES, theApp.m_GlobeShowGPS);
	DDX_Check(pDX, IDC_VIEWPORT, theApp.m_GlobeShowViewport);
	DDX_Check(pDX, IDC_CROSSHAIRS, theApp.m_GlobeShowCrosshairs);

	if (pDX->m_bSaveAndValidate)
		theApp.m_nTextureSize = m_wndTextureSize.GetCurSel();
}


BEGIN_MESSAGE_MAP(GlobeOptionsDlg, CDialog)
	ON_BN_CLICKED(IDC_VIEWPORT, OnViewport)
END_MESSAGE_MAP()

BOOL GlobeOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_GLOBEOPTIONS);
	SetIcon(hIcon, TRUE);		// Gro�es Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Texturgr��e
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_AUTOMATIC));
	m_wndTextureSize.AddString(tmpStr);
	m_wndTextureSize.AddString(_T("1024�1024"));
	m_wndTextureSize.AddString(_T("2048�2048"));
	m_wndTextureSize.AddString(_T("4096�4096"));
	m_wndTextureSize.AddString(_T("8192�4096"));
	m_wndTextureSize.SetCurSel(theApp.m_nTextureSize);

	// Fadenkreuz
	OnViewport();

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void GlobeOptionsDlg::OnViewport()
{
	GetDlgItem(IDC_CROSSHAIRS)->EnableWindow(m_wndViewport.GetCheck());
}