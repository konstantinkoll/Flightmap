
// CGroupBox.cpp: Implementierung der Klasse CGroupBox
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CGroupBox
//

CGroupBox::CGroupBox()
	: CStatic()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CGroupBox";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CGroupBox", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CGroupBox::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS | WS_DISABLED);
}


BEGIN_MESSAGE_MAP(CGroupBox, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CGroupBox::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGroupBox::OnPaint()
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

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	BOOL Themed = IsCtrlThemed();

	// Background
	HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	FillRect(dc, rect, hBrush);

	// Border
	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);

	CString Caption;
	GetWindowText(Caption);
	CSize sz = dc.GetTextExtent(Caption);

	CRect rectBounds(rect);
	rectBounds.top += sz.cy/2;

	if (Themed && FMGetApp()->m_UseBgImages)
	{
		rectBounds.right -= 3;
		rectBounds.bottom -= 3;

		Matrix m1;
		m1.Translate(2.0, 2.0);

		Matrix m2;
		m2.Translate(-1.0, -1.0);

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);

		Pen pen(Color(0xE0, 196, 240, 248));
		g.DrawPath(&pen, &path);

		path.Transform(&m1);
		pen.SetColor(Color(0x80, 0xFF, 0xFF, 0xFF));
		g.DrawPath(&pen, &path);

		path.Transform(&m2);
		pen.SetColor(Color(0x40, 60, 96, 112));
		g.DrawPath(&pen, &path);

		dc.SetTextColor(0xCC6600);
	}
	else
		if (Themed)
		{
			if (FMGetApp()->OSVersion==OS_Eight)
			{
				dc.Draw3dRect(rect, 0xDDDDDD, 0xDDDDDD);
			}
			else
			{
				GraphicsPath path;
				CreateRoundRectangle(rectBounds, 2, path);
	
				Pen pen(Color(0xDD, 0xDD, 0xDD));
				g.DrawPath(&pen, &path);
			}

			dc.SetTextColor(0xCC3300);
		}
		else
		{
			rectBounds.right--;
			dc.Draw3dRect(rectBounds, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));

			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		}

	// Caption
	CRect rectCaption(rect);
	rectCaption.left = rectBounds.left+6;
	rectCaption.right = min(rectCaption.left+sz.cx+4, rectBounds.right-6);
	rectCaption.bottom = rectCaption.top+sz.cy;

	if (hBrush)
		FillRect(dc, rectCaption, hBrush);

	dc.DrawText(Caption, rectCaption, DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
