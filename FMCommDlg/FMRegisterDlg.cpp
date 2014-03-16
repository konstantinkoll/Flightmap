
// FMRegisterDlg.cpp: Implementierung der Klasse FMRegisterDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMRegisterDlg
//

FMRegisterDlg::FMRegisterDlg(CWnd* pParentWnd)
	: FMDialog(IDD_REGISTER, pParentWnd)
{
}

void FMRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NRT_UNREGISTERED, m_wndUnregistered);
	DDX_Control(pDX, IDC_NRT_REGISTERED, m_wndRegistered);
}

INT_PTR FMRegisterDlg::DoModal()
{
	CDialogTemplate dlt;
	if (!dlt.Load(MAKEINTRESOURCE(m_nIDTemplate)))
		return -1;

	dlt.SetFont(p_App->GetDefaultFontFace(), 9);

	LPSTR pdata = (LPSTR)GlobalLock(dlt.m_hTemplate);
	m_lpszTemplateName = NULL;
	InitModalIndirect(pdata);

	INT_PTR Res = FMDialog::DoModal();

	GlobalUnlock(dlt.m_hTemplate);
	return Res;
}


BEGIN_MESSAGE_MAP(FMRegisterDlg, FMDialog)
	ON_BN_CLICKED(IDC_PURCHASE, OnPurchase)
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

BOOL FMRegisterDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	m_fntWingdings.CreateFont(-36, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("Wingdings"));
	GetDlgItem(IDC_REGARROW)->SendMessage(WM_SETFONT, (WPARAM)(HFONT)m_fntWingdings);

	m_wndUnregistered.SetPicture(IDB_NRT_UNREGISTERED, _T("PNG"));
	GetDlgItem(IDC_CAPTION1)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	m_wndRegistered.SetPicture(IDB_NRT_REGISTERED, _T("PNG"));
	GetDlgItem(IDC_CAPTION2)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	GetDlgItem(IDC_PURCHASE)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	GetDlgItem(IDC_ENTERLICENSEKEY)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	GetDlgItem(IDOK)->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMRegisterDlg::OnPurchase()
{
	CCmdUI cmd;
	cmd.m_nID = ID_APP_PURCHASE;

	p_App->OnCmdMsg(ID_APP_PURCHASE, CN_COMMAND, &cmd, NULL);
}

void FMRegisterDlg::OnEnterLicenseKey()
{
	FMLicenseDlg dlg(this);
	dlg.DoModal();

	if (FMIsLicensed())
		EndDialog(IDOK);
}
