
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "CLoungeView.h"
#include "CMainWnd.h"
#include "Flightmap.h"


// CMainWnd
//

CMainWnd::CMainWnd()
	: CMainWindow()
{
	m_hIcon = NULL;
	m_pWndMainView = NULL;
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

	return CMainWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_pWndMainView)
		if (m_pWndMainView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	return CMainWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainWnd::AdjustLayout()
{
	CMainWindow::AdjustLayout();

	if (!m_pWndMainView)
		return;

	CRect rect;
	GetClientRect(rect);

	INT top = rect.top;
	if (m_pDialogMenuBar)
		top += m_pDialogMenuBar->GetPreferredHeight();

	m_pWndMainView->SetWindowPos(NULL, rect.left, top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CMainWnd::OpenMainView(BOOL Empty)
{
	if (m_pWndMainView)
	{
		m_pWndMainView->DestroyWindow();
		delete m_pWndMainView;
	}

	if (Empty)
	{
		m_pWndMainView = new CLoungeView();
		((CLoungeView*)m_pWndMainView)->Create(this, 2);
	}
	else
	{
		m_pWndMainView = NULL;	// TODO
	}

	AdjustLayout();
	SetFocus();
}


BEGIN_MESSAGE_MAP(CMainWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMainWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS, 1);

	m_pDialogMenuBar->AddMenuLeft(0, IDM_FILE);
	m_pDialogMenuBar->AddMenuLeft(1, IDM_EDIT);
	m_pDialogMenuBar->AddMenuLeft(2, IDM_MAP);
	m_pDialogMenuBar->AddMenuLeft(3, IDM_GLOBE);
	m_pDialogMenuBar->AddMenuLeft(4, IDM_STATISTICS);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 0);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 1);
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 2);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 3);

	theApp.AddFrame(this);

	OpenMainView(TRUE);

	return 0;
}

void CMainWnd::OnDestroy()
{
	if (m_pWndMainView)
	{
		m_pWndMainView->DestroyWindow();
		delete m_pWndMainView;
	}

	CMainWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (m_pWndMainView)
		m_pWndMainView->SetFocus();
}
