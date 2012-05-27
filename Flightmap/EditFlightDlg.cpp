
// EditFlightDlg.cpp: Implementierung der Klasse EditFlightDlg
//

#include "stdafx.h"
#include "EditFlightDlg.h"
#include "EditFlightAttachmentsPage.h"
#include "EditFlightRoutePage.h"
#include "EditFlightOtherPage.h"


// EditFlightDlg
//

EditFlightDlg::EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParent)
: CPropertySheet(IDS_EDITFLIGHT, pParent)
{
	if (pFlight)
	{
		m_Flight = *pFlight;
	}
	else
	{
		ResetFlight(m_Flight);
	}

	m_Pages[0] = new EditFlightRoutePage(&m_Flight);
	m_Pages[1] = new EditFlightOtherPage(&m_Flight);
	m_Pages[2] = new EditFlightAttachmentsPage(&m_Flight);

	// Seiten hinzufügen
	const UINT nIDTemplates[EditFlightPages] = { IDD_ROUTE, IDD_OTHER, IDD_ATTACHMENTS };
	for (UINT a=0; a<EditFlightPages; a++)
	{
		m_Pages[a]->Construct(nIDTemplates[a]);
		AddPage(m_Pages[a]);
	}
}

void EditFlightDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
	}
}


BEGIN_MESSAGE_MAP(EditFlightDlg, CPropertySheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL EditFlightDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDI_EDIT);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightDlg::OnDestroy()
{
	for (UINT a=0; a<EditFlightPages; a++)
	{
		m_Pages[a]->DestroyWindow();
		delete m_Pages[a];
	}
}
