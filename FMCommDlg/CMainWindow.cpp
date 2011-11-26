
// CMainWindow.cpp: Implementierung der Klasse CMainWindow
//

#include "stdafx.h"
#include "CMainWindow.h"


// CMainWindow
//

CMainWindow::CMainWindow()
	: CWnd()
{
	p_App = (FMApplication*)AfxGetApp();
	p_PopupWindow = NULL;
	m_pDialogMenuBar = NULL;
	m_Active = TRUE;
}

BOOL CMainWindow::Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::CreateEx(WS_EX_APPWINDOW | WS_EX_CONTROLPARENT, lpszClassName, lpszWindowName,
		dwStyle | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, nID);
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg)
{
	if (p_PopupWindow)
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCRBUTTONUP:
		case WM_NCMBUTTONUP:
			if (IsWindow(p_PopupWindow->GetSafeHwnd()))
			{
				CPoint pt;
				GetCursorPos(&pt);

				if (!p_PopupWindow->SendMessage(WM_PTINRECT, MAKEWPARAM(pt.x, pt.y)))
					p_PopupWindow->GetOwner()->SendMessage(WM_CLOSEPOPUP);
			}
		}

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CMainWindow::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return p_App->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainWindow::AdjustLayout()
{
	if (m_pDialogMenuBar)
	{
		CRect rect;
		GetClientRect(rect);

		m_pDialogMenuBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.top+m_pDialogMenuBar->GetPreferredHeight(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void CMainWindow::PostNcDestroy()
{
	delete this;
}

void CMainWindow::RegisterPopupWindow(CWnd* pPopupWnd)
{
	if (!p_PopupWindow)
		p_PopupWindow = pPopupWnd;
}


BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCACTIVATE()
	ON_WM_ACTIVATE()
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_RBUTTONUP()
	ON_MESSAGE_VOID(WM_CLOSEPOPUP, OnClosePopup)
END_MESSAGE_MAP()

INT CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_Active = (CWnd::GetActiveWindow()==this);

	return 0;
}

void CMainWindow::OnDestroy()
{
	if (m_pDialogMenuBar)
	{
		m_pDialogMenuBar->DestroyWindow();
		delete m_pDialogMenuBar;
	}

	CWnd::OnDestroy();
}

BOOL CMainWindow::OnNcActivate(BOOL bActive)
{
	if ((bActive!=m_Active) && ((!p_PopupWindow) || (bActive)))
	{
		m_Active = bActive;

		if (m_pDialogMenuBar)
			m_pDialogMenuBar->Invalidate();
	}

	return (bActive || !p_PopupWindow) ? CWnd::OnNcActivate(bActive) : TRUE;
}

void CMainWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (!p_PopupWindow)
		CWnd::OnActivate(nState, pWndOther, bMinimized);
}

LRESULT CMainWindow::OnThemeChanged()
{
	if (m_pDialogMenuBar)
		m_pDialogMenuBar->SetTheme();

	AdjustLayout();

	return TRUE;
}

void CMainWindow::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainWindow::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CWnd::OnGetMinMaxInfo(lpMMI);

	if (GetStyle() & WS_MAXIMIZEBOX)
	{
		lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 
			m_pDialogMenuBar ? max(384, m_pDialogMenuBar->GetMinWidth()+16) : 384);
		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 
			256+GetSystemMetrics(SM_CYCAPTION));
	}
}

void CMainWindow::OnClosePopup()
{
	if (p_PopupWindow)
	{
		p_PopupWindow->DestroyWindow();
		delete p_PopupWindow;
		p_PopupWindow = NULL;

		SetFocus();
		Invalidate();
		UpdateWindow();				// Essential, as window's redraw flag may be false
	}
}
