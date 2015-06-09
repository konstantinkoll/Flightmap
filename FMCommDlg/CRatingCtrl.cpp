
// CRatingCtrl.cpp: Implementierung der Klasse CRatingCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CRatingCtrl
//

#define PrepareBlend()                      INT w = min(rect.Width(), RatingBitmapWidth); \
                                            INT h = min(rect.Height(), RatingBitmapHeight);
#define Blend(dc, rect, level, bitmaps)     { HDC hdcMem = CreateCompatibleDC(dc); \
                                            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmaps[level>MaxRating ? 0 : level]); \
                                            AlphaBlend(dc, rect.left+(rect.Width()-w)/2, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, BF); \
                                            SelectObject(hdcMem, hbmOld); \
                                            DeleteDC(hdcMem); }

static const BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

CRatingCtrl::CRatingCtrl()
	: CWnd()
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
	wndcls.lpszClassName = L"CRatingCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CRatingCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_Rating = 0;
}

void CRatingCtrl::SetRating(UCHAR Rating, BOOL Prepare)
{
	ASSERT(Rating<=MaxRating);

	if (Prepare)
	{
		CRect rect;
		GetWindowRect(&rect);
		GetParent()->ScreenToClient(&rect);

		if (rect.Width()<RatingBitmapWidth+8)
			SetWindowPos(NULL, rect.left, rect.top, max(rect.Height(), RatingBitmapWidth+8), max(rect.Height(), RatingBitmapHeight+4), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	m_Rating = Rating;
	Invalidate();
}

UCHAR CRatingCtrl::GetRating()
{
	return m_Rating;
}

void CRatingCtrl::SendChangeMessage()
{
	GetOwner()->SendMessage(WM_RATINGCHANGED);
}


BEGIN_MESSAGE_MAP(CRatingCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

BOOL CRatingCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CRatingCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

void CRatingCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	dc.FillSolidRect(rect, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	PrepareBlend();
	Blend(dc, rect, m_Rating, FMGetApp()->m_RatingBitmaps);

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
		Rating++;
		break;
	case 0x25:
	case 0x6D:
	case 0xBD:
		Rating--;
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

	if (Rating<0)
		Rating = 0;
	if (Rating>MaxRating)
		Rating = MaxRating;

	if (m_Rating!=(UCHAR)Rating)
	{
		m_Rating = (UCHAR)Rating;
		Invalidate();

		SendChangeMessage();
	}
}

void CRatingCtrl::OnLButtonDown(UINT /*Flags*/, CPoint point)
{
	if ((point.x>=0) && (point.x<RatingBitmapWidth+2))
		if ((point.x<6) || ((point.x-2)%18<16))
		{
			m_Rating = (UCHAR)((point.x<6) ? 0 : 2*((point.x-2)/18)+((point.x-2)%18>8)+1);
			Invalidate();

			SendChangeMessage();
		}

	SetFocus();
}

BOOL CRatingCtrl::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*Message*/)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	CRect rect;
	GetClientRect(rect);

	SetCursor(AfxGetApp()->LoadStandardCursor((point.x<0) || (point.y<0) || (point.y>=rect.Height()) ? IDC_ARROW : point.x<6 ? IDC_HAND : ((point.x<RatingBitmapWidth+2) && ((point.x-2)%18<16)) ? IDC_HAND : IDC_ARROW));
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
	return DLGC_WANTCHARS;
}
