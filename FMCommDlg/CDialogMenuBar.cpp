
// CDialogMenuBar.cpp: Implementierung der Klasse CDialogMenuBar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CDialogCmdUI
//

CDialogCmdUI::CDialogCmdUI()
	: CCmdUI()
{
	m_Enabled = FALSE;
}

void CDialogCmdUI::Enable(BOOL bOn)
{
	m_Enabled = bOn;
	m_bEnableChanged = TRUE;
}


// CDialogMenuBar
//

#define BORDERBAR     3

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

		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_MENU);
	}

	// Default font
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS)-sizeof(ncm.iPaddedBorderWidth); 
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		m_MenuLogFont = m_NormalLogFont = ncm.lfMenuFont;
		m_MenuHeight = max(ncm.iMenuHeight, 2*BORDERBAR+16);
	}
	else
	{
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(m_MenuLogFont), &m_MenuLogFont);
		m_NormalLogFont = m_MenuLogFont;
		m_MenuHeight = 2*BORDERBAR+max(16, abs(m_MenuLogFont.lfHeight));
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
#define BORDER          4

CDialogMenuPopup::CDialogMenuPopup()
	: CWnd()
{
	p_App = (FMApplication*)AfxGetApp();

	m_Gutter = m_BlueAreaStart = m_FirstRowOffset = 0;
	m_Width = m_Height = 2*BORDERPOPUP;
	m_LargeIconsID = m_SmallIconsID = 0;
	m_SelectedItem = m_LastSelectedItem = -1;
	m_Hover = FALSE;
	hThemeButton = hThemeList = NULL;
	p_Submenu = NULL;
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
	BOOL res = CWnd::CreateEx(WS_EX_TOPMOST, className, _T(""), WS_BORDER | WS_POPUP, 0, 0, 0, 0, pParentWnd->GetSafeHwnd(), NULL);

	SetOwner(pTopLevelParent);

	return res;
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

void CDialogMenuPopup::AddItem(CDialogMenuItem* pItem, INT FirstRowOffset)
{
	ASSERT(pItem);

	if (!m_Items.m_ItemCount)
	{
		m_FirstRowOffset = FirstRowOffset;
		m_Height += FirstRowOffset;
	}

	// Hinzufügen
	MenuPopupItem i;
	ZeroMemory(&i, sizeof(i));
	i.pItem = pItem;
	i.Selectable = pItem->IsSelectable();
	if (i.Selectable)
		i.Enabled = pItem->IsEnabled();

	m_Items.AddItem(i);

	// Maße
	m_Gutter = max(m_Gutter, pItem->GetMinGutter());
	INT w = pItem->GetBorder()*2+pItem->GetMinWidth();
	m_Width = max(m_Width, w);
	m_Height += pItem->GetMinHeight();
}

void CDialogMenuPopup::AddCommand(UINT CmdID, INT IconID, UINT PreferredSize)
{
	AddItem(new CDialogMenuCommand(this, CmdID, IconID, PreferredSize));
}

void CDialogMenuPopup::AddSubmenu(UINT CmdID, INT IconID, UINT PreferredSize, BOOL IsSplit)
{
	AddItem(new CDialogMenuCommand(this, CmdID, IconID, PreferredSize, TRUE, IsSplit));
}

void CDialogMenuPopup::AddFileType(UINT CmdID, CString FileType, UINT PreferredSize)
{
	AddItem(new CDialogMenuFileType(this, CmdID, FileType, PreferredSize));
}

void CDialogMenuPopup::AddFile(UINT CmdID, CString Path, UINT PreferredSize)
{
	AddItem(new CDialogMenuFile(this, CmdID, Path, PreferredSize));
}

void CDialogMenuPopup::AddSeparator(BOOL ForBlueArea)
{
	ASSERT(m_BlueAreaStart==0);

	if (ForBlueArea)
		m_BlueAreaStart = m_Height;

	AddItem(new CDialogMenuSeparator(this, ForBlueArea));
}

void CDialogMenuPopup::AddCaption(UINT ResID)
{
	AddItem(new CDialogMenuCaption(this, ResID), -(BORDERPOPUP+3));
}

INT CDialogMenuPopup::ItemAtPosition(CPoint point)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (PtInRect(&m_Items.m_Items[a].Rect, point))
			return a;

	return -1;
}

