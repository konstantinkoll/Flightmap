
// EditFlightDlg.cpp: Implementierung der Klasse EditFlightDlg
//

#include "stdafx.h"
#include "EditFlightDlg.h"
#include "EditFlightAttachmentsPage.h"
#include "EditFlightRoutePage.h"
#include "EditFlightOtherPage.h"
#include "Flightmap.h"


// EditFlightDlg
//

static UINT LastPageSelected = 0;

EditFlightDlg::EditFlightDlg(AIRX_Flight* pFlight, CWnd* pParentWnd, CItinerary* pItinerary, INT iSelectPage)
	: CPropertySheet(IDS_EDITFLIGHT, pParentWnd, iSelectPage==-1 ? LastPageSelected : iSelectPage)
{
	if (pFlight)
	{
		m_Flight = *pFlight;
	}
	else
	{
		ResetFlight(m_Flight);
	}

	m_pPages[0] = new EditFlightRoutePage(&m_Flight);
	m_pPages[1] = new EditFlightOtherPage(&m_Flight, pItinerary);
	m_pPages[2] = new EditFlightAttachmentsPage(&m_Flight, pItinerary);

	// Seiten hinzufügen
	const UINT nIDTemplates[] = { IDD_EDITFLIGHT_ROUTE, IDD_EDITFLIGHT_OTHER, IDD_EDITFLIGHT_ATTACHMENTS };
	for (UINT a=0; a<EDITFLIGHTPAGES; a++)
	{
		m_pPages[a]->Construct(nIDTemplates[a]);
		AddPage(m_pPages[a]);
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

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadDialogIcon(IDI_EDITFLIGHT);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFlightDlg::OnDestroy()
{
	for (UINT a=0; a<EDITFLIGHTPAGES; a++)
	{
		m_pPages[a]->DestroyWindow();
		delete m_pPages[a];
	}
}
