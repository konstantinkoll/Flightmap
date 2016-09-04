
// CFileMenu.cpp: Implementierung der Klasse CFileMenu
//

#include "stdafx.h"
#include "AttachmentsDlg.h"
#include "CFileMenu.h"
#include "Flightmap.h"
#include "PropertiesDlg.h"
#include <shlobj.h>
#include <winspool.h>


// CFileMenu
//

CIcons CFileMenu::m_LargeIcons;
CIcons CFileMenu::m_SmallIcons;

CFileMenu::CFileMenu()
	: CFrontstageWnd()
{
	m_DefaultPrinterAvailable = m_Resizing = FALSE;
}

BOOL CFileMenu::Create(CWnd* pParentWnd, UINT nID, CItinerary* pItinerary)
{
	p_Itinerary = pItinerary;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CFileMenu::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route commands to owner
	if (!CFrontstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return GetOwner()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

	return TRUE;
}

void CFileMenu::SetItinerary(CItinerary* pItinerary)
{
	p_Itinerary = pItinerary;

	Update();
}

void CFileMenu::Update()
{
	if (p_Itinerary)
	{
		// Header
		m_wndHeaderArea.SetText(p_Itinerary->m_Metadata.Title[0] ? p_Itinerary->m_Metadata.Title : p_Itinerary->m_DisplayName, p_Itinerary->m_FileName);

		// Save and convert
		m_wndFloatButtons.SetGroupAlert(2, p_Itinerary->m_IsModified);

		// Print
		WCHAR tmpBuf[256];
		DWORD cchBuffer = 256;
		if ((m_DefaultPrinterAvailable=GetDefaultPrinter(tmpBuf, &cchBuffer))==TRUE)
		{
			m_wndFloatButtons.SetText(3, 2, IDS_FILEMENU_DEFAULTPRINTER);
			m_wndFloatButtons.SetText(3, 3, tmpBuf, TRUE);
		}
		else
		{
			m_wndFloatButtons.SetText(3, 2, L"");
			m_wndFloatButtons.SetText(3, 3, L"");
		}

		// Prepare
		const BOOL HasAuthor = (p_Itinerary->m_Metadata.Author[0]!=L'\0');
		const BOOL HasComments = (p_Itinerary->m_Metadata.Comments[0]!=L'\0');
		const BOOL HasKeywords = (p_Itinerary->m_Metadata.Keywords[0]!=L'\0');
		const BOOL HasTitle = (p_Itinerary->m_Metadata.Title[0]!=L'\0');
		const BOOL HasAttachments = (p_Itinerary->m_Attachments.m_ItemCount>0);
		const BOOL HasMetadata = HasAuthor || HasComments || HasKeywords || HasTitle || HasAttachments;

		m_wndFloatButtons.SetGroupAlert(4, HasMetadata);
		m_wndFloatButtons.SetText(4, 2, HasMetadata ? IDS_FILEMENU_METADATA : 0);
		m_wndFloatButtons.SetText(4, 3, HasAuthor ? IDS_FILEMENU_AUTHOR : 0, TRUE);
		m_wndFloatButtons.SetText(4, 4, HasComments ? IDS_FILEMENU_COMMENTS : 0, TRUE);
		m_wndFloatButtons.SetText(4, 5, HasKeywords ? IDS_FILEMENU_KEYWORDS : 0, TRUE);
		m_wndFloatButtons.SetText(4, 6, HasTitle ? IDS_FILEMENU_TITLE : 0, TRUE);

		if (HasAttachments)
		{
			INT64 FileSize = 0;
			for (UINT a=0; a<p_Itinerary->m_Attachments.m_ItemCount; a++)
				FileSize += p_Itinerary->m_Attachments[a].Size;

			StrFormatByteSize(FileSize, tmpBuf, 256);

			CString tmpStr;
			tmpStr.Format(p_Itinerary->m_Attachments.m_ItemCount==1 ? IDS_FILESTATUS_SINGULAR : IDS_FILESTATUS_PLURAL, p_Itinerary->m_Attachments.m_ItemCount, tmpBuf);

			m_wndFloatButtons.SetText(4, 7, tmpStr, TRUE);
		}
		else
		{
			m_wndFloatButtons.SetText(4, 7, L"");
		}

		m_wndFloatButtons.AdjustLayout();
	}
	else
	{
		// Save and convert
		m_wndFloatButtons.SetGroupAlert(2, FALSE);

		// Prepare
		m_wndFloatButtons.SetGroupAlert(4, FALSE);
	}
}

void CFileMenu::AdjustLayout()
{
	m_Resizing = TRUE;

	CRect rect;
	GetClientRect(rect);

	UINT HeaderHeight = 0;
	if (p_Itinerary)
	{
		HeaderHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, 0, 0, rect.right, HeaderHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	const INT MaxWidth = max(m_wndRecentFilesPane.GetMinWidth(), min(rect.Width()*2/5, 350));
	const INT RecentFilesWidth = min(MaxWidth, max(m_wndRecentFilesPane.GetMinWidth(), m_wndRecentFilesPane.GetPreferredWidth()));

	m_wndRecentFilesPane.SetMaxWidth(MaxWidth);
	m_wndRecentFilesPane.SetWindowPos(NULL, rect.right-RecentFilesWidth, HeaderHeight, RecentFilesWidth, rect.bottom-HeaderHeight, SWP_NOZORDER | SWP_NOACTIVATE);

	m_wndFloatButtons.SetWindowPos(NULL, 0, HeaderHeight, rect.right-RecentFilesWidth, rect.bottom-HeaderHeight, SWP_NOZORDER | SWP_NOACTIVATE);

	m_Resizing = FALSE;
}


BEGIN_MESSAGE_MAP(CFileMenu, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)

	ON_COMMAND(IDM_FILE_VIEWINFOLDER, OnFileViewInFolder)
	ON_COMMAND(IDM_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(IDM_FILE_ATTACHMENTS, OnFileAttachments)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_VIEWINFOLDER, IDM_FILE_ATTACHMENTS, OnUpdateFileCommands)
END_MESSAGE_MAP()

INT CFileMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Header area
	if (!m_wndHeaderArea.Create(this, 1))
		return FALSE;

	m_wndHeaderArea.AddButton(IDM_FILE_PROPERTIES);
	m_wndHeaderArea.AddButton(IDM_FILE_VIEWINFOLDER);

	// Float buttons
	if (!m_wndFloatButtons.Create(this, m_LargeIcons, m_SmallIcons, IDB_FILEMENU_16, 2))
		return -1;

	m_wndFloatButtons.BeginGroup(IDS_FILEMENU_NEW);
	m_wndFloatButtons.AddButton(IDM_FILE_NEW, 0);
	m_wndFloatButtons.AddText(IDS_FILEMENU_TEMPLATES);
	m_wndFloatButtons.AddButton(IDM_FILE_NEWSAMPLE1, 1, TRUE);
	m_wndFloatButtons.AddButton(IDM_FILE_NEWSAMPLE2, 1, TRUE);

	m_wndFloatButtons.BeginGroup(IDS_FILEMENU_OPEN);
	m_wndFloatButtons.AddButton(IDM_FILE_OPEN, 2);
	m_wndFloatButtons.AddButton(IDM_FILE_CLOSE, 3);

	m_wndFloatButtons.BeginGroup(IDS_FILEMENU_SAVE);
	m_wndFloatButtons.AddButton(IDM_FILE_SAVE, 5);
	m_wndFloatButtons.AddText(IDS_FILEMENU_SAVEAS);
	m_wndFloatButtons.AddButton(IDM_FILE_SAVEAS, 6, TRUE);
	m_wndFloatButtons.BeginNewColumn();
	m_wndFloatButtons.AddText(IDS_FILEMENU_CONVERT);
	m_wndFloatButtons.AddButton(IDM_FILE_SAVEAS_CSV, 7, TRUE);
	m_wndFloatButtons.AddButton(IDM_FILE_SAVEAS_TXT, 8, TRUE);
	m_wndFloatButtons.AddText(IDS_FILEMENU_OTHERFORMATS);
	m_wndFloatButtons.AddButton(IDM_FILE_SAVEAS_OTHER, 9, TRUE);

	m_wndFloatButtons.BeginGroup(IDS_FILEMENU_PRINT);
	m_wndFloatButtons.AddButton(IDM_FILE_PRINTQUICK, 10);
	m_wndFloatButtons.AddButton(IDM_FILE_PRINT, 11);
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");

	m_wndFloatButtons.BeginGroup(IDS_FILEMENU_PREPARE);
	m_wndFloatButtons.AddButton(IDM_FILE_PROPERTIES, 12);
	m_wndFloatButtons.AddButton(IDM_FILE_ATTACHMENTS, 13);
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");
	m_wndFloatButtons.AddText(L"");

	// Recents pane
	if (!m_wndRecentFilesPane.Create(this, 3, FALSE, 250))
		return -1;

	m_wndRecentFilesPane.SetOwner(GetOwner());

	Update();

	return 0;
}

BOOL CFileMenu::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFileMenu::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileMenu::OnSettingChange(UINT /*nFlags*/, LPCTSTR /*lpszSection*/)
{
	Update();
}

void CFileMenu::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndFloatButtons.SetFocus();
}

