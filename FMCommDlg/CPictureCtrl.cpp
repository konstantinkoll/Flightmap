
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
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CPictureCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CPictureCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_DisplayMode = PC_COLOR;
	m_DisplayColor = 0x000000;
	p_Picture = NULL;
	m_Hover = FALSE;
}

BOOL CPictureCtrl::Create(CWnd* pParentWnd, const CRect& rect, UINT nID, UINT nPictureID, UINT nTooltipID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	if (CWnd::Create(className, _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, rect, pParentWnd, nID))
	{
		SetPicture(nPictureID, nTooltipID);

		return TRUE;
	}

	return FALSE;
}

BOOL CPictureCtrl::PreTranslateMessage(MSG* pMsg)
{
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
		FMGetApp()->HideTooltip();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CPictureCtrl::SetPicture(Bitmap* pPicture, const CString& Caption, const CString& Hint, BOOL ScaleToFit)
{
	m_DisplayMode = ScaleToFit ? PC_PICTURE_SCALETOFIT : PC_PICTURE_NORMAL;
	p_Picture = pPicture;

	m_Caption = Caption;
	m_Hint = Hint;

	if (m_Hover)
	{
		FMGetApp()->HideTooltip();
		m_Hover = FALSE;
	}

	Invalidate();
}

void CPictureCtrl::SetPicture(UINT nPictureID, const CString& Caption, const CString& Hint, BOOL ScaleToFit)
{
	SetPicture(FMGetApp()->GetCachedResourceImage(nPictureID), Caption, Hint, ScaleToFit);
}

void CPictureCtrl::SetPicture(UINT nPictureID, UINT nTooltipID, BOOL ScaleToFit)
{
	CString Caption;
	CString Hint;

	if (nTooltipID)
	{
		ENSURE(Caption.LoadString(nTooltipID));

		INT Pos = Caption.Find(L'\n');
		if (Pos!=-1)
		{
			Hint = Caption.Mid(Pos+1);
			Caption = Caption.Left(Pos);
		}
	}

	SetPicture(nPictureID, Caption, Hint, ScaleToFit);
}

void CPictureCtrl::SetColor(COLORREF clr, const CString& Caption, const CString& Hint)
{
	m_DisplayMode = PC_COLOR;
	m_DisplayColor = clr;
	p_Picture = NULL;

	m_Caption = Caption;
	m_Hint = Hint;

	if (m_Hover)
	{
		FMGetApp()->HideTooltip();
		m_Hover = FALSE;
	}

	Invalidate();
}

void CPictureCtrl::SetColor(COLORREF clr, UINT nTooltipID)
{
	CString Caption;
	CString Hint;

	if (nTooltipID)
	{
		ENSURE(Caption.LoadString(nTooltipID));

		INT Pos = Caption.Find(L'\n');
		if (Pos!=-1)
		{
			Hint = Caption.Mid(Pos+1);
			Caption = Caption.Left(Pos);
		}
	}

	SetColor(clr, Caption, Hint);
}


BEGIN_MESSAGE_MAP(CPictureCtrl, CWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

void CPictureCtrl::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	lpncsp->rgrc[0].top += 2;
	lpncsp->rgrc[0].left += 2;
	lpncsp->rgrc[0].bottom -= 2;
	lpncsp->rgrc[0].right -= 2;
}

void CPictureCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

BOOL CPictureCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPictureCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	if ((m_DisplayMode==PC_COLOR) || !p_Picture)
	{
		dc.FillSolidRect(rect, m_DisplayColor);
	}
	else
	{
		Graphics g(dc);

		const INT Width = p_Picture->GetWidth();
		const INT Height = p_Picture->GetHeight();

		if (m_DisplayMode==PC_PICTURE_SCALETOFIT)
		{
			g.DrawImage(p_Picture, 0, 0, rect.Width(), rect.Height());
		}
		else
			if ((Width>=rect.Width()) && (Height>=rect.Height()))
			{
				g.DrawImage(p_Picture, -(Width-rect.Width())/2, -(Height-rect.Height())/2);
			}
			else
			{
				const DOUBLE Scale = max((DOUBLE)rect.Width()/Width, (DOUBLE)rect.Height()/Height);
				g.DrawImage(p_Picture, 0, 0, (INT)(Width*Scale), (INT)(Height*Scale));
			}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}


void CPictureCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CPictureCtrl::OnMouseLeave()
{
	FMGetApp()->HideTooltip();
	m_Hover = FALSE;

	Invalidate();
}

void CPictureCtrl::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!FMGetApp()->IsTooltipVisible())
			if (!m_Caption.IsEmpty() || !m_Hint.IsEmpty())
				FMGetApp()->ShowTooltip(this, point, m_Caption, m_Hint);
	}
	else
	{
		FMGetApp()->HideTooltip();
	}
}
