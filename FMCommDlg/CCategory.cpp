
// CCategory.cpp: Implementierung der Klasse CCategory
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CCategory
//

CCategory::CCategory()
	: CStatic()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CCategory";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CCategory", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CCategory::PreSubclassWindow()
{
	CStatic::PreSubclassWindow();

	ModifyStyle(WS_BORDER, WS_CLIPSIBLINGS | WS_DISABLED);
}


BEGIN_MESSAGE_MAP(CCategory, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CCategory::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CCategory::OnPaint()
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

	// Caption
	WCHAR tmpStr[256];
	GetWindowText(tmpStr, 256);

	WCHAR* pChar = wcschr(tmpStr, L'\n');
	if (pChar)
		*(pChar++) = L'\0';

	DrawCategory(dc, rect, tmpStr, pChar, IsCtrlThemed());

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}
