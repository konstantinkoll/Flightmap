
// PropertiesDlg.cpp: Implementierung der Klasse PropertiesDlg
//

#include "stdafx.h"
#include "Flightmap.h"
#include "PropertiesDlg.h"


// PropertiesDlg
//

PropertiesDlg::PropertiesDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: FMDialog(IDD_PROPERTIES, pParentWnd)
{
	ASSERT(pItinerary);

	p_Itinerary = pItinerary;
}

void PropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_TITLE, p_Itinerary->m_Metadata.Title, 256);
	DDX_Text(pDX, IDC_AUTHOR, p_Itinerary->m_Metadata.Author, 256);
	DDX_Text(pDX, IDC_KEYWORDS, p_Itinerary->m_Metadata.Keywords, 256);
	DDX_Text(pDX, IDC_COMMENTS, p_Itinerary->m_Metadata.Comments, 256);

	if (pDX->m_bSaveAndValidate)
		p_Itinerary->m_IsModified = TRUE;
}

BOOL PropertiesDlg::InitDialog()
{
	// Titel
	CString Caption;
	GetWindowText(Caption);

	CString Text;
	Text.Format(Caption, p_Itinerary->m_DisplayName);

	SetWindowText(Text);

	return TRUE;
}
