
// CPictureCtrl.cpp: Implementierung der Klasse CPictureCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CPictureCtrl
//

CPictureCtrl::CPictureCtrl()
	: CFrontstageWnd()
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
}

BOOL CPictureCtrl::Create(CWnd* pParentWnd, const CRect& rect, UINT nID, UINT nPictureID, UINT nTooltipID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	if (CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, rect, pParentWnd, nID))
	{
		SetPicture(nPictureID, nTooltipID);

		return TRUE;
	}

	return FALSE;
}

void CPictureCtrl::SetPicture(Bitmap* pPicture, const CString& Caption, const CString& Hint, BOOL ScaleToFit)
{
	m_DisplayMode = ScaleToFit ? PC_PICTURE_SCALETOFIT : PC_PICTURE_NORMAL;
	p_Picture = pPicture;

	m_Caption = Caption;
	m_Hint = Hint;

	HideTooltip();
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

	HideTooltip();
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

INT CPictureCtrl::ItemAtPosition(CPoint /*point*/) const
{
	return 0;
}

void CPictureCtrl::ShowTooltip(const CPoint& point)
{
	if (!m_Caption.IsEmpty() || !m_Hint.IsEmpty())
		FMGetApp()->ShowTooltip(this, point, m_Caption, m_Hint);
}


BEGIN_MESSAGE_MAP(CPictureCtrl, CFrontstageWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
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
