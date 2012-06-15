
// CFileView.cpp: Implementierung der Klasse CFileView
//

#include "stdafx.h"
#include "CFileView.h"


// CFileView
//

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

void CFileView::Init()
{
	m_wndTaskbar.Create(this, IDB_TASKS, 1);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_ADD, 0);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_OPEN, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_COPY, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_FILEVIEW_RENAME, 4);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS;
	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(dwStyle, rect, this, 2);

	FMApplication* pApp = (FMApplication*)AfxGetApp();
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&pApp->m_SystemImageListLarge, LVSIL_NORMAL);

	IMAGEINFO ii;
	pApp->m_SystemImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndExplorerList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy")).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndExplorerList.SetFocus();

	AdjustLayout();
}


BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_SIZE()

	ON_COMMAND(IDM_FILEVIEW_ADD, OnAdd)
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


void CFileView::OnAdd()
{
	MessageBox(_T("Test"));
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
			b = (m_wndExplorerList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED)!=-1);
	}

	pCmdUI->Enable(b);
}
