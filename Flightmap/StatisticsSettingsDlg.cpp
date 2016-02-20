
// StatisticsSettingsDlg.cpp: Implementierung der Klasse StatisticsSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "StatisticsSettingsDlg.h"


// StatisticsSettingsDlg
//

StatisticsSettingsDlg::StatisticsSettingsDlg(CWnd* pParentWnd)
	: FMDialog(IDD_STATISTICSSETTINGS, pParentWnd)
{
}

void StatisticsSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_METROPOLITAN_MERGE, theApp.m_StatisticsMergeMetro);
	DDX_Control(pDX, IDC_METROPOLITAN_PREVIEW, m_wndMetropolitanPreview);
	DDX_Check(pDX, IDC_MERGEDIRECTIONS, theApp.m_StatisticsMergeDirections);
	DDX_Check(pDX, IDC_MERGEAWARDS, theApp.m_StatisticsMergeAwards);
	DDX_Check(pDX, IDC_MERGECLASSES, theApp.m_StatisticsMergeClasses);
}

BOOL StatisticsSettingsDlg::InitDialog()
{
	OnChangeMergeMetropolitan();

	return TRUE;
}


BEGIN_MESSAGE_MAP(StatisticsSettingsDlg, FMDialog)
	ON_BN_CLICKED(IDC_METROPOLITAN_MERGE, OnChangeMergeMetropolitan)
END_MESSAGE_MAP()

void StatisticsSettingsDlg::OnChangeMergeMetropolitan()
{
	m_wndMetropolitanPreview.SetPicture(((CButton*)GetDlgItem(IDC_METROPOLITAN_MERGE))->GetCheck() ? IDB_METROPOLITAN_MERGED : IDB_METROPOLITAN_DISTINCT);
}
