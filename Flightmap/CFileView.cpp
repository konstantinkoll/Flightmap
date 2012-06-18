
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"


// CFileView
//

#define GetSelectedFile() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)
#define GetSelectedAttachment() GetAttachment(GetSelectedFile())

CFileView::CFileView()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CFileView";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CFileView", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Itinerary = NULL;
	p_Flight = NULL;
}

void CFileView::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CFileView::AdjustLayout()
{
	if (!IsWindow(m_wndTaskbar))
		return;
	if (!IsWindow(m_wndExplorerList))
		return;

	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndExplorerList.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::SetData(CItinerary* pItinerary, AIRX_Flight* pFlight)
{
	p_Itinerary = pItinerary;
	p_Flight = pFlight;

	m_wndExplorerList.SetView(p_Flight ? LV_VIEW_TILE : LV_VIEW_DETAILS);
	Reload();
}

void CFileView::Reload()
{
	m_wndExplorerList.SetItemCount(p_Flight ? p_Flight->AttachmentCount : p_Itinerary->m_Attachments.m_ItemCount);
	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);
}

AIRX_Attachment* CFileView::GetAttachment(INT idx)
{
	return idx==-1 ? NULL : p_Flight ? &p_Itinerary->m_Attachments.m_Items[p_Flight->Attachments[idx]] : &p_Itinerary->m_Attachments.m_Items[idx];
}

void CFileView::Init()
{
	m_wndTaskbar.Create(this, IDB_TASKS, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_COPY, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS | LVS_SINGLESEL;
	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(dwStyle, rect, this, 2);

	FMApplication* pApp = (FMApplication*)AfxGetApp();
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListLarge, LVSIL_NORMAL);

	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_SUBITEM_NAME+a));

		m_wndExplorerList.AddColumn(a, tmpStr);
	}

	IMAGEINFO ii;
	pApp->m_SystemImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndExplorerList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy")).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndExplorerList.SetMenus(IDM_FILEVIEW_ITEM, TRUE, IDM_FILEVIEW_BACKGROUND);
	m_wndExplorerList.SetFocus();

	AdjustLayout();
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED, 2, OnItemChanged)
	ON_NOTIFY(LVN_ENDLABELEDIT, 2, OnEndLabelEdit)

	ON_COMMAND(IDM_FILEVIEW_ADD, OnAdd)
	ON_COMMAND(IDM_FILEVIEW_RENAME, OnRename)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILEVIEW_ADD, IDM_FILEVIEW_RENAME, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CFileView::OnNcPaint()
{
	DrawControlBorder(this);
}

void CFileView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO*)pNMHDR;
	AIRX_Attachment* pAttachment = GetAttachment(pDispInfo->item.iItem);

	// Columns
	if (pDispInfo->item.mask & LVIF_COLUMNS)
	{
		pDispInfo->item.cColumns = 2;
		pDispInfo->item.puColumns[0] = 3;
		pDispInfo->item.puColumns[1] = 1;
	}

	// Text
	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			pDispInfo->item.pszText = pAttachment->Name;
			break;
		case 1:
			StrFormatByteSize(pAttachment->Size, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
			break;
		case 2:
		case 3:
			wcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, L"XXX");
			break;
		}
	}

	// Icon
	if (pDispInfo->item.mask & LVIF_IMAGE)
	{
		if (pAttachment->IconID==-1)
		{
			CString tmpStr(pAttachment->Name);
			INT Pos = tmpStr.ReverseFind(L'.');

			CString Ext = (Pos==-1) ? _T("*") : tmpStr.Mid(Pos);

			SHFILEINFO sfi;
			pAttachment->IconID = SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? sfi.iIcon : 3;
		}

		pDispInfo->item.iImage = pAttachment->IconID;
	}

	*pResult = 0;
}

void CFileView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = FALSE;

	if (pDispInfo->item.pszText)
		if (pDispInfo->item.pszText[0]!=L'\0')
		{
			AIRX_Attachment* pAttachment = GetAttachment(pDispInfo->item.iItem);
			if (pAttachment)
			{
				wcscpy_s(pAttachment->Name, MAX_PATH, pDispInfo->item.pszText);
				pAttachment->IconID = -1;
				p_Itinerary->m_IsModified = TRUE;

				*pResult = TRUE;
			}
		}
}

void CFileView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged & LVIF_STATE)
		m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

	*pResult = 0;
}


void CFileView::OnAdd()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, NULL, this);
	if (dlg.DoModal()==IDOK)
	{
		POSITION pos = dlg.GetStartPosition();
		while (pos)
			if (!p_Itinerary->AddAttachment(*p_Flight, dlg.GetNextPathName(pos)))
				break;

		Reload();
	}
}

void CFileView::OnRename()
{
	INT idx = GetSelectedFile();
	if (idx!=-1)
	{
		if (GetFocus()!=&m_wndExplorerList)
			m_wndExplorerList.SetFocus();

		m_wndExplorerList.EditLabel(idx);
	}
}

void CFileView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	switch (pCmdUI->m_nID)
	{
	case IDM_FILEVIEW_ADD:
		if (p_Flight)
			b = (p_Flight->AttachmentCount<AIRX_MaxAttachmentCount);
		break;
	default:
		if (m_wndExplorerList.GetItemCount())
			b = (GetSelectedFile()!=-1);
	}

	pCmdUI->Enable(b);
}
