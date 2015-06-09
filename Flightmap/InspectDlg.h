
// InspectDlg.h: Schnittstelle der Klasse InspectDlg
//

#pragma once
#include "CItinerary.h"


// InspectDlg
//

class InspectDlg : public CDialog
{
public:
	InspectDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

protected:
	void Update();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnShowMetadata();
	afx_msg void OnDeleteMetadata();
	afx_msg void OnShowAttachments();
	afx_msg void OnDeleteAttachments();
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
};
