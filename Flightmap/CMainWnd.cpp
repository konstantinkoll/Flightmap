
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "CMainWnd.h"
#include "Flightmap.h"


// CMainWnd
//

CMainWnd::CMainWnd()
{
	m_hIcon = NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CMainWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CGlasWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	//if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return TRUE;

	return CGlasWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainWnd::AdjustLayout()
{
	/*if (!IsWindow(m_wndJournalButton))
		return;
	if (!IsWindow(m_wndMainView))
		return;

	CRect rect;
	GetLayoutRect(rect);

	const UINT JournalHeight = m_wndJournalButton.GetPreferredHeight();
	const UINT JournalWidth = m_wndJournalButton.GetPreferredWidth();
	m_wndJournalButton.SetWindowPos(NULL, rect.left+1, rect.top+(m_Margins.cyTopHeight-JournalHeight-2)/2, JournalWidth, JournalHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT HistoryHeight = m_wndHistory.GetPreferredHeight();
	const UINT SearchWidth = max(150, (rect.Width()-JournalWidth)/4);
	m_wndHistory.SetWindowPos(NULL, rect.left+JournalWidth+7, rect.top+(m_Margins.cyTopHeight-HistoryHeight-3)/2, rect.Width()-JournalWidth-SearchWidth-14, HistoryHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndSearch.SetWindowPos(NULL, rect.right-SearchWidth, rect.top+(m_Margins.cyTopHeight-HistoryHeight-3)/2, SearchWidth, HistoryHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndMainView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);*/
}


BEGIN_MESSAGE_MAP(CMainWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	theApp.AddFrame(this);

	AdjustLayout();
	SetFocus();

	return 0;
}

void CMainWnd::OnDestroy()
{
	CGlasWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	//if (IsWindow(m_wndMainView))
	//	m_wndMainView.SetFocus();
}
