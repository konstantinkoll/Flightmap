
// AttachmentsDlg.h: Schnittstelle der Klasse AttachmentsDlg
//

#pragma once
#include "CFileView.h"
#include "CItinerary.h"
#include "FMCommDlg.h"


// AttachmentsDlg
//

class AttachmentsDlg : public FMDialog
{
public:
	AttachmentsDlg(CItinerary* pItinerary, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

protected:
	CItinerary* p_Itinerary;
	CFileView m_wndFileView;
};
