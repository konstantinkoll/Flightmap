
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

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS)-sizeof(ncm.iPaddedBorderWidth); 
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		m_MenuLogFont = ncm.lfMenuFont;
		m_MenuHeight = max(ncm.iMenuHeight, 2*BORDER+16);
	}
	else
	{
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(m_MenuLogFont), &m_MenuLogFont);
		m_MenuHeight = 2*BORDER+max(16, abs(m_MenuLogFont.lfHeight));
	}

	m_MenuFont.CreateFontIndirect(&m_MenuLogFont);
}

BOOL CDialogMenuBar::Create(CWnd* pParentWnd, UINT ResID, UINT nID)
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

BOOL CDialogMenuBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return (BOOL)GetOwner()->SendMessage(WM_COMMAND, wParam, lParam);
}

UINT CDialogMenuBar::GetPreferredHeight()
{
	return m_MenuHeight;
}

void CDialogMenuBar::AddMenuLeft(UINT nID, UINT nCaptionResID)
{
	MenuBarItem i;
	ZeroMemory(&i, sizeof(i));
	i.PopupID = nID;
	ENSURE(LoadString(AfxGetResourceHandle(), nCaptionResID, i.Name, 256));

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_MenuFont);
	i.MinWidth = pDC->GetTextExtent(i.Name, wcslen(i.Name)).cx;
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	m_Items.AddItem(i);
}

void CDialogMenuBar::AddMenuRight(UINT nCmdID, INT nIconID)
{
	MenuBarItem i;
	ZeroMemory(&i, sizeof(i));
	i.CmdID = nCmdID;
	i.IconID = nIconID;
	i.MinWidth = 16;

	m_Items.AddItem(i);
}

void CDialogMenuBar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT Left = 0;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		m_Items.m_Items[a].Left = Left;
		m_Items.m_Items[a].Right = Left+m_Items.m_Items[a].MinWidth+10;
		Left = m_Items.m_Items[a].Right;
	}

	Invalidate();
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

	// Background
	BOOL Themed = IsCtrlThemed();

	if (hTheme)
	{
		p_App->zDrawThemeBackground(hTheme, dc, MENU_BARBACKGROUND, ((CGlasWindow*)GetParent())->m_Active ? MB_ACTIVE : MB_INACTIVE, rect, rect);
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(Themed ? COLOR_BTNFACE : COLOR_MENUBAR));
	}

	// Items
	CFont* pOldFont = dc.SelectObject(&m_MenuFont);

	UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	if (GetFocus()!=this)
	{
		BOOL AlwaysUnderline = FALSE;
		if (SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &AlwaysUnderline, 0))
			if (!AlwaysUnderline)
				format |= DT_HIDEPREFIX;
	}

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CRect rectItem(m_Items.m_Items[a].Left, rect.top, m_Items.m_Items[a].Right, rect.bottom);

		if (m_Items.m_Items[a].CmdID)
		{
			CAfxDrawState ds;
			m_Icons.PrepareDrawImage(ds);
			m_Icons.Draw(&dc, rectItem.left, (rectItem.Height()-16)/2, m_Items.m_Items[a].IconID);
			m_Icons.EndDrawImage(ds);
		}
		else
		{
			dc.DrawText(m_Items.m_Items[a].Name, -1, rectItem, format);
		}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
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
