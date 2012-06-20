
// InspectDlg.cpp: Implementierung der Klasse InspectDlg
//

#include "stdafx.h"
#include "InspectDlg.h"


// InspectDlg
//

InspectDlg::InspectDlg(CItinerary* pItinerary, CWnd* pParent)
	: CDialog(IDD_INSPECT, pParent)
{
	p_Itinerary = pItinerary;
}


BEGIN_MESSAGE_MAP(InspectDlg, CDialog)
END_MESSAGE_MAP()

BOOL InspectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_INSPECT);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
