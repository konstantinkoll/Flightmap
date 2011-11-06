
// CDialogMenuBar.cpp: Implementierung der Klasse CDialogMenuBar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CDialogMenuBar
//

#define BORDERLEFT      16
#define BORDER          3

CDialogMenuBar::CDialogMenuBar()
	: CWnd()
{
	p_App = (FMApplication*)AfxGetApp();
	hTheme = NULL;
}

BOOL CDialogMenuBar::Create(CWnd* pParentWnd, UINT ResID, UINT nID)
{
	Icons.SetImageSize(CSize(16, 16));
	if (ResID)
		Icons.Load(ResID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CDialogMenuBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return (BOOL)GetOwner()->SendMessage(WM_COMMAND, wParam, lParam);
}

UINT CDialogMenuBar::GetPreferredHeight()
{
	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);

	return 2*BORDER+max(16, abs(lf.lfHeight));
}

/*CTaskButton* CDialogMenuBar::AddButton(UINT nID, INT IconID, BOOL ForceIcon, BOOL AddRight)
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
	btn->Create(AddRight ? _T("") : Caption, Caption, Hint, &Icons,
		ForceIcon || AddRight || (((LFApplication*)AfxGetApp())->OSVersion<OS_Seven) ? IconID : -1,
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
}*/

void CDialogMenuBar::AdjustLayout()
{
	SetRedraw(FALSE);

	CRect rect;
	GetClientRect(rect);

/*	INT Row = BORDER-1;
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
	}*/

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CDialogMenuBar::SetTheme()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_MENU);
	}
}


BEGIN_MESSAGE_MAP(CDialogMenuBar, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

INT CDialogMenuBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	SetTheme();

	return 0;
}

void CDialogMenuBar::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CWnd::OnDestroy();


/*	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
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
	}*/
}

BOOL CDialogMenuBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CDialogMenuBar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	BOOL Themed = IsCtrlThemed();

	if (hTheme)
	{
		p_App->zDrawThemeBackground(hTheme, dc, MENU_BARBACKGROUND, ((CGlasWindow*)GetParent())->m_Active ? MB_ACTIVE : MB_INACTIVE, rect, rect);
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(Themed ? COLOR_BTNFACE : COLOR_MENUBAR));
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

}

LRESULT CDialogMenuBar::OnThemeChanged()
{
	SetTheme();
	AdjustLayout();

	return TRUE;
}

void CDialogMenuBar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDialogMenuBar::OnIdleUpdateCmdUI()
{
/*	BOOL Update = FALSE;

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
		AdjustLayout();*/
}

void CDialogMenuBar::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
/*	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			CString tmpStr;
			btn->GetWindowText(tmpStr);
			menu.AppendMenu(0, btn->GetDlgCtrlID(), _T("&")+tmpStr);
		}
	}

	if (menu.GetMenuItemCount())
		menu.AppendMenu(MF_SEPARATOR);

	for (POSITION p=m_ButtonsRight.GetTailPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetPrev(p);
		if (btn->IsWindowEnabled())
		{
			CString tmpStr;
			btn->GetWindowText(tmpStr);
			menu.AppendMenu(0, btn->GetDlgCtrlID(), _T("&")+tmpStr);
		}
	}

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this, NULL);*/
}

void CDialogMenuBar::OnSetFocus(CWnd* /*pOldWnd*/)
{
/*	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			btn->SetFocus();
			break;
		}
	}*/
}
