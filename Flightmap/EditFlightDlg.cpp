
// EditFlightDlg.cpp: Implementierung der Klasse EditFlightDlg
//

#include "stdafx.h"
#include "EditFlightDlg.h"
#include "EditFlightAttachmentsPage.h"
#include "EditFlightRoutePage.h"
#include "EditFlightOtherPage.h"


// EditFlightDlg
//

static UINT LastPageSelected = 0;

EditFlightDlg::EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParent, CItinerary* pItinerary, INT iSelectPage)
	: CPropertySheet(IDS_EDITFLIGHT, pParent, iSelectPage==-1 ? LastPageSelected : iSelectPage)
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
	m_Pages[1] = new EditFlightOtherPage(&m_Flight, pItinerary);
	m_Pages[2] = new EditFlightAttachmentsPage(&m_Flight);

	// Seiten hinzuf�gen
	const UINT nIDTemplates[EditFlightPages] = { IDD_ROUTE, IDD_OTHER, IDD_ATTACHMENTS };
	for (UINT a=0; a<EditFlightPages; a++)
	{
		m_Pages[a]->Construct(nIDTemplates[a]);
		AddPage(m_Pages[a]);
	}

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}

BOOL EditFlightDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (LOWORD(wParam)==IDOK)
		LastPageSelected = GetPageIndex(GetActivePage());

	return CPropertySheet::OnCommand(wParam, lParam);
}


BEGIN_MESSAGE_MAP(EditFlightDlg, CPropertySheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL EditFlightDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDI_EDITFLIGHT);
	SetIcon(hIcon, TRUE);		// Gro�es Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightDlg::OnDestroy()
{
	for (UINT a=0; a<EditFlightPages; a++)
	{
		m_Pages[a]->DestroyWindow();
		delete m_Pages[a];
	}
}