void CDialogMenuPopup::InvalidateItem(INT idx)
{
	if (idx!=-1)
		InvalidateRect(&m_Items.m_Items[idx].Rect);
}

void CDialogMenuPopup::SelectItem(INT idx)
{
	if (idx!=m_SelectedItem)
	{
		InvalidateItem(m_LastSelectedItem);

		m_SelectedItem = idx;
		if (idx!=-1)
			m_LastSelectedItem = idx;

		InvalidateItem(m_SelectedItem);

		for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
			if (a!=m_LastSelectedItem)
				m_Items.m_Items[a].pItem->OnDeselect();
	}
}

void CDialogMenuPopup::Track(CPoint point)
{
	SetWindowPos(NULL, point.x, point.y, m_Width+2, m_Height+2, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	SetFocus();
}

void CDialogMenuPopup::TrackSubmenu(CDialogMenuPopup* pPopup)
{
	p_Submenu = pPopup;

	if ((m_SelectedItem!=-1) && (pPopup))
	{
		CRect rect(m_Items.m_Items[m_SelectedItem].Rect);
		ClientToScreen(rect);

		pPopup->Track(CPoint(rect.right-BORDERPOPUP, rect.top));
	}
}

void CDialogMenuPopup::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT y = BORDERPOPUP+m_FirstRowOffset;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		m_Items.m_Items[a].Rect.top = y;
		y = m_Items.m_Items[a].Rect.bottom = y+m_Items.m_Items[a].pItem->GetMinHeight();

		m_Items.m_Items[a].Rect.left = m_Items.m_Items[a].pItem->GetBorder();
		m_Items.m_Items[a].Rect.right = rect.Width()-m_Items.m_Items[a].pItem->GetBorder();
	}

	Invalidate();
}

BOOL CDialogMenuPopup::HasItems()
{
	return m_Items.m_ItemCount>0;
}

__forceinline INT CDialogMenuPopup::GetGutter()
{
	return m_Gutter;
}

__forceinline INT CDialogMenuPopup::GetBlueAreaStart()
{
	return m_BlueAreaStart;
}

__forceinline CFont* CDialogMenuPopup::SelectNormalFont(CDC* pDC)
{
	return pDC->SelectObject(&((CMainWindow*)GetTopLevelParent())->m_pDialogMenuBar->m_NormalFont);
}

__forceinline CFont* CDialogMenuPopup::SelectCaptionFont(CDC* pDC)
{
	return pDC->SelectObject(&((CMainWindow*)GetTopLevelParent())->m_pDialogMenuBar->m_CaptionFont);
}

void CDialogMenuPopup::DrawSelectedBackground(CDC* pDC, LPRECT rect, BOOL Enabled, BOOL Focused)
{
	if (hThemeList)
	{
		p_App->zDrawThemeBackground(hThemeList, *pDC, LVP_LISTITEM, !Enabled ? LISS_SELECTEDNOTFOCUS : Focused ? LISS_HOTSELECTED : LISS_HOT, rect, rect);
	}
	else
	{
		pDC->FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
	}
}


BEGIN_MESSAGE_MAP(CDialogMenuPopup, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_MESSAGE(WM_PTINRECT, OnPtInRect)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

INT CDialogMenuPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (p_App->m_ThemeLibLoaded)
	{
		hThemeButton = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (p_App->OSVersion>=OS_Vista)
		{
			p_App->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
			hThemeList = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
	}

	return 0;
}

void CDialogMenuPopup::OnDestroy()
{
	if (hThemeButton)
		p_App->zCloseThemeData(hThemeButton);
	if (hThemeList)
		p_App->zCloseThemeData(hThemeList);

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		delete m_Items.m_Items[a].pItem;

	CWnd::OnDestroy();
}

BOOL CDialogMenuPopup::OnEraseBkgnd(CDC* /*pDC*/)
{
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
	dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
}

void CDialogMenuPopup::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

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

	if (m_BlueAreaStart)
	{
		dc.FillSolidRect(rect.left, rect.top, rect.Width(), m_BlueAreaStart, 0xFFFFFF);
		dc.FillSolidRect(rect.left, m_BlueAreaStart, rect.Width(), rect.bottom, Themed ? 0xFBF5F1 : GetSysColor(COLOR_3DFACE));
	}
	else
	{
		dc.FillSolidRect(rect, 0xFFFFFF);
	}

	// Items
	CFont* pOldFont = SelectNormalFont(&dc);
	CRect rectIntersect;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (IntersectRect(&rectIntersect, &m_Items.m_Items[a].Rect, rectUpdate))
		{
			BOOL Selected = (m_SelectedItem==(INT)a) && (m_Items.m_Items[a].Selectable);
			dc.SetTextColor(!m_Items.m_Items[a].Enabled ? GetSysColor(COLOR_3DSHADOW) : hThemeList ? 0x6E1500 : Selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT));

			m_Items.m_Items[a].pItem->OnPaint(&dc, &m_Items.m_Items[a].Rect, m_SelectedItem==(INT)a, hThemeList ? 2 : Themed ? 1 : 0);
		}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

