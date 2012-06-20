
// AttachmentsDlg.cpp: Implementierung der Klasse AttachmentsDlg
//

#include "stdafx.h"
#include "AttachmentsDlg.h"


// AttachmentsDlg
//

AttachmentsDlg::AttachmentsDlg(CItinerary* pItinerary, CWnd* pParent)
	: CDialog(IDD_ATTACHMENTS, pParent)
{
	p_Itinerary = pItinerary;
}

void AttachmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_FILEVIEW, m_wndFileView);
}


BEGIN_MESSAGE_MAP(AttachmentsDlg, CDialog)
END_MESSAGE_MAP()

BOOL AttachmentsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDI_ATTACHMENTS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// FileView
	m_wndFileView.SetData(GetDlgItem(IDC_FILESTATUS), p_Itinerary);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
