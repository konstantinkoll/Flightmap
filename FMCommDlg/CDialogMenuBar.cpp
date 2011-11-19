
// CDialogMenuBar.cpp: Implementierung der Klasse CDialogMenuBar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CDialogMenuBar
//

#define BORDER          3

CDialogMenuBar::CDialogMenuBar()
	: CWnd()
{
	p_App = (FMApplication*)AfxGetApp();
	hTheme = NULL;
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

INT CDialogMenuBar::GetMinWidth()
{
	INT Spacer = (p_App->OSVersion==OS_XP) ? 10 : 6;
	INT MinWidth = 0;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		MinWidth += m_Items.m_Items[a].MinWidth+Spacer;

	return MinWidth;
}

void CDialogMenuBar::AddMenuLeft(UINT nID, UINT nCaptionResID)
{
	MenuBarItem i;
	ZeroMemory(&i, sizeof(i));
	i.PopupID = nID;
	ENSURE(LoadString(AfxGetResourceHandle(), nCaptionResID, i.Name, 256));

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_MenuFont);
	i.MinWidth = pDC->GetTextExtent(i.Name, (INT)wcslen(i.Name)).cx;
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
	INT Spacer = (p_App->OSVersion==OS_XP) ? 10 : 6;
	BOOL OnLeftSide = TRUE;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		if ((m_Items.m_Items[a].CmdID) && OnLeftSide)
		{
			OnLeftSide = FALSE;
			Left = rect.Width()-(m_Items.m_ItemCount-a)*(m_Items.m_Items[a].MinWidth+Spacer);
		}

		m_Items.m_Items[a].Left = Left;
		m_Items.m_Items[a].Right = Left+m_Items.m_Items[a].MinWidth+Spacer;
		Left = m_Items.m_Items[a].Right;
	}

	Invalidate();
}

void CDialogMenuBar::SetTheme()
{
	// Themes
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_MENU);
	}

	// Default font
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS)-sizeof(ncm.iPaddedBorderWidth); 
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		m_MenuLogFont = m_NormalLogFont = ncm.lfMenuFont;
		m_MenuHeight = max(ncm.iMenuHeight, 2*BORDER+16);
	}
	else
	{
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(m_MenuLogFont), &m_MenuLogFont);
		m_NormalLogFont = m_MenuLogFont;
		m_MenuHeight = 2*BORDER+max(16, abs(m_MenuLogFont.lfHeight));
	}

	m_MenuFont.DeleteObject();
	m_MenuFont.CreateFontIndirect(&m_MenuLogFont);

	// Special fonts
	m_NormalLogFont.lfHeight = -max(abs(m_NormalLogFont.lfHeight), 11);
	wcscpy_s(m_NormalLogFont.lfFaceName, 32, p_App->GetDefaultFontFace());
	m_NormalFont.DeleteObject();
	m_NormalFont.CreateFontIndirect(&m_NormalLogFont);

	m_CaptionLogFont = m_NormalLogFont;
	m_CaptionLogFont.lfWeight = FW_BOLD;
	m_CaptionFont.DeleteObject();
	m_CaptionFont.CreateFontIndirect(&m_CaptionLogFont);
}


BEGIN_MESSAGE_MAP(CDialogMenuBar, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
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
		p_App->zDrawThemeBackground(hTheme, dc, MENU_BARBACKGROUND, ((CMainWindow*)GetParent())->m_Active ? MB_ACTIVE : MB_INACTIVE, rect, rect);
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_MENUBAR));
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
		COLORREF clrText = GetSysColor(((CMainWindow*)GetParent())->m_Active ? COLOR_MENUTEXT : COLOR_3DSHADOW);

		if (hTheme)
		{
			p_App->zDrawThemeBackground(hTheme, dc, MENU_BARITEM, MBI_PUSHED, rectItem, rectItem);
		}
		else
			if (Themed)
			{
				dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
				clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
			}
			else
			{

			}

		if (m_Items.m_Items[a].CmdID)
		{
			CAfxDrawState ds;
			m_Icons.PrepareDrawImage(ds);
			m_Icons.Draw(&dc, rectItem.left+(rectItem.Width()-16)/2, rectItem.top+(rectItem.Height()-16)/2, m_Items.m_Items[a].IconID);
			m_Icons.EndDrawImage(ds);
		}
		else
		{
			if (p_App->OSVersion==OS_XP)
				rectItem.bottom -= 2;

			dc.SetTextColor(clrText);
			dc.DrawText(m_Items.m_Items[a].Name, -1, rectItem, format);
		}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
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


// CDialogMenuPopup
//

#define BORDERPOPUP     2

CDialogMenuPopup::CDialogMenuPopup()
	: CWnd()
{
}

