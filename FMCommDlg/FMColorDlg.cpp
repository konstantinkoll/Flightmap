
// FMColorDlg.cpp: Implementierung der Klasse FMColorDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMColorDlg
//

FMColorDlg::FMColorDlg(CWnd* pParentWnd, COLORREF clrInit, DWORD dwFlags, CString Caption)
	: CColorDialog(clrInit, dwFlags, pParentWnd)
{
	m_Caption = Caption;

	m_cc.lpTemplateName = MAKEINTRESOURCE(IDD_CHOOSECOLOR);
	m_cc.hInstance = (HWND)AfxGetResourceHandle();
	m_cc.Flags |= CC_ENABLETEMPLATE | CC_SOLIDCOLOR;
	m_cc.lpCustColors = FMGetApp()->m_CustomColors;
}


BEGIN_MESSAGE_MAP(FMColorDlg, CColorDialog)
END_MESSAGE_MAP()

BOOL FMColorDlg::OnInitDialog()
{
	CColorDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = FMGetApp()->LoadDialogIcon(IDD_CHOOSECOLOR);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	if (!m_Caption.IsEmpty())
		SetWindowText(m_Caption);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
