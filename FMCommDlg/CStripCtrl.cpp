
// CStripCtrl.cpp: Implementierung der Klasse CStripCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// CStripCtrl
//

CStripCtrl::CStripCtrl()
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
	wndcls.lpszClassName = L"CStripCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CStripCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Strip = 0;
	m_Offset = 0;
}

BOOL CStripCtrl::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_DISABLED;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CStripCtrl::SetBitmap(CGdiPlusBitmap* pStrip)
{
	p_Strip = pStrip;

	if (pStrip)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		srand(st.wMilliseconds);

		m_Offset = rand() % pStrip->m_pBitmap->GetWidth();
		SetTimer(1, 17, NULL);
	}
	else
	{
		m_Offset = 0;
		KillTimer(1);
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CStripCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CStripCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CStripCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	if (p_Strip)
	{
		INT l = p_Strip->m_pBitmap->GetWidth();
		INT h = p_Strip->m_pBitmap->GetHeight();
	
		INT left = min(rect.Width(), l-m_Offset);

		Graphics g(pDC);
		g.DrawImage(p_Strip->m_pBitmap, Rect(0, 0, left, h), m_Offset, 0, left, h, UnitPixel);

		if (m_Offset>l-rect.Width())
			g.DrawImage(p_Strip->m_pBitmap, Rect(left, 0, rect.Width()-left, h), 0, 0, rect.Width()-left, h, UnitPixel);
	}
	else
	{
		pDC.FillSolidRect(rect, 0xFFFFFF);
	}
}

void CStripCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ((nIDEvent==1) && (p_Strip))
	{
		INT l = p_Strip->m_pBitmap->GetWidth();
		INT h = p_Strip->m_pBitmap->GetHeight();

		m_Offset = (m_Offset+1) % l;
		ScrollWindow(-1, 0);

		if (p_Strip)
		{
			CPaintDC pDC(this);

			CRect rect;
			GetClientRect(rect);

			Graphics g(pDC);
			g.DrawImage(p_Strip->m_pBitmap, Rect(rect.right-1, 0, 1, h), (m_Offset+rect.right-1) % l, 0, 1, h, UnitPixel);
		}
	}

	CWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}
