
// FMColorDlg.cpp: Implementierung der Klasse FMColorDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMColorDlg
//

FMColorDlg::FMColorDlg(COLORREF* pColor, CWnd* pParentWnd, BOOL AllowReset)
	: FMDialog(IDD_CHOOSECOLOR, pParentWnd)
{
	ASSERT(pColor);

	p_Color = pColor;
	m_AllowReset = AllowReset;
}

void FMColorDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COLORPICKER, m_wndColorPicker);
	DDX_Control(pDX, IDC_COLORHISTORY, m_wndColorHistory);

	if (pDX->m_bSaveAndValidate)
		*p_Color = m_wndColorPicker.GetColor();
}

BOOL FMColorDlg::InitDialog()
{
	// Set color
	if (*p_Color!=(COLORREF)-1)
		m_wndColorPicker.SetColor(*p_Color);

	// Hide reset link
	GetDlgItem(IDC_RESETCOLOR)->ShowWindow(m_AllowReset ? SW_SHOW : SW_HIDE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(FMColorDlg, FMDialog)
	ON_NOTIFY(GRADATIONPYRAMID_DBLCLK, 2, OnDoubleClickPicker)
	ON_NOTIFY(COLORHISTORY_DBLCLK, IDC_COLORHISTORY, OnDoubleClickHistory)
	ON_NOTIFY(NM_CLICK, IDC_RESETCOLOR, OnResetColor)
END_MESSAGE_MAP()

void FMColorDlg::OnDoubleClickPicker(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*p_Color = m_wndColorPicker.GetColor();
	EndDialog(IDOK);

	*pResult = 0;
}

void FMColorDlg::OnDoubleClickHistory(NMHDR* pNMHDR, LRESULT* pResult)
{
	*p_Color = ((NM_COLORDATA*)pNMHDR)->clr;
	EndDialog(IDOK);

	*pResult = 0;
}

void FMColorDlg::OnResetColor(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ASSERT(m_AllowReset);

	*p_Color = 0xFFFFFFFF;
	EndDialog(IDOK);

	*pResult = 0;
}
