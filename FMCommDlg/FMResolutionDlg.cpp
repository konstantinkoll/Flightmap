
// FMResolutionDlg.cpp: Implementierung der Klasse FMResolutionDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMResolutionDlg
//

struct FixedResolution
{
	UINT Width;
	UINT Height;
	WCHAR Hint[17];
};

const FixedResolution FixedResolutions[] =
{
	{ 400, 300, L"QSVGA" },
	{ 640, 480, L"VGA" },
	{ 720, 480, L"NTSC" },
	{ 720, 576, L"PAL" },
	{ 800, 600, L"SVGA" },
	{ 854, 480, L"WVGA" },
	{ 1024, 768, L"XGA" },
	{ 1280, 720, L"HDTV 720" },
	{ 1280, 1024, L"SXGA" },
	{ 1400, 1050, L"SXGA+" },
	{ 1600, 1200, L"UXGA" },
	{ 1680, 1050, L"WSXGA+" },
	{ 1920, 1080, L"HDTV 1080" },
	{ 1920, 1200, L"WUXGA" },
	{ 2048, 1365, L"3 Megapixel 3:2" },
	{ 2048, 1536, L"3 Megapixel 4:3" },
	{ 3072, 2048, L"6 Megapixel 3:2" },
	{ 3072, 2304, L"6 Megapixel 4:3" },
	{ 4096, 2730, L"12 Megapixel 3:2" },
	{ 4096, 3072, L"12 Megapixel 4:3" },
	{ 8192, 4096, L"" }
};

FMResolutionDlg::FMResolutionDlg(UINT* pWidth, UINT* pHeight, CWnd* pParent)
	: CDialog(IDD_RESOLUTION, pParent)
{
	ASSERT(pWidth);
	ASSERT(pHeight);

	p_Width = pWidth;
	p_Height = pHeight;
}

void FMResolutionDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_RESLIST, m_ResolutionList);
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
		}
		else
		{
			INT idx = m_ResolutionList.GetNextItem(-1, LVIS_SELECTED);
			if (idx!=-1)
			{
				*p_Width = FixedResolutions[idx].Width;
				*p_Height = FixedResolutions[idx].Height;
			}
		}
}


BEGIN_MESSAGE_MAP(FMResolutionDlg, CDialog)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_RESLIST, OnDoubleClick)
	ON_BN_CLICKED(IDC_USERDEFINEDRES, OnUserDefinedRes)
END_MESSAGE_MAP()

BOOL FMResolutionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	hIconS = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_RESOLUTION), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SetIcon(hIconS, FALSE);
	hIconL = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_RESOLUTION), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	SetIcon(hIconL, TRUE);

	// Eingabe
	CString tmpStr;

	m_wndWidth.SetValidChars(_T("0123456789"));
	m_wndWidth.SetLimitText(4);
	tmpStr.Format(_T("%d"), *p_Width);
	m_wndWidth.SetWindowText(tmpStr);

	m_wndHeight.SetValidChars(_T("0123456789"));
	m_wndHeight.SetLimitText(4);
	tmpStr.Format(_T("%d"), *p_Height);
	m_wndHeight.SetWindowText(tmpStr);

	// Liste
	BOOL UserDefined = TRUE;

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = 100;
	m_ResolutionList.SetTileViewInfo(&tvi);
	m_ResolutionList.AddColumn(0, _T(""));
	m_ResolutionList.AddColumn(1, _T(""));

	UINT puColumns[1];
	puColumns[0] = 1;

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_COLUMNS | LVIF_STATE;
	lvi.puColumns = puColumns;

	for (UINT a=0; a<sizeof(FixedResolutions)/sizeof(FixedResolution); a++)
	{
		CString tmpStr;
		tmpStr.Format(_T("%d×%d"), FixedResolutions[a].Width, FixedResolutions[a].Height);

		lvi.iItem = a;
		lvi.cColumns = 1;
		lvi.pszText = tmpStr.GetBuffer();
		if ((FixedResolutions[a].Width==*p_Width) && (FixedResolutions[a].Height==*p_Height))
		{
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			UserDefined = FALSE;
		}
		else
		{
			lvi.state = 0;
		}
		lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		INT idx = m_ResolutionList.InsertItem(&lvi);

		m_ResolutionList.SetItemText(idx, 1, FixedResolutions[a].Hint);
	}

	m_ResolutionList.SetView(LV_VIEW_TILE);

	// Checkbox
	((CButton*)GetDlgItem(IDC_USERDEFINEDRES))->SetCheck(UserDefined);
	OnUserDefinedRes();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMResolutionDlg::OnDestroy()
{
	if (hIconL)
		DestroyIcon(hIconL);
	if (hIconS)
		DestroyIcon(hIconS);

	CDialog::OnDestroy();
}

void FMResolutionDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	EndDialog(IDOK);
}

void FMResolutionDlg::OnUserDefinedRes()
{
	BOOL UserDefined = ((CButton*)GetDlgItem(IDC_USERDEFINEDRES))->GetCheck();

	m_ResolutionList.EnableWindow(!UserDefined);
	m_wndWidth.EnableWindow(UserDefined);
	m_wndHeight.EnableWindow(UserDefined);
}