LRESULT CDialogMenuPopup::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hThemeButton)
			p_App->zCloseThemeData(hThemeButton);
		if (hThemeList)
			p_App->zCloseThemeData(hThemeList);

		hThemeButton = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (p_App->OSVersion>=OS_Vista)
			hThemeList = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CDialogMenuPopup::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = FMHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}

	SelectItem(Item);

	if (Item!=-1)
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		m_Items.m_Items[Item].pItem->OnMouseMove(point);
	}
}

void CDialogMenuPopup::OnMouseLeave()
{
	if (m_SelectedItem!=-1)
		m_Items.m_Items[m_SelectedItem].pItem->OnMouseLeave();

	SelectItem(-1);
	m_Hover = FALSE;
}

void CDialogMenuPopup::OnMouseHover(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		m_Items.m_Items[Item].pItem->OnHover(point);
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = FMHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CDialogMenuPopup::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		m_Items.m_Items[Item].pItem->OnButtonDown(point);
	}
}

void CDialogMenuPopup::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		m_Items.m_Items[Item].pItem->OnButtonUp(point);
	}
}

void CDialogMenuPopup::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

LRESULT CDialogMenuPopup::OnPtInRect(WPARAM wParam, LPARAM /*lParam*/)
{
	CRect rect;
	GetClientRect(rect);
	ClientToScreen(rect);

	CPoint pt(LOWORD(wParam), HIWORD(wParam));

	return rect.PtInRect(pt) ? TRUE : p_Submenu ? p_Submenu->OnPtInRect(wParam) : FALSE;
}

INT CDialogMenuPopup::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return p_Submenu ? MA_NOACTIVATE : CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CDialogMenuPopup::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CWnd::OnActivateApp(bActive, dwThreadID);

	if (!bActive)
		GetOwner()->PostMessage(WM_CLOSEPOPUP);
}


// CDialogMenuItem
//

CDialogMenuItem::CDialogMenuItem(CDialogMenuPopup* pParentPopup)
{
	p_ParentPopup = pParentPopup;
}

CDialogMenuItem::~CDialogMenuItem()
{
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

INT CDialogMenuItem::GetBorder()
{
	return BORDERPOPUP;
}

BOOL CDialogMenuItem::IsEnabled()
{
	return FALSE;
}

BOOL CDialogMenuItem::IsSelectable()
{
	return FALSE;
}

void CDialogMenuItem::OnPaint(CDC* /*pDC*/, LPRECT /*rect*/, BOOL /*Selected*/, UINT /*Themed*/)
{
}

void CDialogMenuItem::OnDeselect()
{
}

void CDialogMenuItem::OnButtonDown(CPoint /*point*/)
{
}

void CDialogMenuItem::OnButtonUp(CPoint /*point*/)
{
}
void CDialogMenuItem::OnMouseMove(CPoint /*point*/)
{
}

void CDialogMenuItem::OnMouseLeave()
{
}

void CDialogMenuItem::OnHover(CPoint /*point*/)
{
}


// CDialogMenuCommand
//

CDialogMenuCommand::CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize, BOOL Submenu, BOOL Split)
	: CDialogMenuItem(pParentPopup)
{
	m_CmdID = CmdID;
	m_IconID = IconID;
	m_IconSize.cx = m_IconSize.cy = (IconID==-1) ? 0 : (PreferredSize==CDMB_SMALL) ? 16 : 32;
	m_PreferredSize = PreferredSize;
	m_Submenu = Submenu;
	m_Split = Split;
	m_Enabled = FALSE;
	m_pSubmenu = NULL;

	ENSURE(m_Caption.LoadString(CmdID));

	INT pos = m_Caption.Find(L'\n');
	if (pos!=-1)
	{
		m_Hint = m_Caption.Mid(pos+1);
		m_Caption.Truncate(pos);
	}
}

