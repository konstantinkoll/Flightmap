
// EditFlightDlg.cpp: Implementierung der Klasse EditFlightDlg
//

#include "stdafx.h"
#include "EditFlightDlg.h"


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

	// Seiten hinzufügen
	AddPage(&m_Page0);
}

void EditFlightDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
	}
}


BEGIN_MESSAGE_MAP(EditFlightDlg, CPropertySheet)
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
