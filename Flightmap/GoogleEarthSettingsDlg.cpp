
// GoogleEarthSettingsDlg.cpp: Implementierung der Klasse GoogleEarthSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "GoogleEarthSettingsDlg.h"


// GoogleEarthSettingsDlg
//

GoogleEarthSettingsDlg::GoogleEarthSettingsDlg(CWnd* pParentWnd)
	: FMDialog(IDD_GOOGLEEARTHSETTINGS, pParentWnd)
{
}

void GoogleEarthSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_METROPOLITAN_MERGE, theApp.m_GoogleEarthMergeMetro);
	DDX_Control(pDX, IDC_METROPOLITAN_PREVIEW, m_wndMetropolitanPreview);
	DDX_Check(pDX, IDC_USECOUNT_WIDTH, theApp.m_GoogleEarthUseCount);
	DDX_Check(pDX, IDC_USECOLORS, theApp.m_GoogleEarthUseColors);
	DDX_Check(pDX, IDC_CLAMPHEIGHT, theApp.m_GoogleEarthClampHeight);
}

BOOL GoogleEarthSettingsDlg::InitDialog()
{
	OnChangeMergeMetropolitan();

	return TRUE;
}


BEGIN_MESSAGE_MAP(GoogleEarthSettingsDlg, FMDialog)
	ON_BN_CLICKED(IDC_METROPOLITAN_MERGE, OnChangeMergeMetropolitan)
END_MESSAGE_MAP()

void GoogleEarthSettingsDlg::OnChangeMergeMetropolitan()
{
	m_wndMetropolitanPreview.SetPicture(((CButton*)GetDlgItem(IDC_METROPOLITAN_MERGE))->GetCheck() ? IDB_METROPOLITAN_MERGED : IDB_METROPOLITAN_DISTINCT);
}
