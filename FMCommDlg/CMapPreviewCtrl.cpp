
// CMapPreviewCtrl.cpp: Implementierung der Klasse CMapPreviewCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// CMapPreviewCtrl
//

CMapPreviewCtrl::CMapPreviewCtrl()
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
	wndcls.lpszClassName = L"CMapPreviewCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CMapPreviewCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Airport = NULL;
	m_Location.Latitude = 0;
	m_Location.Longitude = 0;
}

CMapPreviewCtrl::~CMapPreviewCtrl()
{
	if (hMap)
		DeleteObject(hMap);
}

void CMapPreviewCtrl::Update(FMAirport* pAirport)
{
	if (pAirport!=p_Airport)
	{
		p_Airport = pAirport;

		CRect rect;
		GetClientRect(rect);

		if (hMap)
			DeleteObject(hMap);

		hMap = FMIATACreateAirportMap(pAirport, rect.Width(), rect.Height());
		Invalidate();
	}
}


BEGIN_MESSAGE_MAP(CMapPreviewCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CMapPreviewCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMapPreviewCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

void CMapPreviewCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hMap);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(hOldBitmap);
}
