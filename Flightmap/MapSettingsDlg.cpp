
// MapSettingsDlg.cpp: Implementierung der Klasse MapSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "MapSettingsDlg.h"


// MapSettingsDlg
//

const ResolutionPreset ResolutionPresets[] =
{
	{ 320, 480, L"iPhone 2G, 3G, 3GS", 3 },
	{ 400, 300, L"QSVGA", 4 },
	{ 640, 480, L"VGA", 4 },
	{ 640, 640, L"Instagram", 2 },
	{ 640, 960, L"iPhone 4, 4S", 3 },
	{ 640, 1136, L"iPhone 5, 5S, SE", 3 },
	{ 720, 480, L"NTSC", 5 },
	{ 720, 576, L"PAL", 5 },
	{ 750, 1334, L"iPhone 6 – 7S", 3 },
	{ 800, 600, L"SVGA", 4 },
	{ 854, 480, L"WVGA", 4 },
	{ 1024, 768, L"XGA", 4 },
	{ 1080, 1080, L"Instagram", 2 },
	{ 1080, 1920, L"iPhone 6+ – 7S+", 3 },
	{ 1280, 720, L"HDTV 720", 5 },
	{ 1280, 1024, L"SXGA", 4 },
	{ 1400, 1050, L"SXGA+", 4 },
	{ 1600, 1200, L"UXGA", 4 },
	{ 1680, 1050, L"WSXGA+", 4 },
	{ 1920, 1080, L"HDTV 1080", 5 },
	{ 1920, 1200, L"WUXGA", 4 },
	{ 2048, 1365, L"3 Megapixel 3:2", 0 },
	{ 2048, 1536, L"3 Megapixel 4:3", 0 },
	{ 3072, 2048, L"6 Megapixel 3:2", 0 },
	{ 3072, 2304, L"6 Megapixel 4:3", 0 },
	{ 4096, 2730, L"12 Megapixel 3:2", 0 },
	{ 4096, 3072, L"12 Megapixel 4:3", 0 },
	{ 8192, 4096, L"", 1 }
};

UINT MapSettingsDlg::m_LastTab = 0;
CIcons MapSettingsDlg::m_ResolutionPresetIcons;

MapSettingsDlg::MapSettingsDlg(CWnd* pParentWnd)
	: FMTabbedDialog(IDS_MAPSETTINGS, pParentWnd, &m_LastTab)
{
}

void MapSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMTabbedDialog::DoDataExchange(pDX);

	// Tab 0
	DDX_Control(pDX, IDC_RESOLUTIONPRESETS, m_wndResolutionList);
	DDX_Control(pDX, IDC_WIDTH, m_wndEditWidth);
	DDX_Control(pDX, IDC_HEIGHT, m_wndEditHeight);

	// Tab 1
	DDX_Control(pDX, IDC_BACKGROUND, m_wndBackground);
	DDX_Control(pDX, IDC_BACKGROUND_PREVIEW, m_wndBackgroundPreview);
	DDX_Control(pDX, IDC_BACKGROUNDCOLOR_INDICATOR, m_wndColorIndicatorBackground);
	DDX_Radio(pDX, IDC_BACKGROUNDCENTER_ATLANTIC, theApp.m_MapSettings.CenterPacific);
	DDX_Check(pDX, IDC_WIDEBORDER, theApp.m_MapSettings.WideBorder);
	DDX_Radio(pDX, IDC_SCALE_ALLOWSMALLER, theApp.m_MapSettings.ForegroundScale);

	// Tab 2
	DDX_Check(pDX, IDC_METROPOLITAN_MERGE, theApp.m_MapMergeMetro);
	DDX_Control(pDX, IDC_METROPOLITAN_PREVIEW, m_wndMetropolitanPreview);
	DDX_Check(pDX, IDC_SHOWLOCATIONS, theApp.m_MapSettings.ShowLocations);
	DDX_Control(pDX, IDC_LOCATIONS_INNER_INDICATOR, m_wndColorIndicatorLocationsInner);
	DDX_Control(pDX, IDC_LOCATIONS_OUTER_INDICATOR, m_wndColorIndicatorLocationsOuter);
	DDX_Check(pDX, IDC_SHOWIATACODES, theApp.m_MapSettings.ShowIATACodes);
	DDX_Control(pDX, IDC_IATACODES_INNER_INDICATOR, m_wndColorIndicatorIATACodesInner);
	DDX_Control(pDX, IDC_IATACODES_OUTER_INDICATOR, m_wndColorIndicatorIATACodesOuter);

	// Tab 3
	DDX_Check(pDX, IDC_SHOWROUTES, theApp.m_MapSettings.ShowRoutes);
	DDX_Control(pDX, IDC_ROUTECOLOR_INDICATOR, m_wndColorIndicatorRoute);
	DDX_Check(pDX, IDC_USECOLORS, theApp.m_MapSettings.UseColors);
	DDX_Check(pDX, IDC_STRAIGHTLINES, theApp.m_MapSettings.StraightLines);
	DDX_Check(pDX, IDC_ARROWS, theApp.m_MapSettings.Arrows);
	DDX_Check(pDX, IDC_USECOUNT_WIDTH, theApp.m_MapSettings.UseCountWidth);
	DDX_Check(pDX, IDC_USECOUNT_OPACITY, theApp.m_MapSettings.UseCountOpacity);
	DDX_Check(pDX, IDC_NOTES_DISTANCE, theApp.m_MapSettings.NoteDistance);
	DDX_Check(pDX, IDC_NOTES_FLIGHTTIME, theApp.m_MapSettings.NoteFlightTime);
	DDX_Check(pDX, IDC_NOTES_FLIGHTCOUNT, theApp.m_MapSettings.NoteFlightCount);
	DDX_Check(pDX, IDC_NOTES_CARRIER, theApp.m_MapSettings.NoteCarrier);
	DDX_Check(pDX, IDC_NOTES_EQUIPMENT, theApp.m_MapSettings.NoteEquipment);
	DDX_Control(pDX, IDC_NOTECOLOR_INNER_INDICATOR, m_wndColorIndicatorNoteInner);
	DDX_Control(pDX, IDC_NOTECOLOR_OUTER_INDICATOR, m_wndColorIndicatorNoteOuter);
	DDX_Check(pDX, IDC_NOTES_SMALLFONT, theApp.m_MapSettings.NoteSmallFont);

	if (pDX->m_bSaveAndValidate)
	{
		// Tab 0
		if (((CButton*)GetDlgItem(IDC_USERDEFINEDRESOLUTION))->GetCheck())
		{
			CString tmpStr;
			m_wndEditWidth.GetWindowText(tmpStr);
			theApp.m_MapSettings.Width = _wtoi(tmpStr);

			if (theApp.m_MapSettings.Width<128)
				theApp.m_MapSettings.Width = 128;

			if (theApp.m_MapSettings.Width>8192)
				theApp.m_MapSettings.Width = 8192;

			m_wndEditHeight.GetWindowText(tmpStr);
			theApp.m_MapSettings.Height = _wtoi(tmpStr);

			if (theApp.m_MapSettings.Height<128)
				theApp.m_MapSettings.Height = 128;

			if (theApp.m_MapSettings.Height>4096)
				theApp.m_MapSettings.Height = 4096;
		}
		else
		{
			INT Index = m_wndResolutionList.GetNextItem(-1, LVIS_SELECTED);
			if (Index!=-1)
			{
				theApp.m_MapSettings.Width = ResolutionPresets[Index].Width;
				theApp.m_MapSettings.Height = ResolutionPresets[Index].Height;
			}
		}

		// Tab 1
		theApp.m_MapSettings.Background = m_wndBackground.GetCurSel();
		theApp.m_MapSettings.BackgroundColor = m_clrBackground;

		// Tab 2
		theApp.m_MapSettings.LocationsInnerColor = m_clrLocationsInner;
		theApp.m_MapSettings.LocationsOuterColor = m_clrLocationsOuter;
		theApp.m_MapSettings.IATACodesInnerColor = m_clrIATACodesInner;
		theApp.m_MapSettings.IATACodesOuterColor = m_clrIATACodesOuter;

		// Tab 3
		theApp.m_MapSettings.RouteColor = m_clrRoute;
		theApp.m_MapSettings.NoteInnerColor = m_clrNoteInner;
		theApp.m_MapSettings.NoteOuterColor = m_clrNoteOuter;
	}
}

