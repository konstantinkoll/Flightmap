
// AttachmentsDlg.cpp: Implementierung der Klasse AttachmentsDlg
//

#include "stdafx.h"
#include "AttachmentsDlg.h"


// AttachmentsDlg
//

AttachmentsDlg::AttachmentsDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: FMDialog(IDD_ATTACHMENTS, pParentWnd)
{
	p_Itinerary = pItinerary;
}

void AttachmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_FILEVIEW, m_wndFileView);
}

BOOL AttachmentsDlg::InitDialog()
{
	// FileView
	m_wndFileView.SetItinerary(p_Itinerary);

	return TRUE;
}
