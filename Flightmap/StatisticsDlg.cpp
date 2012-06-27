
// StatisticsDlg.cpp: Implementierung der Klasse StatisticsDlg
//

#include "stdafx.h"
#include "StatisticsDlg.h"


// StatisticsDlg
//

StatisticsDlg::StatisticsDlg(CItinerary* pItinerary, CWnd* pParent)
	: CDialog(IDD_STATISTICS, pParent)
{
	p_Itinerary = pItinerary;
}

void StatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	/*DDX_Control(pDX, IDC_CARRIER, m_wndCarrier);
	DDX_Control(pDX, IDC_ROUTE, m_wndRoute);
	DDX_MaskedText(pDX, IDC_COMMENT, m_wndComment, 22, &m_FlightTemplate);
	DDX_MaskedText(pDX, IDC_ETIXCODE, m_wndEtixCode, 16, &m_FlightTemplate);*/
	DDX_Control(pDX, IDC_FILTER_RATING, m_wndRating);
}


BEGIN_MESSAGE_MAP(StatisticsDlg, CDialog)
END_MESSAGE_MAP()

BOOL StatisticsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_STATISTICS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Rating
	m_wndRating.SetRating(0);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