void CFileMenu::OnAdjustLayout()
{
	if (!m_Resizing)
		AdjustLayout();
}


void CFileMenu::OnFileViewInFolder()
{
	assert(p_Itinerary);
	assert(p_Itinerary->m_FileName[0]!=L'\0');

	LPITEMIDLIST pidlFQ;
	if (SUCCEEDED(SHParseDisplayName(p_Itinerary->m_FileName, NULL, &pidlFQ, 0, NULL)))
	{
		SHOpenFolderAndSelectItems(pidlFQ, 0, NULL, 0);

		FMGetApp()->GetShellManager()->FreeItem(pidlFQ);
	}
}

void CFileMenu::OnFileProperties()
{
	ASSERT(p_Itinerary);

	PropertiesDlg dlg(p_Itinerary, this);
	if (dlg.DoModal()==IDOK)
		Update();
}

void CFileMenu::OnFileAttachments()
{
	ASSERT(p_Itinerary);

	AttachmentsDlg dlg(p_Itinerary, this);
	dlg.DoModal();
}

void CFileMenu::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (p_Itinerary!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_VIEWINFOLDER:
		if (p_Itinerary)
			bEnable = (p_Itinerary->m_FileName[0]!=L'\0');

		break;

	case IDM_FILE_NEW:
	case IDM_FILE_NEWSAMPLE1:
	case IDM_FILE_NEWSAMPLE2:
	case IDM_FILE_OPEN:
	case IDM_FILE_OPENRECENT:
		bEnable = TRUE;
		break;

	case IDM_FILE_SAVE:
		if (p_Itinerary)
			bEnable = p_Itinerary->m_IsModified;

		break;

	case IDM_FILE_PRINTQUICK:
		bEnable &= m_DefaultPrinterAvailable;
		break;

	case IDM_FILE_ATTACHMENTS:
		if (p_Itinerary)
			bEnable = (p_Itinerary->m_Attachments.m_ItemCount>0);

		break;
	}

	pCmdUI->Enable(bEnable);
}
