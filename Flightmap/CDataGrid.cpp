
// CDataGrid.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "CDataGrid.h"
#include "Flightmap.h"
#include "Resource.h"


// CDataGrid
//

CDataGrid::CDataGrid()
	: CWnd()
{
}

BOOL CDataGrid::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CDataGrid, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CDataGrid::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	// TODO
	pDC->TextOut(10, 10, _T("This version of Flightmap does not support editing yet (sorry)."));

	return TRUE;
}