CDialogMenuCommand::~CDialogMenuCommand()
{
	if (m_pSubmenu)
	{
		p_ParentPopup->TrackSubmenu(NULL);

		m_pSubmenu->DestroyWindow();
		delete m_pSubmenu;
	}
}

INT CDialogMenuCommand::GetMinHeight()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectNormalFont(pDC);

	INT h = 0;
	if (m_PreferredSize==CDMB_LARGE)
	{
		h += pDC->GetTextExtent(_T("Wy")).cy*2+BORDER/2;

		p_ParentPopup->SelectCaptionFont(pDC);
	}

	h += pDC->GetTextExtent(_T("Wy")).cy;

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	if (m_IconID!=-1)
		h = max(h, m_IconSize.cy);

	return 2*BORDER+h;
}

INT CDialogMenuCommand::GetMinWidth()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectNormalFont(pDC);

	INT h = 0;
	if (m_PreferredSize==CDMB_LARGE)
	{
		CRect rectHint(0, 0, 1000, 1000);
		pDC->DrawText(m_Hint, rectHint, DT_NOPREFIX | DT_LEFT | DT_END_ELLIPSIS | DT_CALCRECT);
		h = rectHint.Width();

		p_ParentPopup->SelectCaptionFont(pDC);
	}

	CRect rectCaption(0, 0, 1000, 1000);
	pDC->DrawText(m_Caption, rectCaption, DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CALCRECT);

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	m_Hint.Replace('\n', ' ');

	return 3*BORDER+p_ParentPopup->GetGutter()+max(rectCaption.Width(), h);
}

INT CDialogMenuCommand::GetMinGutter()
{
	return (m_IconID==-1) ? 0 : m_IconSize.cx+BORDER;
}

BOOL CDialogMenuCommand::IsEnabled()
{
	CDialogCmdUI cmdUI;
	cmdUI.m_nID = m_CmdID;
	cmdUI.DoUpdate(p_ParentPopup->GetOwner(), TRUE);

	return m_Enabled = cmdUI.m_Enabled;
}

BOOL CDialogMenuCommand::IsSelectable()
{
	return TRUE;
}

