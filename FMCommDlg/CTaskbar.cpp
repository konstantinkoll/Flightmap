
// CTaskbar.cpp: Implementierung der Klasse CTaskbar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CTaskbar
//

#define BORDERLEFT      16
#define BORDER          4

CTaskbar::CTaskbar()
	: CWnd()
{
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
}

BOOL CTaskbar::Create(CWnd* pParentWnd, UINT ResID, UINT nID)
{
	m_Icons.SetImageSize(CSize(16, 16));
	if (ResID)
		m_Icons.Load(ResID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CTaskbar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return (BOOL)GetOwner()->SendMessage(WM_COMMAND, wParam, lParam);
}

UINT CTaskbar::GetPreferredHeight()
{
	LOGFONT lf;
	UINT h = 4*BORDER+(IsCtrlThemed() ? 4 : 3);

	((FMApplication*)AfxGetApp())->m_DefaultFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	return h;
}

CTaskButton* CTaskbar::AddButton(UINT nID, INT IconID, BOOL ForceIcon, BOOL AddRight)
{
	CString Caption;
	CString Hint;
	ENSURE(Caption.LoadString(nID));

	INT pos = Caption.Find(L'\n');
	if (pos!=-1)
	{
		Hint = Caption.Left(pos);
		Caption.Delete(0, pos+1);

		if (Hint.GetLength()>40)
		{
			pos = Hint.Find(L' ', Hint.GetLength()/2);
			if (pos!=-1)
				Hint.SetAt(pos, L'\n');
		}
	}

	CTaskButton* btn = new CTaskButton();
	btn->Create(AddRight ? _T("") : Caption, Caption, Hint, &m_Icons,
		ForceIcon || AddRight || (((FMApplication*)AfxGetApp())->OSVersion<OS_Seven) ? IconID : -1,
		this, nID);
	btn->EnableWindow(FALSE);

	if (AddRight)
	{
		m_ButtonsRight.AddHead(btn);
	}
	else
	{
		m_ButtonsLeft.AddTail(btn);
	}

	return btn;
}

void CTaskbar::AdjustLayout()
{
	SetRedraw(FALSE);

	CRect rect;
	GetClientRect(rect);

	INT Row = BORDER-1;
	INT h = rect.Height()-2*BORDER+(IsCtrlThemed() ? 1 : 2);

	INT RPos = rect.right+2*BORDER-BORDERLEFT;
	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			INT l = btn->GetPreferredWidth();
			RPos -= l+BORDER;
			if (RPos>=BORDERLEFT)
			{
				btn->SetWindowPos(NULL, RPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				btn->ShowWindow(SW_SHOW);
			}
			else
			{
				btn->ShowWindow(SW_HIDE);
			}
		}
		else
		{
			btn->ShowWindow(SW_HIDE);
		}
	}

	INT LPos = rect.left+BORDERLEFT-BORDER;
	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			INT l = btn->GetPreferredWidth();
			if (LPos+l+BORDERLEFT-BORDER<RPos)
			{
				btn->SetWindowPos(NULL, LPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				btn->ShowWindow(SW_SHOW);
			}
			else
			{
				btn->ShowWindow(SW_HIDE);
			}
			LPos += l+BORDERLEFT;
		}
		else
		{
			btn->ShowWindow(SW_HIDE);
		}
	}

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}


BEGIN_MESSAGE_MAP(CTaskbar, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_THEMECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CTaskbar::OnDestroy()
{
	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CWnd::OnDestroy();
}

BOOL CTaskbar::OnEraseBkgnd(CDC* pDC)
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

		if (!IsCtrlThemed())
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}
		else
			switch (((FMApplication*)AfxGetApp())->OSVersion)
			{
			case OS_Vista:
				{
					Color c1(0x04, 0x48, 0x75);
					Color c2(0x19, 0x6C, 0x77);
					Color c3(0x80, 0xC0, 0xFF, 0xE0);

					LinearGradientBrush brush1(Point(0, 0), Point(rect.right, 0), c1, c2);
					g.FillRectangle(&brush1, 0, 0, rect.right, rect.bottom);

					SolidBrush brush2(Color(0x60, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush2, 0, rect.bottom-1, rect.right, 1);

					UINT line = rect.Height()/2;

					LinearGradientBrush brush3(Point(0, 0), Point(0, line), Color(144, 0xFF, 0xFF, 0xFF), Color(48, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush3, 0, 0, rect.right, line);

					LinearGradientBrush brush4(Point(0, rect.bottom-line*2/3), Point(0, rect.bottom-1), Color(0, 0xFF, 0xFF, 0xFF), c3);
					g.FillRectangle(&brush4, 0, rect.bottom-line*2/3+1, rect.right, rect.bottom-line-2);

					SolidBrush brush5(Color(64, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush5, 0, 0, rect.right, 1);
					g.FillRectangle(&brush5, 0, rect.bottom-2, rect.right, 1);
					g.FillRectangle(&brush5, 0, 0, 1, rect.bottom-1);
					g.FillRectangle(&brush5, rect.right-1, 0, 1, rect.bottom-1);

					break;
				}
			case OS_XP:
			case OS_Seven:
			case OS_Eight:
				{
					UINT line = (rect.Height()-2)/2;

					LinearGradientBrush brush1(Point(0, 0), Point(0, line+1), Color(0xFD, 0xFE, 0xFF), Color(0xE6, 0xF0, 0xFA));
					g.FillRectangle(&brush1, 1, 0, rect.right-2, line+1);

					LinearGradientBrush brush2(Point(0, line+2), Point(0, rect.bottom-4), Color(0xDC, 0xE6, 0xF4), Color(0xDD, 0xE9, 0xF7));
					g.FillRectangle(&brush2, 1, line+1, rect.right-2, rect.bottom-line-4);

					LinearGradientBrush brush3(Point(0, 1), Point(0, rect.bottom-4), Color(0xFF, 0xFF, 0xFF), Color(0xEE, 0xF4, 0xFB));
					g.FillRectangle(&brush3, 0, 0, 1, rect.bottom-3);
					g.FillRectangle(&brush3, rect.right-1, 0, 1, rect.bottom-3);

					dc.FillSolidRect(0, rect.bottom-3, rect.right, 1, 0xFBEFE4);
					dc.FillSolidRect(0, rect.bottom-2, rect.right, 1, 0xEADACD);
					dc.FillSolidRect(0, rect.bottom-1, rect.right, 1, 0xC3AFA0);
					break;
				}
			}

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	dc.SelectObject(pOldBitmap);
	return TRUE;
}

void CTaskbar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	FillRect(pDC, rect, hBackgroundBrush);
}

void CTaskbar::OnSysColorChange()
{
	m_BackBufferL = m_BackBufferH = 0;
}

LRESULT CTaskbar::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;
	AdjustLayout();

	return TRUE;
}

HBRUSH CTaskbar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CTaskbar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CTaskbar::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		BOOL Enabled = btn->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = btn->GetDlgCtrlID();
		cmdUI.m_pOther = btn;
		cmdUI.DoUpdate(GetOwner(), TRUE);

		if (btn->IsWindowEnabled()!=Enabled)
			Update = TRUE;
	}

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		BOOL Enabled = btn->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = btn->GetDlgCtrlID();
		cmdUI.m_pOther = btn;
		cmdUI.DoUpdate(GetOwner(), TRUE);

		if (btn->IsWindowEnabled()!=Enabled)
			Update = TRUE;
	}

	if (Update)
		AdjustLayout();
}

void CTaskbar::OnSetFocus(CWnd* /*pOldWnd*/)
{
	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			btn->SetFocus();
			break;
		}
	}
}