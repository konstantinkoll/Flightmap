
// FMDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FMDialog.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMDialog
//

FMDialog::FMDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;

	p_App = FMGetApp();
	hIconS = hIconL = NULL;
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
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
	if (Themed && p_App->m_UseBgImages)
	{
		CGdiPlusBitmap* pBackdrop = p_App->GetCachedResourceImage(IDB_BACKDROP, _T("PNG"));
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
			dc.FillSolidRect(0, Line++, m_BackBufferL, 1, p_App->OSVersion==OS_Eight ? 0xCCCCCC : 0xDFDFDF);
			dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, p_App->OSVersion==OS_Eight ? 0xF1F1F1 : 0xF0F0F0);
		}
		else
		{
			dc.FillSolidRect(0, Line++, m_BackBufferL, rect.Height()-Line, GetSysColor(COLOR_3DFACE));
		}
	}
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

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	hIconS = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SetIcon(hIconS, FALSE);
	hIconL = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	SetIcon(hIconL, TRUE);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMDialog::OnDestroy()
{
	if (hIconL)
		DestroyIcon(hIconL);
	if (hIconS)
		DestroyIcon(hIconS);
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
