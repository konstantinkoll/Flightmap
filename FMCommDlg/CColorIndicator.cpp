
// CColorIndicator.cpp: Implementierung der Klasse CColorIndicator
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CColorIndicator
//

CColorIndicator::CColorIndicator()
	: CStatic()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CColorIndicator";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CColorIndicator", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_Color = (COLORREF)-1;
}

void CColorIndicator::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS);
}

void CColorIndicator::SetColor(COLORREF clr)
{
	m_Color = clr;
	Invalidate();
}


BEGIN_MESSAGE_MAP(CColorIndicator, CStatic)
	ON_WM_ENABLE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CColorIndicator::OnEnable(BOOL /*bEnable*/)
{
	Invalidate();
}

BOOL CColorIndicator::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CColorIndicator::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Color
	DrawColor(dc, rect, IsCtrlThemed(), m_Color, IsWindowEnabled());

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}
