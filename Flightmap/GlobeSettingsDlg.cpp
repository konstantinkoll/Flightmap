
// GlobeSettingsDlg.cpp: Implementierung der Klasse GlobeSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "GlobeSettingsDlg.h"


// GlobeSettingsDlg
//

GlobeSettingsDlg::GlobeSettingsDlg(CWnd* pParentWnd)
	: FMDialog(IDD_GLOBESETTINGS, pParentWnd)
{
}

void GlobeSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_METROPOLITAN_MERGE, theApp.m_GlobeMergeMetro);
	DDX_Control(pDX, IDC_METROPOLITAN_PREVIEW, m_wndMetropolitanPreview);
}

BOOL GlobeSettingsDlg::InitDialog()
{
	OnChangeMergeMetropolitan();

	return TRUE;
}


BEGIN_MESSAGE_MAP(GlobeSettingsDlg, FMDialog)
	ON_BN_CLICKED(IDC_METROPOLITAN_MERGE, OnChangeMergeMetropolitan)
END_MESSAGE_MAP()

void GlobeSettingsDlg::OnChangeMergeMetropolitan()
{
	m_wndMetropolitanPreview.SetPicture(((CButton*)GetDlgItem(IDC_METROPOLITAN_MERGE))->GetCheck() ? IDB_METROPOLITAN_MERGED : IDB_METROPOLITAN_DISTINCT);
}
