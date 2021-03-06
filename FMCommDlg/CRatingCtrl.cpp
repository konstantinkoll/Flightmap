
// CRatingCtrl.cpp: Implementierung der Klasse CRatingCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CRatingCtrl
//

CRatingCtrl::CRatingCtrl()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CRatingCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CRatingCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Rating
	m_Rating = 0;
}

void CRatingCtrl::SetInitialRating(UCHAR Rating)
{
	ASSERT(Rating<=MAXRATING);

	// Resize window
	CRect rect;
	GetWindowRect(rect);
	GetParent()->ScreenToClient(rect);

	if (rect.Width()<RATINGBITMAPWIDTH+8)
		SetWindowPos(NULL, rect.left, rect.top, max(rect.Height(), RATINGBITMAPWIDTH+8), max(rect.Height(), RATINGBITMAPHEIGHT+4), SWP_NOACTIVATE | SWP_NOZORDER);

	// Set rating
	m_Rating = Rating;

	Invalidate();
}


BEGIN_MESSAGE_MAP(CRatingCtrl, CFrontstageWnd)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

void CRatingCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// Background
	dc.FillSolidRect(rect, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	// Bitmap
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(FMGetApp()->hRatingBitmaps[m_Rating]);

	dc.AlphaBlend((rect.Width()-RATINGBITMAPWIDTH)/2, (rect.Height()-RATINGBITMAPHEIGHT)/2, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);

	SelectObject(dcMem, hOldBitmap);

	// Focus rectangle
	if (GetFocus()==this)
	{
		dc.SetBkColor(0x000000);
		dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.DrawFocusRect(rect);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CRatingCtrl::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	INT Rating = m_Rating;

	switch (nChar)
	{
	case 0x27:
	case 0x6B:
	case 0xBB:
		if (++Rating>MAXRATING)
			Rating = MAXRATING;

		break;

	case 0x25:
	case 0x6D:
	case 0xBD:
		if (--Rating<0)
			Rating = 0;

		break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		Rating = (nChar-'0')*2;
		break;

	default:
		return;
	}

	if (m_Rating!=(UCHAR)Rating)
		SetRating((UCHAR)Rating);
}

void CRatingCtrl::OnLButtonDown(UINT /*Flags*/, CPoint point)
{
	if ((point.x>=0) && (point.x<RATINGBITMAPWIDTH+2))
		if ((point.x<6) || ((point.x-2)%18<16))
			SetRating((UCHAR)((point.x<6) ? 0 : 2*((point.x-2)/18)+((point.x-2)%18>8)+1));

	if (GetFocus()!=this)
		SetFocus();
}

void CRatingCtrl::OnRButtonDown(UINT /*Flags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

BOOL CRatingCtrl::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	CRect rect;
	GetClientRect(rect);

	SetCursor(FMGetApp()->LoadStandardCursor((point.x<0) || (point.y<0) || (point.y>=rect.Height()) ? IDC_ARROW : point.x<6 ? IDC_HAND : ((point.x<RATINGBITMAPWIDTH+2) && ((point.x-2)%18<16)) ? IDC_HAND : IDC_ARROW));

	return TRUE;
}

void CRatingCtrl::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CRatingCtrl::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

UINT CRatingCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}
