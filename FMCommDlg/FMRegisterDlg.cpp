
// FMRegisterDlg.cpp: Implementierung der Klasse FMRegisterDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMRegisterDlg
//

FMRegisterDlg::FMRegisterDlg(CWnd* pParent)
	: FMDialog(IDD_REGISTER, FMDS_Blue, pParent)
{
}

void FMRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NRT_UNREGISTERED, m_wndUnregistered);
	DDX_Control(pDX, IDC_NRT_REGISTERED, m_wndRegistered);
}


BEGIN_MESSAGE_MAP(FMRegisterDlg, FMDialog)
END_MESSAGE_MAP()

BOOL FMRegisterDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	m_wndUnregistered.SetPicture(IDB_NRT_UNREGISTERED, _T("PNG"));
	GetDlgItem(IDC_CAPTION1)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	m_wndRegistered.SetPicture(IDB_NRT_REGISTERED, _T("PNG"));
	GetDlgItem(IDC_CAPTION2)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	//	m_pBackdrop = new CGdiPlusBitmapResource();
//	ENSURE(m_pBackdrop->Load(IDB_BACKDROP_BLUE, _T("PNG"), AfxGetResourceHandle()));

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