BOOL MapSettingsDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!FMTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_MAPSETTINGS_RESOLUTION, pszTabArea);
	AddTab(IDD_MAPSETTINGS_BACKGROUND, pszTabArea);
	AddTab(IDD_MAPSETTINGS_LOCATIONS, pszTabArea);
	AddTab(IDD_MAPSETTINGS_ROUTES, pszTabArea);

	return TRUE;
}

BOOL MapSettingsDlg::InitDialog()
{
	BOOL Result = FMTabbedDialog::InitDialog();

	// Tab 0
	CString tmpStr;

	m_wndEditWidth.SetValidChars(_T("0123456789"));
	m_wndEditWidth.SetLimitText(4);
	tmpStr.Format(_T("%u"), theApp.m_MapSettings.Width);
	m_wndEditWidth.SetWindowText(tmpStr);

	m_wndEditHeight.SetValidChars(_T("0123456789"));
	m_wndEditHeight.SetLimitText(4);
	tmpStr.Format(_T("%u"), theApp.m_MapSettings.Height);
	m_wndEditHeight.SetWindowText(tmpStr);

	m_wndResolutionList.SetFont(&theApp.m_DefaultFont);

	m_ResolutionPresetIcons.Load(IDB_RESOLUTIONPRESETICONS_16);
	m_ResolutionPresetIconsImageList.Attach(m_ResolutionPresetIcons.ExtractImageList());
	m_wndResolutionList.SetImageList(&m_ResolutionPresetIconsImageList, LVSIL_SMALL);

	m_wndResolutionList.AddColumn(0, CString((LPCSTR)IDS_COLUMN_RESOLUTION));
	m_wndResolutionList.AddColumn(1, CString((LPCSTR)IDS_COLUMN_APPLICATION));

	BOOL bUserDefined = TRUE;
	static UINT puColumns[1] = { 1 };

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS | LVIF_STATE;
	lvi.cColumns = 1;
	lvi.puColumns = puColumns;
	lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

	for (UINT a=0; a<sizeof(ResolutionPresets)/sizeof(ResolutionPreset); a++)
	{
		tmpStr.Format(_T("%u×%u"), ResolutionPresets[a].Width, ResolutionPresets[a].Height);

		lvi.iItem = a;
		lvi.pszText = tmpStr.GetBuffer();
		lvi.iImage = ResolutionPresets[a].iImage;

		if ((ResolutionPresets[a].Width==theApp.m_MapSettings.Width) && (ResolutionPresets[a].Height==theApp.m_MapSettings.Height))
		{
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			bUserDefined = FALSE;
		}
		else
		{
			lvi.state = 0;
		}

		m_wndResolutionList.SetItemText(m_wndResolutionList.InsertItem(&lvi), 1, ResolutionPresets[a].Hint);
	}

	m_wndResolutionList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_wndResolutionList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	((CButton*)GetDlgItem(IDC_USERDEFINEDRESOLUTION))->SetCheck(bUserDefined);
	OnUserDefinedResolution();

	// Tab 1
	for (UINT a=0; a<4; a++)
		m_wndBackground.AddString(CString((LPCSTR)(IDS_BACKGROUND_DEFAULT+a)));

	m_wndBackground.SetCurSel(theApp.m_MapSettings.Background);
	m_wndColorIndicatorBackground.SetColor(m_clrBackground=theApp.m_MapSettings.BackgroundColor);

	OnBackground();

	// Tab 2
	OnChangeMergeMetropolitan();

	OnShowLocations();
	m_wndColorIndicatorLocationsInner.SetColor(m_clrLocationsInner=theApp.m_MapSettings.LocationsInnerColor);
	m_wndColorIndicatorLocationsOuter.SetColor(m_clrLocationsOuter=theApp.m_MapSettings.LocationsOuterColor);

	OnShowIATACodes();
	m_wndColorIndicatorIATACodesInner.SetColor(m_clrIATACodesInner=theApp.m_MapSettings.IATACodesInnerColor);
	m_wndColorIndicatorIATACodesOuter.SetColor(m_clrIATACodesOuter=theApp.m_MapSettings.IATACodesOuterColor);

	// Tab 3
	OnShowRoutes();

	m_wndColorIndicatorRoute.SetColor(m_clrRoute=theApp.m_MapSettings.RouteColor);
	m_wndColorIndicatorNoteInner.SetColor(m_clrNoteInner=theApp.m_MapSettings.NoteInnerColor);
	m_wndColorIndicatorNoteOuter.SetColor(m_clrNoteOuter=theApp.m_MapSettings.NoteOuterColor);

	return Result;
}


