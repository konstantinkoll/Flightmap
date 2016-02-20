
// AttachmentsDlg.cpp: Implementierung der Klasse AttachmentsDlg
//

#include "stdafx.h"
#include "AttachmentsDlg.h"
#include "Flightmap.h"


// AttachmentsDlg
//

AttachmentsDlg::AttachmentsDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_ATTACHMENTS, pParentWnd)
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

	// FileView
	m_wndFileView.SetData(p_Itinerary);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
