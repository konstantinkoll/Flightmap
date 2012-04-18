
// PropertiesDlg.cpp: Implementierung der Klasse PropertiesDlg
//

#include "stdafx.h"
#include "PropertiesDlg.h"


// PropertiesDlg
//

PropertiesDlg::PropertiesDlg(CItinerary* pItinerary, CWnd* pParent)
	: CDialog(IDD_PROPERTIES, pParent)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;
}

void PropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		GetDlgItem(IDC_TITLE)->GetWindowText(p_Itinerary->m_Metadata.Title, 256);
		GetDlgItem(IDC_AUTHOR)->GetWindowText(p_Itinerary->m_Metadata.Author, 256);
		GetDlgItem(IDC_KEYWORDS)->GetWindowText(p_Itinerary->m_Metadata.Keywords, 256);
		GetDlgItem(IDC_COMMENTS)->GetWindowText(p_Itinerary->m_Metadata.Comments, 256);

		p_Itinerary->m_IsModified = TRUE;
	}
}


BEGIN_MESSAGE_MAP(PropertiesDlg, CDialog)
END_MESSAGE_MAP()

BOOL PropertiesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_PROPERTIES);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titel
	CString mask;
	CString caption;
	GetWindowText(mask);
	caption.Format(mask, p_Itinerary->m_DisplayName);
	SetWindowText(caption);

	GetDlgItem(IDC_TITLE)->SetWindowText(p_Itinerary->m_Metadata.Title);
	GetDlgItem(IDC_AUTHOR)->SetWindowText(p_Itinerary->m_Metadata.Author);
	GetDlgItem(IDC_KEYWORDS)->SetWindowText(p_Itinerary->m_Metadata.Keywords);
	GetDlgItem(IDC_COMMENTS)->SetWindowText(p_Itinerary->m_Metadata.Comments);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