BEGIN_MESSAGE_MAP(MapSettingsDlg, FMTabbedDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_RESOLUTIONPRESETS, OnDoubleClick)
	ON_BN_CLICKED(IDC_USERDEFINEDRESOLUTION, OnUserDefinedResolution)

	ON_CBN_SELCHANGE(IDC_BACKGROUND, OnBackground)
	ON_BN_CLICKED(IDC_BACKGROUNDCOLOR_CHOOSE, OnChooseColorBackground)

	ON_BN_CLICKED(IDC_METROPOLITAN_MERGE, OnChangeMergeMetropolitan)
	ON_BN_CLICKED(IDC_SHOWLOCATIONS, OnShowLocations)
	ON_BN_CLICKED(IDC_LOCATIONS_INNER_CHOOSE, OnChooseColorLocationsInner)
	ON_BN_CLICKED(IDC_LOCATIONS_OUTER_CHOOSE, OnChooseColorLocationsOuter)
	ON_BN_CLICKED(IDC_SHOWIATACODES, OnShowIATACodes)
	ON_BN_CLICKED(IDC_IATACODES_INNER_CHOOSE, OnChooseColorIATACodesInner)
	ON_BN_CLICKED(IDC_IATACODES_OUTER_CHOOSE, OnChooseColorIATACodesOuter)

	ON_BN_CLICKED(IDC_SHOWROUTES, OnShowRoutes)
	ON_BN_CLICKED(IDC_ROUTECOLOR_CHOOSE, OnChooseColorRoute)
	ON_BN_CLICKED(IDC_NOTECOLOR_INNER_CHOOSE, OnChooseColorNoteInner)
	ON_BN_CLICKED(IDC_NOTECOLOR_OUTER_CHOOSE, OnChooseColorNoteOuter)
	ON_BN_CLICKED(IDC_NOTES_DISTANCE, OnShowRoutes)
	ON_BN_CLICKED(IDC_NOTES_FLIGHTTIME, OnShowRoutes)
	ON_BN_CLICKED(IDC_NOTES_FLIGHTCOUNT, OnShowRoutes)
	ON_BN_CLICKED(IDC_NOTES_CARRIER, OnShowRoutes)
	ON_BN_CLICKED(IDC_NOTES_EQUIPMENT, OnShowRoutes)
END_MESSAGE_MAP()

void MapSettingsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, IDOK);
}

void MapSettingsDlg::OnUserDefinedResolution()
{
	BOOL bUserDefined = ((CButton*)GetDlgItem(IDC_USERDEFINEDRESOLUTION))->GetCheck();

	if (bUserDefined && (GetFocus()==&m_wndResolutionList))
		m_wndEditWidth.SetFocus();

	m_wndResolutionList.EnableWindow(!bUserDefined);
	m_wndEditWidth.EnableWindow(bUserDefined);
	m_wndEditHeight.EnableWindow(bUserDefined);
}


void MapSettingsDlg::OnBackground()
{
	INT Background = m_wndBackground.GetCurSel();
	ASSERT(Background>=0);

	BOOL bColor = (Background>=3);

	GetDlgItem(IDC_BACKGROUNDCOLOR_CHOOSE)->EnableWindow(bColor);
	m_wndColorIndicatorBackground.EnableWindow(bColor);

	if (bColor)
	{
		m_wndBackgroundPreview.SetColor(m_clrBackground, IDS_BACKGROUND_COLOR);
	}
	else
	{
		static const UINT ResID[3] = { IDB_BLUEMARBLE_512, IDB_NIGHT_512, IDB_ABSTRACT_512 };
		m_wndBackgroundPreview.SetPicture(ResID[Background], IDS_BACKGROUND_DEFAULT+Background, TRUE);
	}
}

void MapSettingsDlg::OnChooseColorBackground()
{
	ChooseColor(&m_clrBackground, FALSE, &m_wndColorIndicatorBackground);

	m_wndBackgroundPreview.SetColor(m_clrBackground, IDS_BACKGROUND_COLOR);
}


