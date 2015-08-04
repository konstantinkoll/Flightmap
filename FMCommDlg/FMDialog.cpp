
// FMDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMDialog
//

FMDialog::FMDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;

	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
	p_BottomLeftControl = NULL;

}

void FMDialog::DoDataExchange(CDataExchange* pDX)
{
	for (UINT a=0; a<4; a++)
	{
		CWnd* pWnd = GetDlgItem(IDC_GROUPBOX1+a);
		if (pWnd)
			DDX_Control(pDX, IDC_GROUPBOX1+a, m_GroupBox[a]);
	}
}

CWnd* FMDialog::GetBottomWnd() const
{
	CWnd* pBottomWnd = GetDlgItem(IDOK);
	if (!pBottomWnd)
		pBottomWnd = GetDlgItem(IDCANCEL);

	return pBottomWnd;
}

void FMDialog::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	CWnd* pBottomWnd = GetBottomWnd();
	if (!pBottomWnd)
	{
		rect.SetRectEmpty();
		return;
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	pBottomWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);

	CRect layout;
	GetLayoutRect(layout);
	INT Line = layout.bottom;

	BOOL Themed = IsCtrlThemed();
	if (Themed && FMGetApp()->m_UseBgImages)
	{
		CGdiPlusBitmap* pBackdrop = FMGetApp()->GetCachedResourceImage(IDB_BACKDROP, _T("PNG"));
		INT l = pBackdrop->m_pBitmap->GetWidth();
		INT h = pBackdrop->m_pBitmap->GetHeight();

		DOUBLE f = max((DOUBLE)rect.Width()/l, (DOUBLE)rect.Height()/h);
		l = (INT)(l*f);
		h = (INT)(h*f);

		g.DrawImage(pBackdrop->m_pBitmap, rect.Width()-l, rect.Height()-h, l, h);

		SolidBrush brush1(Color(168, 255, 255, 255));
		g.FillRectangle(&brush1, 0, 0, m_BackBufferL, --Line);
		brush1.SetColor(Color(224, 205, 250, 255));
		g.FillRectangle(&brush1, 0, Line++, m_BackBufferL, 1);
		brush1.SetColor(Color(180, 183, 210, 240));
		g.FillRectangle(&brush1, 0, Line++, m_BackBufferL, 1);
		brush1.SetColor(Color(224, 247, 250, 254));
		g.FillRectangle(&brush1, 0, Line, m_BackBufferL, 1);
		LinearGradientBrush brush2(Point(0, Line++), Point(0, rect.Height()), Color(120, 255, 255, 255), Color(32, 128, 192, 255));
		g.FillRectangle(&brush2, 0, Line, m_BackBufferL, rect.Height()-Line);
	}
	else
	{
		dc.FillSolidRect(0, 0, m_BackBufferL, Line, 0xFFFFFF);
		if (Themed)
		{
			dc.FillSolidRect(0, Line++, m_BackBufferL, 1, FMGetApp()->OSVersion==OS_Eight ? 0xCCCCCC : 0xDFDFDF);
			dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, FMGetApp()->OSVersion==OS_Eight ? 0xF1F1F1 : 0xF0F0F0);
		}
		else
		{
			dc.FillSolidRect(0, Line++, m_BackBufferL, rect.Height()-Line, GetSysColor(COLOR_3DFACE));
		}
	}
}

void FMDialog::SetBottomLeftControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	p_BottomLeftControl = pChildWnd;
}

void FMDialog::SetBottomLeftControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		SetBottomLeftControl(pChildWnd);
}

void FMDialog::AddBottomRightControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	m_BottomRightControls.AddTail(pChildWnd);
}

void FMDialog::AddBottomRightControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		AddBottomRightControl(pChildWnd);
}

void FMDialog::AdjustLayout()
{
}

void FMDialog::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CWnd* pBottomWnd = GetBottomWnd();
	if (!pBottomWnd)
		return;

	CRect btn;
	pBottomWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);
	lpRect->bottom = btn.top-borders.Height()-2;
}


BEGIN_MESSAGE_MAP(FMDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_REGISTERED_MESSAGE(FMGetApp()->m_UseBgImagesChangedMsg, OnUseBgImagesChanged)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL FMDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = FMGetApp()->LoadDialogIcon(m_nIDTemplate);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	CRect rect;
	GetClientRect(rect);
	m_LastSize = CPoint(rect.Width(), rect.Height());

	AddBottomRightControl(IDOK);
	AddBottomRightControl(IDCANCEL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMDialog::OnDestroy()
{
	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CDialog::OnDestroy();
}

BOOL FMDialog::OnEraseBkgnd(CDC* pDC)
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

void FMDialog::OnSize(UINT nType, INT cx, INT cy)
{
	CDialog::OnSize(nType, cx, cy);

	CPoint diff(cx-m_LastSize.x, cy-m_LastSize.y);
	m_LastSize.x = cx;
	m_LastSize.y = cy;

	INT MaxRight = cx;
	for (POSITION p=m_BottomRightControls.GetHeadPosition(); p; )
	{
		CWnd* pWnd = m_BottomRightControls.GetNext(p);

		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);

		pWnd->SetWindowPos(NULL, rect.left+diff.x, rect.top+diff.y, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);

		MaxRight = min(MaxRight, rect.left+diff.x);
	}

	if (p_BottomLeftControl)
	{
		CRect rect;
		p_BottomLeftControl->GetWindowRect(&rect);
		ScreenToClient(&rect);

		p_BottomLeftControl->SetWindowPos(NULL, rect.left, rect.top+diff.y, MaxRight-rect.left, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	AdjustLayout();

	m_BackBufferL = m_BackBufferH = 0;
	Invalidate();
}

LRESULT FMDialog::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;

	return TRUE;
}

void FMDialog::OnSysColorChange()
{
	if (!IsCtrlThemed())
		m_BackBufferL = m_BackBufferH = 0;
}

LRESULT FMDialog::OnUseBgImagesChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	FMGetApp()->m_UseBgImages = (BOOL)wParam;

	if (IsCtrlThemed())
	{
		m_BackBufferL = m_BackBufferH = 0;
		Invalidate();
	}

	return NULL;
}

HBRUSH FMDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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
