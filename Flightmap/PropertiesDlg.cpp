
// PropertiesDlg.cpp: Implementierung der Klasse PropertiesDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "PropertiesDlg.h"


// PropertiesDlg
//

PropertiesDlg::PropertiesDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_PROPERTIES, pParentWnd)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;
}

void PropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_TITLE, p_Itinerary->m_Metadata.Title, 256);
	DDX_Text(pDX, IDC_AUTHOR, p_Itinerary->m_Metadata.Author, 256);
	DDX_Text(pDX, IDC_KEYWORDS, p_Itinerary->m_Metadata.Keywords, 256);
	DDX_Text(pDX, IDC_COMMENTS, p_Itinerary->m_Metadata.Comments, 256);

	if (pDX->m_bSaveAndValidate)
		p_Itinerary->m_IsModified = TRUE;
}


BEGIN_MESSAGE_MAP(PropertiesDlg, CDialog)
END_MESSAGE_MAP()

BOOL PropertiesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadDialogIcon(IDD_PROPERTIES);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Titel
	CString Mask;
	CString Caption;
	GetWindowText(Mask);
	Caption.Format(Mask, p_Itinerary->m_DisplayName);
	SetWindowText(Caption);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
