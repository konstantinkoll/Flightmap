
// FMColorDlg.cpp: Implementierung der Klasse FMColorDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMColorDlg
//

FMColorDlg::FMColorDlg(COLORREF clrInit, DWORD dwFlags, CWnd* pParentWnd)
	: CColorDialog(clrInit, dwFlags, pParentWnd)
{
	hIconL = hIconS = NULL;

	m_cc.lpTemplateName = MAKEINTRESOURCE(IDD_CHOOSECOLOR);
	m_cc.hInstance = (HWND)AfxGetResourceHandle();
	m_cc.Flags |= CC_ENABLETEMPLATE | CC_SOLIDCOLOR;
	m_cc.lpCustColors = ((FMApplication*)AfxGetApp())->m_CustomColors;
}


BEGIN_MESSAGE_MAP(FMColorDlg, CColorDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL FMColorDlg::OnInitDialog()
{
	CColorDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	hIconS = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_CHOOSECOLOR), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SetIcon(hIconS, FALSE);
	hIconL = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_CHOOSECOLOR), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	SetIcon(hIconL, TRUE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMColorDlg::OnDestroy()
{
	if (hIconL)
		DestroyIcon(hIconL);
	if (hIconS)
		DestroyIcon(hIconS);

	CColorDialog::OnDestroy();
}
