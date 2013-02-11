
// InspectDlg.cpp: Implementierung der Klasse InspectDlg
//

#include "stdafx.h"
#include "AttachmentsDlg.h"
#include "InspectDlg.h"
#include "PropertiesDlg.h"


// InspectDlg
//

InspectDlg::InspectDlg(CItinerary* pItinerary, CWnd* pParentWnd)
	: CDialog(IDD_INSPECT, pParentWnd)
{
	p_Itinerary = pItinerary;
}

void InspectDlg::Update()
{
	ASSERT(p_Itinerary);

	// Metadata
	BOOL bEnable = (p_Itinerary->m_Metadata.Author[0]!=L'\0') ||
		(p_Itinerary->m_Metadata.Comments[0]!=L'\0') ||
		(p_Itinerary->m_Metadata.Keywords[0]!=L'\0') ||
		(p_Itinerary->m_Metadata.Title[0]!=L'\0');

	GetDlgItem(IDC_METADATA_SHOW)->EnableWindow(bEnable);
	GetDlgItem(IDC_METADATA_DELETE)->EnableWindow(bEnable);

	CString tmpStr;
	ENSURE(tmpStr.LoadString(bEnable ? IDS_INSPECT_METADATA_PRESENT : IDS_INSPECT_METADATA_NONE));
	GetDlgItem(IDC_METADATA_STATUS)->SetWindowText(tmpStr);

	// Attachments
	bEnable = p_Itinerary->m_Attachments.m_ItemCount;

	GetDlgItem(IDC_ATTACHMENTS_SHOW)->EnableWindow(bEnable);
	GetDlgItem(IDC_ATTACHMENTS_DELETE)->EnableWindow(bEnable);

	if (bEnable)
	{
		INT64 FileSize = 0;
		for (UINT a=0; a<p_Itinerary->m_Attachments.m_ItemCount; a++)
			FileSize += p_Itinerary->m_Attachments.m_Items[a].Size;

		CString tmpMask;
		ENSURE(tmpMask.LoadString(p_Itinerary->m_Attachments.m_ItemCount==1 ? IDS_INSPECT_ATTACHMENTS_SINGULAR : IDS_INSPECT_ATTACHMENTS_PLURAL));

		WCHAR tmpBuf[256];
		StrFormatByteSize(FileSize, tmpBuf, 256);

		tmpStr.Format(tmpMask, p_Itinerary->m_Attachments.m_ItemCount, tmpBuf);
	}
	else
	{
		ENSURE(tmpStr.LoadString(IDS_INSPECT_ATTACHMENTS_NONE));
	}
	GetDlgItem(IDC_ATTACHMENTS_STATUS)->SetWindowText(tmpStr);
}


BEGIN_MESSAGE_MAP(InspectDlg, CDialog)
	ON_BN_CLICKED(IDC_METADATA_SHOW, OnShowMetadata)
	ON_BN_CLICKED(IDC_METADATA_DELETE, OnDeleteMetadata)
	ON_BN_CLICKED(IDC_ATTACHMENTS_SHOW, OnShowAttachments)
	ON_BN_CLICKED(IDC_ATTACHMENTS_DELETE, OnDeleteAttachments)
END_MESSAGE_MAP()

BOOL InspectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_INSPECT);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	Update();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void InspectDlg::OnShowMetadata()
{
	ASSERT(p_Itinerary);

	PropertiesDlg dlg(p_Itinerary, this);
	if (dlg.DoModal()==IDOK)
		Update();
}

void InspectDlg::OnDeleteMetadata()
{
	ASSERT(p_Itinerary);

	ZeroMemory(&p_Itinerary->m_Metadata, sizeof(AIRX_Metadata));
	Update();
}

void InspectDlg::OnShowAttachments()
{
	ASSERT(p_Itinerary);

	AttachmentsDlg dlg(p_Itinerary, this);
	if (dlg.DoModal()==IDOK)
		Update();
}

void InspectDlg::OnDeleteAttachments()
{
	ASSERT(p_Itinerary);

	CString caption;
	CString message;
	ENSURE(caption.LoadString(IDS_DELETE_CAPTION));
	ENSURE(message.LoadString(IDS_DELETE_ALL));

	if (MessageBox(message, caption, MB_YESNO | MB_ICONWARNING)==IDYES)
	{
		p_Itinerary->DeleteAttachments();
		Update();
	}
}
