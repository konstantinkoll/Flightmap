
// AttachmentsDlg.h: Schnittstelle der Klasse AttachmentsDlg
//

#pragma once
#include "CFileView.h"
#include "CItinerary.h"


// AttachmentsDlg
//

class AttachmentsDlg : public CDialog
{
public:
	AttachmentsDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	CItinerary* p_Itinerary;
	CFileView m_wndFileView;
};
