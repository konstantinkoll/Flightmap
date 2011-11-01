
// CPictureCtrl.cpp: Implementierung der Klasse CPictureCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CPictureCtrl
//

CPictureCtrl::CPictureCtrl()
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
	wndcls.lpszClassName = L"CPictureCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CPictureCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CPictureCtrl::SetPicture(UINT nResID, LPCTSTR Type)
{
	ENSURE(m_Picture.Load(nResID, Type));

	Invalidate();
}


BEGIN_MESSAGE_MAP(CPictureCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CPictureCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPictureCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

void CPictureCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	if (m_Picture.m_pBitmap)
	{
		Graphics g(dc);

		INT l = m_Picture.m_pBitmap->GetWidth();
		INT h = m_Picture.m_pBitmap->GetHeight();

		if ((l>=rect.Width()) && (h>=rect.Height()))
		{
			g.DrawImage(m_Picture.m_pBitmap, 0, 0, (l-rect.Width())/2, (h-rect.Height())/2, l, h, UnitPixel);
		}
		else
		{
			DOUBLE f = max((DOUBLE)rect.Width()/l, (DOUBLE)rect.Height()/h);
			g.DrawImage(m_Picture.m_pBitmap, 0, 0, (INT)(l*f), (INT)(h*f));
		}
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