void CDialogMenuCommand::OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed)
{
	Selected |= (m_pSubmenu!=NULL);

	// Hintergrund
	if (Selected)
	{
		p_ParentPopup->DrawSelectedBackground(pDC, rect, m_Enabled);
		if (Themed<2)
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}

	// Icon
	if (m_IconID!=-1)
		OnDrawIcon(pDC, CPoint(rect->left+BORDER+(p_ParentPopup->GetGutter()-BORDER-m_IconSize.cx)/2, rect->top+(rect->bottom-rect->top-m_IconSize.cy)/2));

	// Text
	CRect rectText(rect);
	rectText.left += p_ParentPopup->GetGutter();
	rectText.right -= BORDER;
	rectText.DeflateRect(BORDER, BORDER);

	if (m_PreferredSize==CDMB_LARGE)
	{
		CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

		pDC->DrawText(m_Caption, rectText, DT_TOP | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
		rectText.top += BORDER/2+pDC->GetTextExtent(m_Caption).cy;

		if (pOldFont)
			pDC->SelectObject(pOldFont);

		pDC->DrawText(m_Hint, rectText, DT_NOPREFIX | DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);
	}
	else
	{
		pDC->DrawText(m_Caption, rectText, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
	}
}

void CDialogMenuCommand::OnDrawIcon(CDC* pDC, CPoint pt)
{
	CMFCToolBarImages* pIcons = (m_PreferredSize==CDMB_SMALL) ? &p_ParentPopup->m_SmallIcons : &p_ParentPopup->m_LargeIcons;

	CAfxDrawState ds;
	pIcons->PrepareDrawImage(ds);
	pIcons->Draw(pDC, pt.x, pt.y, m_IconID);
	pIcons->EndDrawImage(ds);
}

void CDialogMenuCommand::OnDeselect()
{
	if (m_pSubmenu)
	{
		p_ParentPopup->TrackSubmenu(NULL);

		m_pSubmenu->DestroyWindow();
		delete m_pSubmenu;
		m_pSubmenu = NULL;
	}
}

void CDialogMenuCommand::OnButtonUp(CPoint /*point*/)
{
	if (m_Submenu)
	{
		if (!m_pSubmenu)
		{
			m_pSubmenu = (CDialogMenuPopup*)p_ParentPopup->GetOwner()->SendMessage(WM_REQUESTSUBMENU, (WPARAM)m_CmdID);
			if (m_pSubmenu)
				p_ParentPopup->TrackSubmenu(m_pSubmenu);
		}
	}
	else
		if (m_Enabled)
		{
			p_ParentPopup->GetOwner()->PostMessage(WM_CLOSEPOPUP);
			p_ParentPopup->GetOwner()->PostMessage(WM_COMMAND, m_CmdID);
		}
}

void CDialogMenuCommand::OnHover(CPoint point)
{
	if (m_Submenu)
		OnButtonUp(point);
}


// CDialogMenuFileType
//

CDialogMenuFileType::CDialogMenuFileType(CDialogMenuPopup* pParentPopup, UINT CmdID, CString FileType, UINT PreferredSize)
	: CDialogMenuCommand(pParentPopup, CmdID, -1, PreferredSize)
{
	p_Icons = (PreferredSize==CDMB_SMALL) ? &((FMApplication*)AfxGetApp())->m_SystemImageListSmall : &((FMApplication*)AfxGetApp())->m_SystemImageListLarge;

	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);
	ImageList_GetIconSize(*p_Icons, &cx, &cy);

	m_IconSize.cx = cx;
	m_IconSize.cy = cy;

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo(FileType, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
	{
		if (sfi.iIcon!=3)
			m_Caption.Format(_T("&%s (%s)"), sfi.szTypeName, FileType);

		m_IconID = sfi.iIcon;
	}
}

INT CDialogMenuFileType::GetMinHeight()
{
	INT h = CDialogMenuCommand::GetMinHeight();

	if (m_IconID!=-1)
		h = max(h, 2*BORDER+m_IconSize.cy);

	return h;
}

INT CDialogMenuFileType::GetMinGutter()
{
	return (m_IconID==-1) ? 0 : m_IconSize.cx+BORDER;
}

void CDialogMenuFileType::OnDrawIcon(CDC* pDC, CPoint pt)
{
	CImageList* pIcons = (m_PreferredSize==CDMB_SMALL) ? &((FMApplication*)AfxGetApp())->m_SystemImageListSmall : &((FMApplication*)AfxGetApp())->m_SystemImageListLarge;
	pIcons->DrawEx(pDC, m_IconID, pt, m_IconSize, CLR_NONE, CLR_NONE, ILD_NORMAL);
}


// CDialogMenuFile
//

CDialogMenuFile::CDialogMenuFile(CDialogMenuPopup* pParentPopup, UINT CmdID, CString Path, UINT PreferredSize)
	: CDialogMenuFileType(pParentPopup, CmdID, Path, PreferredSize)
{
	// Filename
	m_Caption = Path;
	INT pos = Path.ReverseFind(L'\\');
	if (pos!=-1)
		m_Caption.Delete(0, pos+1);

	// Metadata
	if (PreferredSize==CDMB_LARGE)
	{
		m_Hint.Empty();

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(Path, &ffd);
		if (hFind!=INVALID_HANDLE_VALUE)
		{
			if ((ffd.ftLastWriteTime.dwHighDateTime) || (ffd.ftLastWriteTime.dwLowDateTime))
			{
				WCHAR tmpStr[256];

				SYSTEMTIME stUTC;
				SYSTEMTIME stLocal;
				FileTimeToSystemTime(&ffd.ftLastWriteTime, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, tmpStr, 256);
				m_Hint = tmpStr;
				m_Hint.Append(_T(", "));

				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &stLocal, NULL, tmpStr, 256);
				m_Hint.Append(tmpStr);
				m_Hint.Append(_T("\n"));
			}

			ULARGE_INTEGER u;
			u.HighPart = ffd.nFileSizeHigh;
			u.LowPart = ffd.nFileSizeLow;

			WCHAR tmpStr[256];
			StrFormatByteSize(u.QuadPart, tmpStr, 256);
			m_Hint.Append(tmpStr);

			FindClose(hFind);
		}
	}
}


// CDialogMenuSeparator
//

CDialogMenuSeparator::CDialogMenuSeparator(CDialogMenuPopup* pParentPopup, BOOL ForBlueArea)
	: CDialogMenuItem(pParentPopup)
{
	m_ForBlueArea = ForBlueArea;
}

INT CDialogMenuSeparator::GetMinHeight()
{
	return m_ForBlueArea ? 3 : 2*BORDERPOPUP+1;
}

INT CDialogMenuSeparator::GetBorder()
{
	return 0;
}

void CDialogMenuSeparator::OnPaint(CDC* pDC, LPRECT rect, BOOL /*Selected*/, UINT Themed)
{
	INT l = rect->right-rect->left;

	if (!m_ForBlueArea)
	{
		INT left = rect->left+p_ParentPopup->GetGutter()+BORDERPOPUP+BORDER;
		if (Themed)
		{
			pDC->FillSolidRect(left, rect->top+BORDERPOPUP, l-left-BORDERPOPUP, 1, 0xC5C5C5);
		}
		else
		{
			pDC->FillSolidRect(left, rect->top+BORDERPOPUP, l-left-BORDERPOPUP, 1, GetSysColor(COLOR_3DSHADOW));

			INT BlueAreaStart = p_ParentPopup->GetBlueAreaStart();
			if ((BlueAreaStart) && (rect->top>=BlueAreaStart))
				pDC->FillSolidRect(left, rect->top+BORDERPOPUP+1, l-left-BORDERPOPUP, 1, GetSysColor(COLOR_3DHIGHLIGHT));
		}
	}
	else
		if (Themed)
		{
			pDC->FillSolidRect(rect->left, rect->top, l, 1, 0xF1E1DA);
			pDC->FillSolidRect(rect->left, rect->top+1, l, 1, 0xF4EAE3);
			pDC->FillSolidRect(rect->left, rect->top+2, l, 1, 0xF9F0EC);
		}
}


// CDialogMenuTitle
//

CDialogMenuCaption::CDialogMenuCaption(CDialogMenuPopup* pParentPopup, UINT ResID)
	: CDialogMenuItem(pParentPopup)
{
	ENSURE(m_Caption.LoadString(ResID));
}

INT CDialogMenuCaption::GetMinHeight()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

	CRect rectCaption(0, 0, 1000, 1000);
	pDC->DrawText(m_Caption, rectCaption, DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CALCRECT);

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	return BORDER+2*BORDERPOPUP+rectCaption.Height();
}