BOOL CDialogMenuPopup::Create(CWnd* pParentWnd, UINT LargeIconsID, UINT SmallIconsID)
{
	// Load icons
	m_LargeIconsID = LargeIconsID;
	m_LargeIcons.SetImageSize(CSize(32, 32));
	if (LargeIconsID)
		m_LargeIcons.Load(LargeIconsID);

	m_SmallIconsID = SmallIconsID;
	m_SmallIcons.SetImageSize(CSize(16, 16));
	if (SmallIconsID)
		m_SmallIcons.Load(SmallIconsID);

	// Create
	UINT nClassStyle = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	CMainWindow* pTopLevelParent = (CMainWindow*)pParentWnd->GetTopLevelParent();
	pTopLevelParent->RegisterPopupWindow(this);

	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
	if (bDropShadow)
		nClassStyle |= CS_DROPSHADOW;

	CString className = AfxRegisterWndClass(nClassStyle, LoadCursor(NULL, IDC_ARROW));
	BOOL res = CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_BORDER | WS_VISIBLE | WS_POPUP, 0, 0, 100, 100, pParentWnd->GetSafeHwnd(), NULL);

	SetOwner(pTopLevelParent);

	return res;
}

void CDialogMenuPopup::AddItem(CDialogMenuItem* pItem)
{
	ASSERT(pItem);

	MenuPopupItem i;
	ZeroMemory(&i, sizeof(i));
	i.pItem = pItem;
	i.Enabled = pItem->IsEnabled();

	m_Items.AddItem(i);
}

void CDialogMenuPopup::AddCommand(UINT CmdID, INT IconID, UINT PreferredSize)
{
	AddItem(new CDialogMenuCommand(this, CmdID, IconID, PreferredSize));
}

void CDialogMenuPopup::Track(CPoint pt)
{
}

BOOL CDialogMenuPopup::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			GetOwner()->PostMessage(WM_CLOSEPOPUP);
			return TRUE;
		}

	return CWnd::PreTranslateMessage(pMsg);
}

void CDialogMenuPopup::AdjustLayout()
{
/*	if (!IsWindow(m_wndList))
		return;

	CRect rect;
	GetClientRect(rect);

	if (IsWindow(m_wndBottomArea))
	{
		const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
		m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);
		rect.bottom -= BottomHeight;
	}

	m_wndList.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.EnsureVisible(0, FALSE);*/
}


BEGIN_MESSAGE_MAP(CDialogMenuPopup, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATEAPP()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CDialogMenuPopup::OnDestroy()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		delete m_Items.m_Items[a].pItem;

	CWnd::OnDestroy();
}

BOOL CDialogMenuPopup::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	FillRect(*pDC, rect, OnCtlColor(pDC, this, CTLCOLOR_STATIC));

	return TRUE;
}

void CDialogMenuPopup::OnNcPaint()
{
	CRect rect;
	GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(this);

	dc.FillSolidRect(rect, GetSysColor(COLOR_3DSHADOW));
}

void CDialogMenuPopup::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDialogMenuPopup::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
}

void CDialogMenuPopup::OnActivateApp(BOOL bActive, DWORD dwTask)
{
	CWnd::OnActivateApp(bActive, dwTask);

	if (!bActive)
		GetOwner()->PostMessage(WM_CLOSEPOPUP);
}

HBRUSH CDialogMenuPopup::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetDCBrushColor(IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_MENU));
		hbr = (HBRUSH)GetStockObject(DC_BRUSH);
	}

	return hbr;
}


// CDialogMenuItem
//

CDialogMenuItem::CDialogMenuItem(CDialogMenuPopup* pParentPopup)
{
	p_ParentPopup = pParentPopup;
}

INT CDialogMenuItem::GetMinHeight()
{
	return 0;
}

INT CDialogMenuItem::GetMinWidth()
{
	return 0;
}

INT CDialogMenuItem::GetMinGutter()
{
	return 0;
}

BOOL CDialogMenuItem::IsEnabled()
{
	return FALSE;
}

void CDialogMenuItem::OnPaint(CDC* /*pDC*/, LPRECT /*rect*/)
{
}

void CDialogMenuItem::OnSelect()
{
}

void CDialogMenuItem::OnDeselect()
{
}

void CDialogMenuItem::OnClick(CPoint /*point*/)
{
}


// CDialogMenuCommand
//

CDialogMenuCommand::CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize)
	: CDialogMenuItem(pParentPopup)
{
	m_CmdID = CmdID;
	m_IconID = IconID;
	m_PreferredSize = PreferredSize;
}

INT CDialogMenuCommand::GetMinGutter()
{
	return (m_IconID==-1) ? 0 : (m_PreferredSize==CDMB_SMALL) ? 16+BORDER : 32+BORDER;
}