void MapSettingsDlg::OnChangeMergeMetropolitan()
{
	m_wndMetropolitanPreview.SetPicture(((CButton*)GetDlgItem(IDC_METROPOLITAN_MERGE))->GetCheck() ? IDB_METROPOLITAN_MERGED : IDB_METROPOLITAN_DISTINCT);
}

void MapSettingsDlg::OnShowLocations()
{
	BOOL bEnable = ((CButton*)GetDlgItem(IDC_SHOWLOCATIONS))->GetCheck();

	GetDlgItem(IDC_LOCATIONS_INNER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorLocationsInner.EnableWindow(bEnable);

	GetDlgItem(IDC_LOCATIONS_OUTER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorLocationsOuter.EnableWindow(bEnable);
}

void MapSettingsDlg::OnChooseColorLocationsInner()
{
	ChooseColor(&m_clrLocationsInner, FALSE, &m_wndColorIndicatorLocationsInner);
}

void MapSettingsDlg::OnChooseColorLocationsOuter()
{
	ChooseColor(&m_clrLocationsOuter, TRUE, &m_wndColorIndicatorLocationsOuter);
}

void MapSettingsDlg::OnShowIATACodes()
{
	BOOL bEnable = ((CButton*)GetDlgItem(IDC_SHOWIATACODES))->GetCheck();

	GetDlgItem(IDC_IATACODES_INNER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorIATACodesInner.EnableWindow(bEnable);

	GetDlgItem(IDC_IATACODES_OUTER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorIATACodesOuter.EnableWindow(bEnable);
}

void MapSettingsDlg::OnChooseColorIATACodesInner()
{
	ChooseColor(&m_clrIATACodesInner, FALSE, &m_wndColorIndicatorIATACodesInner);
}

void MapSettingsDlg::OnChooseColorIATACodesOuter()
{
	ChooseColor(&m_clrIATACodesOuter, TRUE, &m_wndColorIndicatorIATACodesOuter);
}


void MapSettingsDlg::OnShowRoutes()
{
	BOOL bEnable = ((CButton*)GetDlgItem(IDC_SHOWROUTES))->GetCheck();

	GetDlgItem(IDC_ROUTECOLOR_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorRoute.EnableWindow(bEnable);

	GetDlgItem(IDC_USECOLORS)->EnableWindow(bEnable);
	GetDlgItem(IDC_STRAIGHTLINES)->EnableWindow(bEnable);
	GetDlgItem(IDC_ARROWS)->EnableWindow(bEnable);
	GetDlgItem(IDC_USECOUNT_WIDTH)->EnableWindow(bEnable);
	GetDlgItem(IDC_USECOUNT_OPACITY)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES_DISTANCE)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES_FLIGHTTIME)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES_FLIGHTCOUNT)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES_CARRIER)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES_EQUIPMENT)->EnableWindow(bEnable);

	bEnable &= ((CButton*)GetDlgItem(IDC_NOTES_DISTANCE))->GetCheck() ||
		((CButton*)GetDlgItem(IDC_NOTES_FLIGHTTIME))->GetCheck() ||
		((CButton*)GetDlgItem(IDC_NOTES_FLIGHTCOUNT))->GetCheck() ||
		((CButton*)GetDlgItem(IDC_NOTES_CARRIER))->GetCheck() ||
		((CButton*)GetDlgItem(IDC_NOTES_EQUIPMENT))->GetCheck();

	GetDlgItem(IDC_NOTECOLOR_INNER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorNoteInner.EnableWindow(bEnable);

	GetDlgItem(IDC_NOTECOLOR_OUTER_CHOOSE)->EnableWindow(bEnable);
	m_wndColorIndicatorNoteOuter.EnableWindow(bEnable);

	GetDlgItem(IDC_NOTES_SMALLFONT)->EnableWindow(bEnable);
}

void MapSettingsDlg::OnChooseColorRoute()
{
	ChooseColor(&m_clrRoute, FALSE, &m_wndColorIndicatorRoute);
}

void MapSettingsDlg::OnChooseColorNoteInner()
{
	ChooseColor(&m_clrNoteInner, FALSE, &m_wndColorIndicatorNoteInner);
}

void MapSettingsDlg::OnChooseColorNoteOuter()
{
	ChooseColor(&m_clrNoteOuter, TRUE, &m_wndColorIndicatorNoteOuter);
}