INT CDialogMenuCaption::GetMinWidth()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

	CRect rectCaption(0, 0, 1000, 1000);
	pDC->DrawText(m_Caption, rectCaption, DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CALCRECT);

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	return 2*(BORDER+BORDERPOPUP)+rectCaption.Width();
}

INT CDialogMenuCaption::GetBorder()
{
	return 0;
}

void CDialogMenuCaption::OnPaint(CDC* pDC, LPRECT rect, BOOL /*Selected*/, UINT Themed)
{
	CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

	CRect rectText(rect);
	rectText.DeflateRect(0, BORDERPOPUP);

	if (Themed)
	{
		INT l = rectText.Width();
		INT h = rectText.Height();

		pDC->FillSolidRect(rectText.left, rectText.top, l, 1, 0xC5C5C5);
		pDC->FillSolidRect(rectText.left, rectText.top+1, l, h-2, 0xEEE7DD);
		pDC->FillSolidRect(rectText.left, rectText.top+h-1, l, 1, 0xC5C5C5);
	}

	rectText.DeflateRect(BORDER+BORDERPOPUP, Themed ? 1 : 0);

	pDC->SetTextColor(Themed ? 0x6E1500 : GetSysColor(COLOR_MENUTEXT));
	pDC->DrawText(m_Caption, rectText, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	pDC->SelectObject(pOldFont);
}
