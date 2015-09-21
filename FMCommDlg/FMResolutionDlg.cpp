
// FMResolutionDlg.cpp: Implementierung der Klasse FMResolutionDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMResolutionDlg
//

struct FixedResolution
{
	UINT Width;
	UINT Height;
	WCHAR Hint[17];
	INT Image;
};

const FixedResolution FixedResolutions[] =
{
	{ 400, 300, L"QSVGA", 0 },
	{ 640, 480, L"VGA", 0 },
	{ 640, 640, L"Instagram", 4 },
	{ 720, 480, L"NTSC", 1 },
	{ 720, 576, L"PAL", 1 },
	{ 800, 600, L"SVGA", 0 },
	{ 854, 480, L"WVGA", 0 },
	{ 1024, 768, L"XGA", 0 },
	{ 1080, 1080, L"Instagram", 4 },
	{ 1280, 720, L"HDTV 720", 1 },
	{ 1280, 1024, L"SXGA", 0 },
	{ 1400, 1050, L"SXGA+", 0 },
	{ 1600, 1200, L"UXGA", 0 },
	{ 1680, 1050, L"WSXGA+", 0 },
	{ 1920, 1080, L"HDTV 1080", 1 },
	{ 1920, 1200, L"WUXGA", 0 },
	{ 2048, 1365, L"3 Megapixel 3:2", 2 },
	{ 2048, 1536, L"3 Megapixel 4:3", 2 },
	{ 3072, 2048, L"6 Megapixel 3:2", 2 },
	{ 3072, 2304, L"6 Megapixel 4:3", 2 },
	{ 4096, 2730, L"12 Megapixel 3:2", 2 },
	{ 4096, 3072, L"12 Megapixel 4:3", 2 },
	{ 8192, 4096, L"", 3 }
};

FMResolutionDlg::FMResolutionDlg(UINT* pWidth, UINT* pHeight, CWnd* pParentWnd)
	: CDialog(IDD_RESOLUTION, pParentWnd)
{
	ASSERT(pWidth);
	ASSERT(pHeight);

	p_Width = pWidth;
	p_Height = pHeight;
}

void FMResolutionDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_RESLIST, m_wndResolutionList);
	DDX_Control(pDX, IDC_WIDTH, m_wndWidth);
	DDX_Control(pDX, IDC_HEIGHT, m_wndHeight);

	if (pDX->m_bSaveAndValidate)
		if (((CButton*)GetDlgItem(IDC_USERDEFINEDRES))->GetCheck())
		{
			CString tmpStr;
			m_wndWidth.GetWindowText(tmpStr);
			*p_Width = _wtoi(tmpStr);

			m_wndHeight.GetWindowText(tmpStr);
			*p_Height = _wtoi(tmpStr);

			if (!FMIsLicensed())
			{
				*p_Width = min(*p_Width, 640);
				*p_Height = min(*p_Height, 640);
			}
		}
		else
		{
			INT Index = m_wndResolutionList.GetNextItem(-1, LVIS_SELECTED);
			if (Index!=-1)
			{
				*p_Width = FixedResolutions[Index].Width;
				*p_Height = FixedResolutions[Index].Height;
			}
		}
}


BEGIN_MESSAGE_MAP(FMResolutionDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_RESLIST, OnDoubleClick)
	ON_BN_CLICKED(IDC_USERDEFINEDRES, OnUserDefinedRes)
END_MESSAGE_MAP()

BOOL FMResolutionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Eingabe
	CString tmpStr;

	m_wndWidth.SetValidChars(_T("0123456789"));
	m_wndWidth.SetLimitText(4);
	tmpStr.Format(_T("%u"), *p_Width);
	m_wndWidth.SetWindowText(tmpStr);

	m_wndHeight.SetValidChars(_T("0123456789"));
	m_wndHeight.SetLimitText(4);
	tmpStr.Format(_T("%u"), *p_Height);
	m_wndHeight.SetWindowText(tmpStr);

	// Icons
	m_Icons.Create(IDB_RESOLUTIONICONS, 32, 32);
	m_wndResolutionList.SetImageList(&m_Icons, LVSIL_NORMAL);

	// Liste
	BOOL UserDefined = TRUE;

	m_wndResolutionList.AddColumn(0, _T(""));
	m_wndResolutionList.AddColumn(1, _T(""));

	static UINT puColumns[1] = { 1 };

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS | LVIF_STATE;
	lvi.cColumns = 1;
	lvi.puColumns = puColumns;
	lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

	UINT Count = FMIsLicensed() ? sizeof(FixedResolutions)/sizeof(FixedResolution) : 3;
	for (UINT a=0; a<Count; a++)
	{
		CString tmpStr;
		tmpStr.Format(_T("%u×%u"), FixedResolutions[a].Width, FixedResolutions[a].Height);

		lvi.iItem = a;
		lvi.pszText = tmpStr.GetBuffer();
		lvi.iImage = FixedResolutions[a].Image;
		if ((FixedResolutions[a].Width==*p_Width) && (FixedResolutions[a].Height==*p_Height))
		{
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			UserDefined = FALSE;
		}
		else
		{
			lvi.state = 0;
		}
		INT Index = m_wndResolutionList.InsertItem(&lvi);

		m_wndResolutionList.SetItemText(Index, 1, FixedResolutions[a].Hint);
	}

	m_wndResolutionList.SetView(LV_VIEW_TILE);
	m_wndResolutionList.SetItemsPerRow(4, 1);

	// Checkbox
	((CButton*)GetDlgItem(IDC_USERDEFINEDRES))->SetCheck(UserDefined);
	OnUserDefinedRes();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMResolutionDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, IDOK);
}

void FMResolutionDlg::OnUserDefinedRes()
{
	BOOL UserDefined = ((CButton*)GetDlgItem(IDC_USERDEFINEDRES))->GetCheck();

	m_wndResolutionList.EnableWindow(!UserDefined);
	m_wndWidth.EnableWindow(UserDefined);
	m_wndHeight.EnableWindow(UserDefined);

	if (UserDefined)
		m_wndWidth.SetFocus();
}
