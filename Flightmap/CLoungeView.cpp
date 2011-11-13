
// CLoungeView.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "CLoungeView.h"
#include "Flightmap.h"
#include "Resource.h"


// CLoungeView
//

CLoungeView::CLoungeView()
	: CWnd()
{
	hBackgroundBrush = NULL;
	m_pBackdrop = m_pLogo = NULL;
	m_BackBufferL = m_BackBufferH = 0;
}

BOOL CLoungeView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CLoungeView::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	BOOL Themed = IsCtrlThemed();
	if (Themed && !theApp.m_ReduceVisuals)
	{
		INT l = m_pBackdrop->m_pBitmap->GetWidth();
		INT h = m_pBackdrop->m_pBitmap->GetHeight();

		DOUBLE f = max((DOUBLE)rect.Width()/l, (DOUBLE)rect.Height()/h);
		l = max(rect.Width(), (INT)(l*f));
		h = max(rect.Height(), (INT)(h*f));

		g.DrawImage(m_pBackdrop->m_pBitmap, 0, rect.Height()-h, l, h);

		SolidBrush brush(Color(0x14, 0x00, 0x00, 0x00));
		for (INT a=0; a<5; a++)
			g.FillRectangle(&brush, 0, 0, rect.Width(), a+1);

		l = m_pLogo->m_pBitmap->GetWidth();
		h = m_pLogo->m_pBitmap->GetHeight();
		if ((rect.Width()>=l+24) && (rect.Height()>=h+24))
			g.DrawImage(m_pLogo->m_pBitmap, rect.Width()-l-10, rect.Height()-h-6, l, h);
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

		if ((theApp.OSVersion==OS_XP) || (!Themed))
			dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, GetSysColor(COLOR_SCROLLBAR));
	}
}


BEGIN_MESSAGE_MAP(CLoungeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

INT CLoungeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_pBackdrop = new CGdiPlusBitmapResource(IDB_DOCKED, _T("JPG"));
	m_pLogo = new CGdiPlusBitmapResource(IDB_FLIGHTMAP, _T("PNG"));

	return 0;
}

void CLoungeView::OnDestroy()
{
	if (m_pBackdrop)
		delete m_pBackdrop;
	if (m_pLogo)
		delete m_pLogo;
	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CWnd::OnDestroy();
}

BOOL CLoungeView::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap* pOldBitmap;
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&m_BackBuffer);

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);

		OnEraseBkgnd(dc, g, rect);

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void CLoungeView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_BackBufferL = m_BackBufferH = 0;
	Invalidate();
}

LRESULT CLoungeView::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;
	return TRUE;
}

void CLoungeView::OnSysColorChange()
{
	if (!IsCtrlThemed())
		m_BackBufferL = m_BackBufferH = 0;
}

HBRUSH CLoungeView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);

		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBrushOrg(-rc.left, -rc.top);

		hbr = hBackgroundBrush;
	}

	return hbr;
}

void CLoungeView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	CDialogMenuPopup* pPopup = new CDialogMenuPopup();
	pPopup->Create(this, IDB_MENUFILE_32, IDB_MENUFILE_16);

	pPopup->Track(pos);
}
