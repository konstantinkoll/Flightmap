
// MapSettingsDlg.cpp: Implementierung der Klasse MapSettingsDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "MapSettingsDlg.h"


// CResolutionList
//

CIcons CResolutionList::m_ResolutionPresetIcons;

const ResolutionPreset CResolutionList::m_ResolutionPresets[RESOLUTIONPRESETCOUNT] =
{
	{ 400, 300, L"QSVGA", 4 },
	{ 480, 320, L"iPhone 2G, 3G, 3GS", 3 },
	{ 640, 480, L"VGA", 4 },
	{ 640, 640, L"Instagram", 2 },
	{ 720, 480, L"NTSC", 5 },
	{ 720, 576, L"PAL", 5 },
	{ 800, 600, L"SVGA", 4 },
	{ 854, 480, L"WVGA", 4 },
	{ 960, 640, L"iPhone 4, 4S", 3 },
	{ 1024, 768, L"XGA", 4 },
	{ 1080, 1080, L"Instagram", 2 },
	{ 1136, 640, L"iPhone 5, 5S, SE", 3 },
	{ 1280, 720, L"HDTV 720", 5 },
	{ 1280, 1024, L"SXGA", 4 },
	{ 1334, 750, L"iPhone 6 – 8, SE2", 3 },
	{ 1400, 1050, L"SXGA+", 4 },
	{ 1600, 1200, L"UXGA", 4 },
	{ 1680, 1050, L"WSXGA+", 4 },
	{ 1792, 838, L"iPhone XR, 11", 3 },
	{ 1920, 1080, L"HDTV 1080", 5 },
	{ 1920, 1080, L"iPhone 6+ – 8+", 3 },
	{ 1920, 1200, L"WUXGA", 4 },
	{ 2048, 1365, L"3 Megapixel 3:2", 0 },
	{ 2048, 1536, L"3 Megapixel 4:3", 0 },
	{ 2224, 1668, L"iPad Pro 10.5\"", 3 },
	{ 2388, 1668, L"iPad Pro 11\"", 3 },
	{ 2436, 1125, L"iPhone X, Xs, 11 Pro", 3 },
	{ 2688, 1242, L"iPhone Xs Max, 11 Pro Max", 3 },
	{ 2732, 2048, L"iPad Pro 12.9\"", 3 },
	{ 3072, 2048, L"6 Megapixel 3:2", 0 },
	{ 3072, 2304, L"6 Megapixel 4:3", 0 },
	{ 4096, 2730, L"12 Megapixel 3:2", 0 },
	{ 4096, 3072, L"12 Megapixel 4:3", 0 },
	{ 8192, 4096, L"", 1 }
};

CResolutionList::CResolutionList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM)
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = theApp.LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CResolutionList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CResolutionList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Item
	SetItemHeight(m_ResolutionPresetIcons.Load(IDB_RESOLUTIONPRESETICONS_16), 1, ITEMCELLPADDINGY);
}


// Header

void CResolutionList::UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const
{
	HeaderItem.mask = HDI_WIDTH;

	if ((HeaderItem.cxy=m_ColumnWidth[Attr])<ITEMVIEWMINWIDTH)
		HeaderItem.cxy = ITEMVIEWMINWIDTH;
}


// Layouts

void CResolutionList::AdjustLayout()
{
	// Header
	m_ColumnWidth[0] = m_IconSize+ITEMCELLPADDINGX+theApp.m_DefaultFont.GetTextExtent(_T("0000×0000")).cx+ITEMCELLSPACER;
	m_ColumnWidth[1] = 0;

	SetFixedColumnWidths(m_ColumnOrder, m_ColumnWidth);

	UpdateHeader();

	// Item layout
	AdjustLayoutList();
}


// Item data

BOOL CResolutionList::SetResolutions(UINT Width, UINT Height)
{
	// Header
	AddHeaderColumn(FALSE, IDS_COLUMN_RESOLUTION);
	AddHeaderColumn(FALSE, IDS_COLUMN_APPLICATION);

	// Items
	SetItemCount(RESOLUTIONPRESETCOUNT, TRUE);
	ValidateAllItems();

	AdjustLayout();

	// Find resolution
	for (UINT a=0; a<RESOLUTIONPRESETCOUNT; a++)
		if ((m_ResolutionPresets[a].Width==Width) && (m_ResolutionPresets[a].Height==Height))
		{
			SetFocusItem(a);

			return TRUE;
		}

	return FALSE;
}


// Item selection

BOOL CResolutionList::GetSelectedResolution(UINT& Width, UINT& Height) const
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		Width = m_ResolutionPresets[Index].Width;
		Height = m_ResolutionPresets[Index].Height;

		return TRUE;
	}

	return FALSE;
}


// Drawing

void CResolutionList::DrawItemCell(CDC& dc, CRect& rectCell, INT Index, UINT Attr, BOOL /*Themed*/)
{
	if (Attr==0)
	{
		// Icon
		m_ResolutionPresetIcons.Draw(dc, rectCell.left, rectCell.top+(rectCell.Height()-m_IconSize)/2, m_ResolutionPresets[Index].IconID, FALSE, !IsWindowEnabled());

		rectCell.left += m_IconSize+ITEMCELLPADDINGX;
	}

	// Column
	CString strCell;

	switch (Attr)
	{
	case 0:
		strCell.Format(_T("%u×%u"), m_ResolutionPresets[Index].Width, m_ResolutionPresets[Index].Height);
		break;

	case 1:
		strCell = m_ResolutionPresets[Index].Hint;
		break;
	}

	dc.DrawText(strCell, rectCell, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
}

void CResolutionList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	ASSERT(rectItem);

	DrawListItem(dc, rectItem, Index, Themed, m_ColumnOrder, m_ColumnWidth);
}


// MapSettingsDlg
//

UINT MapSettingsDlg::m_LastTab = 0;

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
			GetSelectedResolution(theApp.m_MapSettings.Width, theApp.m_MapSettings.Height);
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

	((CButton*)GetDlgItem(IDC_USERDEFINEDRESOLUTION))->SetCheck(!m_wndResolutionList.SetResolutions(theApp.m_MapSettings.Width, theApp.m_MapSettings.Height));
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

	return FMTabbedDialog::InitDialog();
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
	const INT Background = m_wndBackground.GetCurSel();
	ASSERT(Background>=0);

	const BOOL bColor = (Background>=3);

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
	const BOOL bEnable = ((CButton*)GetDlgItem(IDC_SHOWLOCATIONS))->GetCheck();

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
	const BOOL bEnable = ((CButton*)GetDlgItem(IDC_SHOWIATACODES))->GetCheck();

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
