
// CDialogMenuBar.cpp: Implementierung der Klasse CDialogMenuBar
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CDialogCmdUI
//

CDialogCmdUI::CDialogCmdUI()
	: CCmdUI()
{
	m_Enabled = m_Checked = FALSE;
	m_CheckedItem = -1;
}

void CDialogCmdUI::Enable(BOOL bOn)
{
	m_Enabled = bOn;
	m_bEnableChanged = TRUE;
}

void CDialogCmdUI::SetCheck(INT nCheck)
{
	m_Checked = (nCheck!=0);
	m_CheckedItem = nCheck;
}


// CDialogMenuBar
//

#define BORDERBAR     2
#define FOCUSED       ((GetFocus()==this) || m_UseDropdown)

CDialogMenuBar::CDialogMenuBar()
	: CWnd()
{
	hTheme = NULL;
	m_SelectedItem = m_HoverItem = -1;
	m_LastMove.x = m_LastMove.y = -1;
	m_Hover = m_UseDropdown = FALSE;
	m_pPopup = NULL;
}

BOOL CDialogMenuBar::Create(CWnd* pParentWnd, UINT LargeResID, UINT SmallResID, UINT nID)
{
	LOGFONT lf;
	FMGetApp()->m_DefaultFont.GetLogFont(&lf);

	m_IconSize = abs(lf.lfHeight)>=24 ? 32 : 16;

	m_Icons.SetImageSize(CSize(m_IconSize, m_IconSize));
	if (LargeResID)
		m_Icons.Load(m_IconSize==16 ? SmallResID : LargeResID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CDialogMenuBar::PreTranslateMessage(MSG* pMsg)
{
	CWnd* pWnd = GetTopLevelParent();

	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
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
		if (pWnd)
		{
			CPoint pt;
			GetCursorPos(&pt);

			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			if (!rect.PtInRect(pt))
			{
				OnMouseLeave();

				pWnd->ScreenToClient(&pt);
				pMsg->lParam = MAKELPARAM(pt.x, pt.y);

				if (!pWnd->PreTranslateMessage(pMsg))
					pWnd->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

				return TRUE;
			}
		}
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

UINT CDialogMenuBar::GetPreferredHeight()
{
	return IsCtrlThemed() ? m_MenuHeight : m_MenuHeight+3;
}

INT CDialogMenuBar::GetMinWidth()
{
	INT Spacer = (FMGetApp()->OSVersion==OS_XP) ? 10 : 6;
	INT MinWidth = 0;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		MinWidth += m_Items.m_Items[a].MinWidth+Spacer;

	return MinWidth;
}

void CDialogMenuBar::AddMenuLeft(UINT nID)
{
	MenuBarItem i;
	ZeroMemory(&i, sizeof(i));
	i.PopupID = nID;
	i.Enabled = TRUE;
	ENSURE(LoadString(AfxGetResourceHandle(), nID, i.Name, 256));

	WCHAR* pChar = wcschr(i.Name, L'&');
	i.Accelerator = pChar ? towupper(*(pChar+1)) : 0;

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_MenuFont);
	i.MinWidth = pDC->GetTextExtent(i.Name, (INT)wcslen(i.Name)).cx;
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	m_Items.AddItem(i);
}

void CDialogMenuBar::AddMenuRight(UINT nCmdID, INT nIconID)
{
	CDialogCmdUI cmdUI;
	cmdUI.m_nID = nCmdID;
	cmdUI.DoUpdate(GetOwner(), TRUE);

	MenuBarItem i;
	ZeroMemory(&i, sizeof(i));
	i.CmdID = nCmdID;
	i.IconID = nIconID;
	i.MinWidth = m_IconSize;
	i.Enabled = cmdUI.m_Enabled;

	m_Items.AddItem(i);
}

INT CDialogMenuBar::ItemAtPosition(CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if ((point.y<rect.top) || (point.y>rect.bottom))
		return -1;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if ((point.x>=m_Items.m_Items[a].Left) && (point.x<m_Items.m_Items[a].Right))
			return a;

	return -1;
}

void CDialogMenuBar::InvalidateItem(INT idx)
{
	if (idx!=-1)
	{
		CRect rect;
		GetClientRect(rect);

		rect.left = m_Items.m_Items[idx].Left;
		rect.right = m_Items.m_Items[idx].Right;

		InvalidateRect(&rect);
	}
}

void CDialogMenuBar::SelectItem(INT idx, BOOL Keyboard)
{
	if (idx!=m_SelectedItem)
	{
		if (m_pPopup)
		{
			((CMainWindow*)GetTopLevelParent())->p_PopupWindow = NULL;

			m_pPopup->DestroyWindow();
			delete m_pPopup;
			m_pPopup = NULL;
		}

		InvalidateItem(m_SelectedItem);
		m_SelectedItem = idx;
		InvalidateItem(m_SelectedItem);

		if (m_UseDropdown && (idx!=-1))
		{
			m_pPopup = (CDialogMenuPopup*)GetOwner()->SendMessage(WM_REQUESTSUBMENU, m_Items.m_Items[idx].PopupID);

			if (m_pPopup)
			{
				m_pPopup->SetParentMenu(this, Keyboard);

				CRect rect;
				GetClientRect(rect);

				rect.left = m_Items.m_Items[idx].Left;
				rect.right = m_Items.m_Items[idx].Right;

				ClientToScreen(rect);
				m_pPopup->Track(rect);
			}
			else
			{
				SetFocus();
			}
		}
	}
}

void CDialogMenuBar::ExecuteItem(INT idx)
{
	if (idx!=-1)
		if (m_Items.m_Items[idx].CmdID)
		{
			m_SelectedItem = m_HoverItem = -1;
			m_UseDropdown = FALSE;

			Invalidate();

			GetOwner()->PostMessage(WM_CLOSEPOPUP);
			GetOwner()->PostMessage(WM_COMMAND, m_Items.m_Items[idx].CmdID);
		}
}

void CDialogMenuBar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT Left = 0;
	INT Spacer = (FMGetApp()->OSVersion==OS_XP) ? 10 : 6;
	BOOL OnLeftSide = TRUE;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		if ((m_Items.m_Items[a].CmdID) && OnLeftSide)
		{
			OnLeftSide = FALSE;

			Left = rect.Width();
			for (UINT b=a; b<m_Items.m_ItemCount; b++)
				if (m_Items.m_Items[b].Enabled)
					Left -= m_Items.m_Items[a].MinWidth+Spacer;
		}

		if (m_Items.m_Items[a].Enabled)
		{
			m_Items.m_Items[a].Left = Left;
			m_Items.m_Items[a].Right = Left+m_Items.m_Items[a].MinWidth+Spacer;
			Left = m_Items.m_Items[a].Right;
		}
		else
		{
			m_Items.m_Items[a].Left = m_Items.m_Items[a].Right = 0;
		}
	}

	Invalidate();
}

BOOL CDialogMenuBar::HasFocus()
{
	return (GetFocus()==this) || m_pPopup;
}

void CDialogMenuBar::SetTheme()
{
	// Themes
	if (FMGetApp()->m_ThemeLibLoaded)
	{
		if (hTheme)
			FMGetApp()->zCloseThemeData(hTheme);

		hTheme = FMGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_MENU);
	}

	// Default font
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS)-sizeof(ncm.iPaddedBorderWidth);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		m_MenuLogFont = m_NormalLogFont = ncm.lfMenuFont;
		m_MenuHeight = max(2*BORDERBAR+m_IconSize, ncm.iMenuHeight);
	}
	else
	{
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(m_MenuLogFont), &m_MenuLogFont);
		m_NormalLogFont = m_MenuLogFont;
		m_MenuHeight = 2*BORDERBAR+max(m_IconSize, abs(m_MenuLogFont.lfHeight));
	}

	m_MenuFont.DeleteObject();
	m_MenuFont.CreateFontIndirect(&m_MenuLogFont);

	// Special fonts
	m_NormalLogFont.lfHeight = -max(abs(m_NormalLogFont.lfHeight), 11);
	wcscpy_s(m_NormalLogFont.lfFaceName, 32, FMGetApp()->GetDefaultFontFace());
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
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_CLOSEPOPUP, OnClosePopup)
	ON_MESSAGE(WM_PTINRECT, OnPtInRect)
	ON_MESSAGE_VOID(WM_MENULEFT, OnMenuLeft)
	ON_MESSAGE_VOID(WM_MENURIGHT, OnMenuRight)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
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
		FMGetApp()->zCloseThemeData(hTheme);

	CWnd::OnDestroy();
}

void CDialogMenuBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (IsCtrlThemed() && (hTheme==NULL))
		lpncsp->rgrc[0].bottom--;
}

void CDialogMenuBar::OnNcPaint()
{
	CWindowDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(rectClient);

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	rectClient.OffsetRect(-rectWindow.TopLeft());
	rectWindow.OffsetRect(-rectWindow.TopLeft());

	pDC.ExcludeClipRect(rectClient);
	pDC.FillSolidRect(rectWindow, GetSysColor(COLOR_SCROLLBAR));
	pDC.SelectClipRgn(NULL);
}

BOOL CDialogMenuBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CDialogMenuBar::OnPaint()
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
	if (hTheme)
	{
		FMGetApp()->zDrawThemeBackground(hTheme, dc, MENU_BARBACKGROUND, ((CMainWindow*)GetParent())->m_Active ? MB_ACTIVE : MB_INACTIVE, rect, rect);
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_MENUBAR));
	}

	// Items
	CFont* pOldFont = dc.SelectObject(&m_MenuFont);

	BOOL Focused = FOCUSED;
	UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	if (!Focused || m_pPopup)
	{
		BOOL AlwaysUnderline = FALSE;
		if (SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &AlwaysUnderline, 0))
			if (!AlwaysUnderline)
				format |= DT_HIDEPREFIX;
	}

	BOOL Themed = IsCtrlThemed();
	INT Item = Focused ? m_SelectedItem : m_HoverItem;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CRect rectItem(m_Items.m_Items[a].Left, rect.top, m_Items.m_Items[a].Right, rect.bottom);

		CRect rectIntersect;
		if (IntersectRect(&rectIntersect, rectItem, rectUpdate))
		{
			COLORREF clrText = GetSysColor(((CMainWindow*)GetParent())->m_Active ? COLOR_MENUTEXT : COLOR_3DSHADOW);

			if (Item==(INT)a)
				if (hTheme)
				{
					FMGetApp()->zDrawThemeBackground(hTheme, dc, MENU_BARITEM, Focused ? MBI_PUSHED : MBI_HOT, rectItem, rectItem);
				}
				else
					if (Themed)
					{
						dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
						clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
					}
					else
					{
						dc.Draw3dRect(rectItem, GetSysColor(m_UseDropdown ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT), GetSysColor(m_UseDropdown ? COLOR_3DHIGHLIGHT : COLOR_3DSHADOW));

						if (m_UseDropdown)
							rectItem.OffsetRect(1, 1);
					}

			if (m_Items.m_Items[a].CmdID)
			{
				CAfxDrawState ds;
				m_Icons.PrepareDrawImage(ds);
				m_Icons.Draw(&dc, rectItem.left+(rectItem.Width()-m_IconSize)/2, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_Items.m_Items[a].IconID);
				m_Icons.EndDrawImage(ds);
			}
			else
			{
				if (FMGetApp()->OSVersion==OS_XP)
					rectItem.bottom -= 2;

				dc.SetTextColor(clrText);
				dc.DrawText(m_Items.m_Items[a].Name, -1, rectItem, format);
			}
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

void CDialogMenuBar::OnClosePopup()
{
	if (m_pPopup)
	{
		m_pPopup = NULL;

		m_UseDropdown = FALSE;
		m_SelectedItem = -1;
		Invalidate();
	}
}

LRESULT CDialogMenuBar::OnPtInRect(WPARAM wParam, LPARAM lParam)
{
	CRect rect;
	GetClientRect(rect);
	ClientToScreen(rect);

	CPoint pt(LOWORD(wParam), HIWORD(wParam));

	return m_pPopup ? rect.PtInRect(pt) || m_pPopup->SendMessage(WM_PTINRECT, wParam, lParam) : FALSE;
}

void CDialogMenuBar::OnMenuLeft()
{
	INT idx = m_SelectedItem;
	for (UINT a=0; a<m_Items.m_ItemCount-1; a++)
	{
		idx--;
		if (idx<0)
			idx = m_Items.m_ItemCount-1;

		if (m_Items.m_Items[idx].Enabled)
			break;
	}

	SelectItem(idx, TRUE);
}

void CDialogMenuBar::OnMenuRight()
{
	INT idx = m_SelectedItem;
	for (UINT a=0; a<m_Items.m_ItemCount-1; a++)
	{
		idx++;
		if (idx>=(INT)m_Items.m_ItemCount)
			idx = 0;

		if (m_Items.m_Items[idx].Enabled)
			break;
	}

	SelectItem(idx, TRUE);
}

void CDialogMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	INT Item = ItemAtPosition(point);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = FMHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}

	if ((m_LastMove.x!=point.x) || (m_LastMove.y!=point.y))
	{
		m_LastMove = point;
		m_UseDropdown |= (Item!=-1) && (nFlags & MK_LBUTTON);

		if ((Item!=-1) || (!m_UseDropdown))
		{
			InvalidateItem(m_HoverItem);
			m_HoverItem = Item;
			InvalidateItem(Item);

			if (FOCUSED)
				SelectItem(Item, FALSE);
		}
	}
}

void CDialogMenuBar::OnMouseLeave()
{
	m_Hover = FALSE;
	m_HoverItem = -1;
	Invalidate();
}

void CDialogMenuBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		if (!m_UseDropdown)
		{
			m_UseDropdown = TRUE;
			m_SelectedItem = -1;
			Invalidate();
		}

		SelectItem(Item, FALSE);

		if (!FOCUSED)
			SetFocus();
	}
}

void CDialogMenuBar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	ExecuteItem(ItemAtPosition(point));
}

void CDialogMenuBar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (((nChar>='A') && (nChar<='Z')) || ((nChar>='0') && (nChar<='9')))
	{
		OnSysKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
	else
		switch (nChar)
		{
		case VK_HOME:
			for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
				if (m_Items.m_Items[a].Enabled)
				{
					SelectItem(a, TRUE);
					break;
				}
			return;
		case VK_END:
			for (INT a=(INT)m_Items.m_ItemCount; a>=0; a--)
				if (m_Items.m_Items[a].Enabled)
				{
					SelectItem(a, TRUE);
					break;
				}
			return;
		case VK_UP:
		case VK_DOWN:
			if (!m_UseDropdown && (m_SelectedItem!=-1))
			{
				INT Item = m_SelectedItem;
				m_SelectedItem = -1;

				m_UseDropdown = TRUE;
				SelectItem(Item, TRUE);
			}
			return;
		case VK_LEFT:
		case VK_RIGHT:
			SendMessage(nChar==VK_LEFT ? WM_MENULEFT : WM_MENURIGHT);
			return;
		case VK_RETURN:
		case VK_EXECUTE:
			ExecuteItem(m_SelectedItem);
			return;
		case VK_ESCAPE:
			GetOwner()->SetFocus();
			return;
		}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDialogMenuBar::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (((nChar>='A') && (nChar<='Z')) || ((nChar>='0') && (nChar<='9')))
	{
		INT idx = m_SelectedItem;
		INT Cnt = 0;
		INT Item = -1;

		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (++idx>=(INT)m_Items.m_ItemCount)
				idx = 0;
			if (nChar==m_Items.m_Items[idx].Accelerator)
				if ((Cnt++)==0)
					Item = idx;
		}

		if (Cnt>=1)
		{
			m_UseDropdown |= (Cnt==1);
			SelectItem(Item, TRUE);
		}

		return;
	}

	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CDialogMenuBar::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (((nChar==VK_MENU) || (nChar==VK_F10)) && (m_SelectedItem==-1))
		SelectItem(0, TRUE);

	CWnd::OnSysKeyUp(nChar, nRepCnt, nFlags);
}

void CDialogMenuBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
}

void CDialogMenuBar::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items.m_Items[a].CmdID)
		{
			BOOL Enabled = m_Items.m_Items[a].Enabled;

			CDialogCmdUI cmdUI;
			cmdUI.m_nID = m_Items.m_Items[a].CmdID;
			cmdUI.DoUpdate(GetOwner(), TRUE);

			if (cmdUI.m_Enabled!=Enabled)
			{
				m_Items.m_Items[a].Enabled = cmdUI.m_Enabled;
				Update = TRUE;
			}
		}

	if (Update)
		AdjustLayout();
}

void CDialogMenuBar::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CDialogMenuBar::OnKillFocus(CWnd* /*pKillWnd*/)
{
	if (!m_pPopup)
	{
		m_SelectedItem = m_HoverItem = -1;
		m_UseDropdown = FALSE;

		Invalidate();
	}
}


// CDialogMenuPopup
//

#define BORDERPOPUP     2
#define BORDER          4

CDialogMenuPopup::CDialogMenuPopup()
	: CWnd()
{
	m_Gutter = m_BlueAreaStart = m_FirstRowOffset = 0;
	m_Width = m_Height = 2*BORDERPOPUP;
	m_LargeIconsID = m_SmallIconsID = 0;
	m_SelectedItem = m_LastSelectedItem = -1;
	m_LastMove.x = m_LastMove.y = -1;
	m_EnableHover = m_Hover = m_Keyboard = FALSE;
	hThemeList = hThemeButton = NULL;
	p_ParentMenu = p_SubMenu = NULL;
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

	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
	if (bDropShadow)
		nClassStyle |= CS_DROPSHADOW;

	CString className = AfxRegisterWndClass(nClassStyle, FMGetApp()->LoadStandardCursor(IDC_ARROW));
	BOOL Result = CWnd::CreateEx(WS_EX_TOPMOST, className, _T(""), WS_BORDER | WS_POPUP, 0, 0, 0, 0, pParentWnd->GetSafeHwnd(), NULL);

	if (pParentWnd)
	{
		CMainWindow* pTopLevelParent = (CMainWindow*)pParentWnd->GetTopLevelParent();
		pTopLevelParent->RegisterPopupWindow(this);
		SetOwner(pTopLevelParent);
	}

	return Result;
}

BOOL CDialogMenuPopup::PreTranslateMessage(MSG* pMsg)
{
	CWnd* pWnd = p_ParentMenu ? p_ParentMenu : GetTopLevelParent();

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (pMsg->wParam==VK_ESCAPE)
		{
			GetOwner()->PostMessage(WM_CLOSEPOPUP);
			return TRUE;
		}
		break;
	case WM_SYSKEYDOWN:
		if ((pMsg->wParam==VK_F4) && (GetParent()))
		{
			GetOwner()->PostMessage(WM_CLOSE);
			return TRUE;
		}
		break;
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
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
		if (pWnd)
		{
			CPoint pt;
			GetCursorPos(&pt);

			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			if (!rect.PtInRect(pt))
			{
				OnMouseLeave();

				pWnd->ScreenToClient(&pt);
				pMsg->lParam = MAKELPARAM(pt.x, pt.y);

				if (!pWnd->PreTranslateMessage(pMsg))
					pWnd->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);

				return TRUE;
			}
		}
		break;
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
	{
		i.Enabled = pItem->IsEnabled();
		i.Checked = pItem->IsChecked();
	}
	i.Accelerator = towupper(pItem->GetAccelerator());

	m_Items.AddItem(i);

	// Maße
	m_Gutter = max(m_Gutter, pItem->GetMinGutter());
	INT w = pItem->GetOuterBorder()*2+pItem->GetMinWidth();
	m_Width = max(m_Width, w);
	m_Height += pItem->GetMinHeight();
}

void CDialogMenuPopup::AddGallery(UINT CmdID, UINT IconsID, CSize IconSize, UINT FirstCaptionID, UINT ItemCount, UINT Columns, COLORREF DefaultColor, BOOL CloseOnExecute)
{
	AddItem(new CDialogMenuGallery(this, CmdID, IconsID, IconSize, FirstCaptionID, ItemCount, Columns, DefaultColor, CloseOnExecute));
}

void CDialogMenuPopup::AddCommand(UINT CmdID, INT IconID, UINT PreferredSize, BOOL CloseOnExecute)
{
	AddItem(new CDialogMenuCommand(this, CmdID, IconID, PreferredSize, FALSE, FALSE, CloseOnExecute));
}

void CDialogMenuPopup::AddSubmenu(UINT CmdID, INT IconID, UINT PreferredSize, BOOL IsSplit)
{
	AddItem(new CDialogMenuCommand(this, CmdID, IconID, PreferredSize, TRUE, IsSplit));
}

void CDialogMenuPopup::AddFileType(UINT CmdID, CString FileType, UINT PreferredSize, BOOL RetainCaption)
{
	AddItem(new CDialogMenuFileType(this, CmdID, FileType, PreferredSize, RetainCaption));
}

void CDialogMenuPopup::AddFile(UINT CmdID, CString Path, UINT PreferredSize)
{
	AddItem(new CDialogMenuFile(this, CmdID, Path, PreferredSize));
}

void CDialogMenuPopup::AddCheckbox(UINT CmdID, BOOL Radio, BOOL CloseOnExecute)
{
	AddItem(new CDialogMenuCheckbox(this, CmdID, Radio, CloseOnExecute));
}

void CDialogMenuPopup::AddColor(UINT CmdID, COLORREF* pColor)
{
	AddItem(new CDialogMenuColor(this, CmdID, pColor));
}

void CDialogMenuPopup::AddResolution(UINT CmdID, INT IconID, UINT* pWidth, UINT* pHeight)
{
	AddItem(new CDialogMenuResolution(this, CmdID, IconID, pWidth, pHeight));
}

void CDialogMenuPopup::AddSeparator(BOOL ForBlueArea)
{
	if (ForBlueArea)
		m_BlueAreaStart = m_Height;

	AddItem(new CDialogMenuSeparator(this, ForBlueArea));
}

void CDialogMenuPopup::AddCaption(UINT ResID)
{
	AddItem(new CDialogMenuCaption(this, ResID), -(BORDERPOPUP+3));
}

void CDialogMenuPopup::GetCheckSize(CSize& sz)
{
	if (hThemeButton)
	{
		CDC* dc = GetDC();
		FMGetApp()->zGetThemePartSize(hThemeButton, *dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &sz);
		ReleaseDC(dc);
	}
	else
	{
		sz.cx = GetSystemMetrics(SM_CXMENUCHECK);
		sz.cy = GetSystemMetrics(SM_CYMENUCHECK);
	}
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

void CDialogMenuPopup::SelectItem(INT idx, BOOL FromTop)
{
	if (idx!=m_SelectedItem)
	{
		InvalidateItem(m_LastSelectedItem);

		m_SelectedItem = idx;
		if (idx!=-1)
		{
			m_Items.m_Items[idx].pItem->OnSelect(m_Keyboard, FromTop);
			m_LastSelectedItem = idx;
		}

		InvalidateItem(m_SelectedItem);

		for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
			if (a!=m_LastSelectedItem)
				m_Items.m_Items[a].pItem->OnDeselect();
	}
}

void CDialogMenuPopup::SetParentMenu(CWnd* pWnd, BOOL Keyboard)
{
	p_ParentMenu = pWnd;
	m_Keyboard = Keyboard;

	if (pWnd && Keyboard)
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
			if (m_Items.m_Items[a].Enabled)
			{
				if (IsWindow(GetSafeHwnd()))
				{
					SelectItem(a, Keyboard);
				}
				else
				{
					m_SelectedItem = a;
				}

				break;
			}
}

void CDialogMenuPopup::Track(CRect rect, BOOL Down)
{
	// Get screen rectangle
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	// Determine point
	CPoint pt;
	if (Down)
	{
		pt.x = max(rect.left, 0);
		pt.y = rect.bottom;

		if (pt.x+m_Width+2>rectScreen.right)
			pt.x = max(rect.right-m_Width-2, 0);
		if ((pt.y+m_Height+2>rectScreen.bottom) && (rect.top-m_Height-2>0))
			pt.y = rect.top-m_Height-2;
	}
	else
	{
		pt.x = rect.right-BORDERPOPUP+1;
		pt.y = max(rect.top, 0);

		if ((pt.x+m_Width+2>rectScreen.right) && (rect.left-m_Width-3+BORDERPOPUP>0))
			pt.x = rect.left-m_Width-3+BORDERPOPUP;
		if (pt.y+m_Height+2>rectScreen.bottom)
			pt.y = max(rectScreen.bottom-m_Height-2, 0);
	}

	// Set
	SetWindowPos(NULL, pt.x, pt.y, m_Width+2, m_Height+2, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	SetFocus();

	GetCursorPos(&m_LastMove);
	ScreenToClient(&m_LastMove);

	SetCapture();
	SetCursor(FMGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CDialogMenuPopup::Track(CPoint point)
{
	Track(CRect(point, point));
}

void CDialogMenuPopup::TrackSubmenu(CDialogMenuPopup* pPopup, BOOL Keyboard)
{
	p_SubMenu = pPopup;

	if ((m_SelectedItem!=-1) && (pPopup))
	{
		CRect rect(m_Items.m_Items[m_SelectedItem].Rect);
		ClientToScreen(rect);

		pPopup->SetParentMenu(this, Keyboard);
		pPopup->Track(rect, FALSE);
	}

	if (!pPopup)
	{
		SetCapture();
		SetCursor(FMGetApp()->LoadStandardCursor(IDC_ARROW));
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

		m_Items.m_Items[a].Rect.left = m_Items.m_Items[a].pItem->GetOuterBorder();
		m_Items.m_Items[a].Rect.right = rect.Width()-m_Items.m_Items[a].pItem->GetOuterBorder();
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

void CDialogMenuPopup::DrawBevelRect(CDC& dc, INT x, INT y, INT width, INT height, BOOL Themed)
{
	if (Themed && (FMGetApp()->OSVersion!=OS_Eight))
	{
		Graphics g(dc);

		LinearGradientBrush brush1(Point(x, y), Point(x, y+height/2), Color(0xF5, 0xF8, 0xFC), Color(0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush1, x, y, width, height/2);

		LinearGradientBrush brush2(Point(x, y+height/2-1), Point(x, y+height), Color(0xFF, 0xFF, 0xFF), Color(0xF5, 0xF8, 0xFC));
		g.FillRectangle(&brush2, x, y+height/2, width, height-height/2);
	}
	else
	{
		dc.FillSolidRect(x, y, width, height, 0xFFFFFF);
	}
}

void CDialogMenuPopup::DrawSelectedBackground(CDC* pDC, LPRECT rect, BOOL Enabled, BOOL Focused)
{
	if (hThemeList)
	{
		FMGetApp()->zDrawThemeBackground(hThemeList, *pDC, LVP_LISTITEM, !Enabled ? LISS_SELECTEDNOTFOCUS : Focused ? LISS_HOTSELECTED : LISS_HOT, rect, rect);
	}
	else
	{
		pDC->FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
	}
}

void CDialogMenuPopup::DrawButton(CDC* pDC, LPRECT rect, BOOL Radio, BOOL Checked, BOOL Enabled, BOOL Selected, BOOL Pressed)
{
	if (hThemeButton)
	{
		INT uiStyle = Enabled ? Selected ? Pressed ? CBS_UNCHECKEDPRESSED : CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL : CBS_UNCHECKEDDISABLED;
		if (Checked)
			uiStyle += 4;

		FMGetApp()->zDrawThemeBackground(hThemeButton, *pDC, Radio ? BP_RADIOBUTTON : BP_CHECKBOX, uiStyle, rect, rect);
	}
	else
	{
		UINT uiStyle = (Radio ? DFCS_BUTTONRADIO : DFCS_BUTTONCHECK) | (Enabled ? 0 : DFCS_INACTIVE) | (Checked ? DFCS_CHECKED : 0) | ((Selected && Pressed) ? DFCS_PUSHED : 0);

		pDC->DrawFrameControl(rect, DFC_BUTTON, uiStyle);
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
	ON_MESSAGE_VOID(WM_MENULEFT, OnMenuLeft)
	ON_MESSAGE_VOID(WM_MENURIGHT, OnMenuRight)
	ON_MESSAGE(WM_MENUUPDATESTATUS, OnMenuUpdateStatus)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

INT CDialogMenuPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (FMGetApp()->m_ThemeLibLoaded)
	{
		if (FMGetApp()->OSVersion>=OS_Vista)
		{
			FMGetApp()->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
			hThemeList = FMGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}

		hThemeButton = FMGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

	return 0;
}

void CDialogMenuPopup::OnDestroy()
{
	if (hThemeList)
		FMGetApp()->zCloseThemeData(hThemeList);
	if (hThemeButton)
		FMGetApp()->zCloseThemeData(hThemeButton);

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
		DrawBevelRect(dc, rect.left, rect.top, rect.Width(), m_BlueAreaStart, Themed);
		dc.FillSolidRect(rect.left, m_BlueAreaStart, rect.Width(), rect.bottom, Themed && (FMGetApp()->OSVersion!=OS_Eight) ? 0xFBF5F1 : GetSysColor(COLOR_3DFACE));
	}
	else
	{
		DrawBevelRect(dc, rect.left, rect.top, rect.Width(), rect.Height(), Themed);
	}

	// Items
	CFont* pOldFont = SelectNormalFont(&dc);
	CRect rectIntersect;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (IntersectRect(&rectIntersect, &m_Items.m_Items[a].Rect, rectUpdate))
		{
			BOOL Selected = (m_SelectedItem==(INT)a) && (m_Items.m_Items[a].Selectable);
			dc.SetTextColor(!m_Items.m_Items[a].Enabled ? GetSysColor(COLOR_3DSHADOW) : hThemeList ? FMGetApp()->OSVersion!=OS_Eight ? 0x6E1500 : GetSysColor(COLOR_MENUTEXT) : Selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT));

			m_Items.m_Items[a].pItem->OnPaint(&dc, &m_Items.m_Items[a].Rect, m_SelectedItem==(INT)a, hThemeList ? 2 : Themed ? 1 : 0);
		}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

LRESULT CDialogMenuPopup::OnThemeChanged()
{
	if (FMGetApp()->m_ThemeLibLoaded)
	{
		if (hThemeList)
			FMGetApp()->zCloseThemeData(hThemeList);
		if (hThemeButton)
			FMGetApp()->zCloseThemeData(hThemeButton);

		if (FMGetApp()->OSVersion>=OS_Vista)
			hThemeList = FMGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		hThemeButton = FMGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

	return TRUE;
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

	return rect.PtInRect(pt) ? TRUE : p_SubMenu ? p_SubMenu->OnPtInRect(wParam) : FALSE;
}

void CDialogMenuPopup::OnMenuLeft()
{
	if ((m_LastSelectedItem!=-1) && (p_SubMenu))
	{
		m_Items.m_Items[m_LastSelectedItem].pItem->OnDeselect();
	}
	else
		if (p_ParentMenu)
		{
			p_ParentMenu->SendMessage(WM_MENULEFT);
		}
}

void CDialogMenuPopup::OnMenuRight()
{
	if (p_ParentMenu)
		p_ParentMenu->SendMessage(WM_MENURIGHT);
}

LRESULT CDialogMenuPopup::OnMenuUpdateStatus(WPARAM /*wParam*/, LPARAM lParam)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		if (m_Items.m_Items[a].Selectable)
		{
			BOOL Enabled = m_Items.m_Items[a].pItem->IsEnabled();
			BOOL Checked = m_Items.m_Items[a].pItem->IsChecked();
			if ((Enabled!=m_Items.m_Items[a].Enabled) || (Checked!=m_Items.m_Items[a].Checked))
			{
				m_Items.m_Items[a].Enabled = Enabled;
				m_Items.m_Items[a].Checked = Checked;
				InvalidateItem(a);
			}
		}

		if (m_Items.m_Items[a].pItem==(CDialogMenuItem*)lParam)
			InvalidateItem(a);
	}

	return p_ParentMenu ? p_ParentMenu->SendMessage(WM_MENUUPDATESTATUS) : NULL;
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

	if ((m_LastMove.x!=point.x) || (m_LastMove.y!=point.y))
	{
		m_LastMove = point;

		if ((Item!=m_SelectedItem) && (Item!=-1))
			m_EnableHover = TRUE;

		if ((p_SubMenu) && (Item==-1))
			Item = m_LastSelectedItem;

		SelectItem(Item);
		m_Keyboard = FALSE;

		if (Item!=-1)
		{
			point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
			if (m_Items.m_Items[Item].pItem->OnMouseMove(point))
				InvalidateItem(Item);
		}
	}
}

void CDialogMenuPopup::OnMouseLeave()
{
	if (!p_SubMenu && !m_Keyboard)
	{
		if (m_SelectedItem!=-1)
			if (m_Items.m_Items[m_SelectedItem].pItem->OnMouseLeave())
				InvalidateItem(m_SelectedItem);

		SelectItem(-1);
	}

	m_Hover = FALSE;
	if (!p_SubMenu)
		m_LastMove.x = m_LastMove.y = -1;
}

void CDialogMenuPopup::OnMouseHover(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if ((Item!=-1) && (m_EnableHover))
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		if (m_Items.m_Items[Item].pItem->OnHover(point))
		{
			InvalidateItem(Item);
			SelectItem(Item);
		}

		m_EnableHover = FALSE;
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
		if (m_Items.m_Items[Item].pItem->OnButtonDown(point))
			InvalidateItem(Item);
	}
}

void CDialogMenuPopup::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		point.Offset(-m_Items.m_Items[Item].Rect.left, -m_Items.m_Items[Item].Rect.top);
		if (m_Items.m_Items[Item].pItem->OnButtonUp(point))
			InvalidateItem(Item);
	}
}

void CDialogMenuPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT idx = m_SelectedItem;
	if (idx!=-1)
		if (m_Items.m_Items[idx].pItem->OnKeyDown(nChar))
		{
			InvalidateItem(idx);
			return;
		}

	if (((nChar>='A') && (nChar<='Z')) || ((nChar>='0') && (nChar<='9')))
	{
		INT Cnt = 0;
		INT Item = -1;

		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (++idx>=(INT)m_Items.m_ItemCount)
				idx = 0;
			if ((m_Items.m_Items[idx].Selectable) && (nChar==m_Items.m_Items[idx].Accelerator))
				if ((Cnt++)==0)
					Item = idx;
		}

		if (Cnt>=1)
		{
			SelectItem(Item);
			m_Keyboard = TRUE;

			if (Cnt==1)
				if (m_Items.m_Items[Item].Enabled)
					m_Items.m_Items[Item].pItem->OnKeyDown(VK_EXECUTE);
		}
		else
		{
			FMGetApp()->PlayStandardSound();
		}

		return;
	}
	else
		switch (nChar)
		{
		case VK_HOME:
			for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
				if (m_Items.m_Items[a].Selectable)
				{
					SelectItem(a, FALSE);
					m_Keyboard = TRUE;
					break;
				}
			return;
		case VK_END:
			for (INT a=(INT)m_Items.m_ItemCount-1; a>=0; a--)
				if (m_Items.m_Items[a].Selectable)
				{
					SelectItem(a, TRUE);
					m_Keyboard = TRUE;
					break;
				}
			return;
		case VK_UP:
			for (UINT a=0; a<m_Items.m_ItemCount; a++)
			{
				if (--idx<0)
					idx = m_Items.m_ItemCount-1;
				if (m_Items.m_Items[idx].Selectable)
				{
					SelectItem(idx, FALSE);
					m_Keyboard = TRUE;
					break;
				}
			}
			return;
		case VK_DOWN:
			for (UINT a=0; a<m_Items.m_ItemCount; a++)
			{
				if (++idx>=(INT)m_Items.m_ItemCount)
					idx = 0;
				if (m_Items.m_Items[idx].Selectable)
				{
					SelectItem(idx, TRUE);
					m_Keyboard = TRUE;
					break;
				}
			}
			return;
		case VK_LEFT:
		case VK_RIGHT:
			if (p_ParentMenu)
				p_ParentMenu->SendMessage(nChar==VK_LEFT ? WM_MENULEFT : WM_MENURIGHT);
			return;
		}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDialogMenuPopup::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
}

INT CDialogMenuPopup::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT Message)
{
	return p_SubMenu ? MA_NOACTIVATE : CWnd::OnMouseActivate(pDesktopWnd, nHitTest, Message);
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

INT CDialogMenuItem::GetOuterBorder()
{
	return BORDERPOPUP;
}

WCHAR CDialogMenuItem::GetAccelerator()
{
	return L'\0';
}

BOOL CDialogMenuItem::IsEnabled()
{
	return FALSE;
}

BOOL CDialogMenuItem::IsChecked()
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

void CDialogMenuItem::OnSelect(BOOL /*Keyboard*/, BOOL /*FromTop*/)
{
}

void CDialogMenuItem::OnDeselect()
{
}

BOOL CDialogMenuItem::OnButtonDown(CPoint /*point*/)
{
	return FALSE;
}

BOOL CDialogMenuItem::OnButtonUp(CPoint /*point*/)
{
	return FALSE;
}

BOOL CDialogMenuItem::OnMouseMove(CPoint /*point*/)
{
	return FALSE;
}

BOOL CDialogMenuItem::OnMouseLeave()
{
	return FALSE;
}

BOOL CDialogMenuItem::OnHover(CPoint /*point*/)
{
	return FALSE;
}

BOOL CDialogMenuItem::OnKeyDown(UINT /*nChar*/)
{
	return FALSE;
}


// CDialogMenuGallery
//

CDialogMenuGallery::CDialogMenuGallery(CDialogMenuPopup* pParentPopup, UINT CmdID, UINT IconsID, CSize IconSize, UINT FirstCaptionID, UINT ItemCount, UINT Columns, COLORREF DefaultColor, BOOL CloseOnExecute)
	: CDialogMenuItem(pParentPopup)
{
	ASSERT(Columns>0);

	m_CmdID = CmdID;
	m_IconSize = IconSize;
	m_ItemCount = ItemCount;
	m_Rows = (ItemCount+Columns-1)/Columns;
	m_Columns = Columns;
	m_Enabled = m_Pressed = FALSE;
	m_CloseOnExecute = CloseOnExecute;
	m_SelectedItem = 0;
	m_HoverItem = -1;
	m_DefaultColor = DefaultColor;

	pParentPopup->GetCheckSize(m_CheckSize);
	m_Icons.SetImageSize(IconSize);
	if (IconsID)
		m_Icons.Load(IconsID);

	m_Captions = new CString[ItemCount];
	for (UINT a=0; a<ItemCount; a++)
		ENSURE(m_Captions[a].LoadString(FirstCaptionID+a));
}

CDialogMenuGallery::~CDialogMenuGallery()
{
	delete[] m_Captions;
}

void CDialogMenuGallery::Execute()
{
	if (m_CloseOnExecute)
		p_ParentPopup->GetOwner()->PostMessage(WM_CLOSEPOPUP);

	p_ParentPopup->GetOwner()->PostMessage(WM_GALLERYCHANGED, m_CmdID, m_HoverItem);

	if (!m_CloseOnExecute)
		p_ParentPopup->PostMessage(WM_MENUUPDATESTATUS, NULL, (LPARAM)this);
}

INT CDialogMenuGallery::GetMinHeight()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectNormalFont(pDC);
	m_ItemHeight = max(m_CheckSize.cy, pDC->GetTextExtent(_T("Wy")).cy)+BORDER+m_IconSize.cy+5;
	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	return m_ItemHeight*m_Rows;
}

INT CDialogMenuGallery::GetMinWidth()
{
	m_ItemWidth = BORDER+m_IconSize.cx+4;

	return m_ItemWidth*m_Columns;
}

BOOL CDialogMenuGallery::IsEnabled()
{
	CDialogCmdUI cmdUI;
	cmdUI.m_nID = m_CmdID;
	cmdUI.DoUpdate(p_ParentPopup->GetOwner(), TRUE);

	return m_Enabled = cmdUI.m_Enabled;
}

BOOL CDialogMenuGallery::IsChecked()
{
	CDialogCmdUI cmdUI;
	cmdUI.m_nID = m_CmdID;
	cmdUI.DoUpdate(p_ParentPopup->GetOwner(), TRUE);

	m_SelectedItem = cmdUI.m_CheckedItem;
	return TRUE;
}

BOOL CDialogMenuGallery::IsSelectable()
{
	return TRUE;
}

void CDialogMenuGallery::OnPaint(CDC* pDC, LPRECT rect, BOOL Selected, UINT Themed)
{
	for (UINT a=0; a<m_ItemCount; a++)
	{
		CRect rectItem(CPoint(rect->left+(a%m_Columns)*m_ItemWidth, rect->top+(a/m_Columns)*m_ItemHeight), CSize(m_ItemWidth, m_ItemHeight));
		if (Selected && ((INT)a==m_HoverItem))
			p_ParentPopup->DrawSelectedBackground(pDC, rectItem, m_Enabled);

		rectItem.DeflateRect(BORDER/2+1, BORDER/2+1);

		if ((INT)a<m_Icons.GetCount())
		{
			CAfxDrawState ds;
			m_Icons.PrepareDrawImage(ds);
			m_Icons.Draw(pDC, rectItem.left+1, rectItem.top+1, a);
			m_Icons.EndDrawImage(ds);
		}
		else
		{
			pDC->FillSolidRect(rectItem.left+1, rectItem.top+1, m_IconSize.cx, m_IconSize.cy, m_DefaultColor);
		}

		COLORREF clr = Themed ? m_Enabled ? (Selected && ((INT)a==m_HoverItem)) ? 0x000000 : 0x8F8F8E : 0xB1B1B1 : 0x000000;
		pDC->Draw3dRect(rectItem.left, rectItem.top, m_IconSize.cx+2, m_IconSize.cy+2, clr, clr);

		CRect rectText(rectItem);
		rectText.top += m_IconSize.cy+BORDER/2+1;
		rectText.bottom++;

		INT l = pDC->GetTextExtent(m_Captions[a]).cx+m_CheckSize.cx+BORDER;

		CRect rectButton(rectText);
		rectButton.left = rectText.left+max(0, (rectText.Width()-l)/2);
		rectButton.right = rectButton.left+m_CheckSize.cx;

		p_ParentPopup->DrawButton(pDC, rectButton, TRUE, (INT)a==m_SelectedItem, m_Enabled, (Selected && ((INT)a==m_HoverItem)), m_Pressed);
		rectText.left = rectButton.right+BORDER;

		pDC->SetTextColor(!m_Enabled ? GetSysColor(COLOR_3DSHADOW) : Themed==2 ? FMGetApp()->OSVersion!=OS_Eight ? 0x6E1500 : GetSysColor(COLOR_MENUTEXT) : (Selected && ((INT)a==m_HoverItem)) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT));
		pDC->DrawText(m_Captions[a], rectText, DT_HIDEPREFIX | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
	}
}

void CDialogMenuGallery::OnSelect(BOOL Keyboard, BOOL FromTop)
{
	if (Keyboard)
		m_HoverItem = FromTop ? 0 : (m_Rows-1)*m_Columns;
}

void CDialogMenuGallery::OnDeselect()
{
	m_Pressed = FALSE;
}

BOOL CDialogMenuGallery::OnButtonDown(CPoint /*point*/)
{
	m_Pressed = TRUE;

	return TRUE;
}

BOOL CDialogMenuGallery::OnButtonUp(CPoint /*point*/)
{
	m_Pressed = FALSE;

	if (m_Enabled && (m_HoverItem!=-1))
		Execute();

	return TRUE;
}

BOOL CDialogMenuGallery::OnMouseMove(CPoint point)
{
	INT Col = point.x/m_ItemWidth;
	INT Row = point.y/m_ItemHeight;
	m_HoverItem = (Col<(INT)m_Columns) && (Row<(INT)m_Rows) ? Row*m_Columns+Col : -1;

	return TRUE;
}

BOOL CDialogMenuGallery::OnMouseLeave()
{
	m_HoverItem = -1;

	return TRUE;
}

BOOL CDialogMenuGallery::OnKeyDown(UINT nChar)
{
	INT Item = m_HoverItem==-1 ? m_SelectedItem : m_HoverItem;
	INT Col = Item%m_Columns;
	INT Row = Item/m_Columns;

	switch (nChar)
	{
	case VK_RETURN:
	case VK_EXECUTE:
		if (m_Enabled && (m_HoverItem!=-1))
		{
			Execute();
			return TRUE;
		}
		break;
	case VK_UP:
		if (Row>0)
		{
			m_HoverItem = (Row-1)*m_Columns+Col;
			return TRUE;
		}
		break;
	case VK_DOWN:
		if (Row<((INT)m_Rows)-1)
		{
			m_HoverItem = (Row+1)*m_Columns+Col;
			return TRUE;
		}
		break;
	case VK_LEFT:
		if (Col>0)
		{
			m_HoverItem = Row*m_Columns+Col-1;
			return TRUE;
		}
		break;
	case VK_RIGHT:
		if (Col<((INT)m_Columns)-1)
		{
			m_HoverItem = Row*m_Columns+Col+1;
			return TRUE;
		}
		break;
	}

	return FALSE;
}


// CDialogMenuCommand
//

#define ARROWWIDTH     4
#define MARGIN         BORDER+3

CDialogMenuCommand::CDialogMenuCommand(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT PreferredSize, BOOL Submenu, BOOL Split, BOOL CloseOnExecute)
	: CDialogMenuItem(pParentPopup)
{
	if (PreferredSize==CDMB_SMALL)
	{
		LOGFONT lf;
		FMGetApp()->m_DefaultFont.GetLogFont(&lf);

		if (abs(lf.lfHeight)>=24)
			PreferredSize = CDMB_MEDIUM;
	}

	m_CmdID = CmdID;
	m_IconID = IconID;
	m_IconSize.cx = m_IconSize.cy = (IconID==-1) ? 0 : (PreferredSize==CDMB_SMALL) ? 16 : 32;
	m_PreferredSize = PreferredSize;
	m_Submenu = Submenu;
	m_Split = Split && Submenu;
	m_Enabled = m_HoverOverCommand = FALSE;
	m_AutoFlow = TRUE;
	m_CloseOnExecute = CloseOnExecute;
	m_pSubmenu = NULL;

	if (CmdID)
		ENSURE(m_Caption.LoadString(CmdID));

	INT pos = m_Caption.ReverseFind(L'\n');
	if (pos!=-1)
	{
		m_Hint = m_Caption.Mid(0, pos);
		m_Caption.Delete(0, pos+1);
	}
}

CDialogMenuCommand::~CDialogMenuCommand()
{
	if (m_pSubmenu)
	{
		p_ParentPopup->TrackSubmenu(NULL, FALSE);

		m_pSubmenu->DestroyWindow();
		delete m_pSubmenu;
	}
}

BOOL CDialogMenuCommand::PtOnSubmenuArrow(CPoint point)
{
	return m_Submenu ? m_Split ? point.x>=m_Width-2*GetInnerBorder()-ARROWWIDTH : TRUE : FALSE;
}

BOOL CDialogMenuCommand::TrackSubmenu(BOOL Select)
{
	if (!m_pSubmenu)
	{
		m_pSubmenu = (CDialogMenuPopup*)p_ParentPopup->GetOwner()->SendMessage(WM_REQUESTSUBMENU, (WPARAM)m_CmdID);
		if (m_pSubmenu)
		{
			p_ParentPopup->TrackSubmenu(m_pSubmenu, Select);
			return TRUE;
		}
	}

	return FALSE;
}

void CDialogMenuCommand::Execute()
{
	if (m_CloseOnExecute)
		p_ParentPopup->GetOwner()->PostMessage(WM_CLOSEPOPUP);

	p_ParentPopup->GetOwner()->PostMessage(WM_COMMAND, m_CmdID);

	if (!m_CloseOnExecute)
		p_ParentPopup->PostMessage(WM_MENUUPDATESTATUS, NULL, (LPARAM)this);
}

INT CDialogMenuCommand::GetMinHeight()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectNormalFont(pDC);

	INT h = 0;
	if (m_PreferredSize==CDMB_LARGE)
	{
		h += pDC->GetTextExtent(_T("Wy")).cy*2+GetInnerBorder()/2;

		p_ParentPopup->SelectCaptionFont(pDC);
	}

	h += pDC->GetTextExtent(_T("Wy")).cy;

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	if (m_IconID!=-1)
		h = max(h, m_IconSize.cy);

	return 2*GetInnerBorder()+h;
}

INT CDialogMenuCommand::GetMinWidth()
{
	CDC* pDC = p_ParentPopup->GetWindowDC();
	CFont* pOldFont = p_ParentPopup->SelectNormalFont(pDC);

	INT l = 0;
	if (m_PreferredSize==CDMB_LARGE)
	{
		CRect rectHint(0, 0, 1000, 1000);
		pDC->DrawText(m_Hint, rectHint, DT_NOPREFIX | DT_LEFT | DT_END_ELLIPSIS | DT_CALCRECT);
		l = rectHint.Width();

		p_ParentPopup->SelectCaptionFont(pDC);
	}

	CRect rectCaption(0, 0, 1000, 1000);
	pDC->DrawText(m_Caption, rectCaption, DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CALCRECT);

	pDC->SelectObject(pOldFont);
	p_ParentPopup->ReleaseDC(pDC);

	if (m_AutoFlow)
		m_Hint.Replace('\n', ' ');

	return GetInnerBorder()+MARGIN+p_ParentPopup->GetGutter()+max(rectCaption.Width(), l)+(m_Submenu ? ARROWWIDTH+2*GetInnerBorder() : 0);
}

INT CDialogMenuCommand::GetMinGutter()
{
	return (m_IconID==-1) ? 0 : m_IconSize.cx+GetInnerBorder();
}

__forceinline INT CDialogMenuCommand::GetInnerBorder()
{
	return m_PreferredSize==CDMB_SMALL ? BORDER-1 : BORDER+1;
}

WCHAR CDialogMenuCommand::GetAccelerator()
{
	INT Pos = m_Caption.Find(L'&');

	return (Pos==-1) ? L'\0' : m_Caption[Pos+1];
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
	m_Width = rect->right-rect->left;

	Selected |= (m_pSubmenu!=NULL);

	// Hintergrund
	if (Selected)
	{
		if (m_Split)
		{
			CRect rectLeft(rect);
			CRect rectRight(rect);
			rectRight.left = rect->right-ARROWWIDTH-2*GetInnerBorder();
			rectLeft.right = rectRight.left+1;

			p_ParentPopup->DrawSelectedBackground(pDC, rectLeft, m_Enabled, m_HoverOverCommand);
			p_ParentPopup->DrawSelectedBackground(pDC, rectRight, TRUE, (m_pSubmenu!=NULL) || !m_HoverOverCommand);
		}
		else
		{
			p_ParentPopup->DrawSelectedBackground(pDC, rect, m_Enabled);
		}

		if (Themed<2)
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}

	// Icon
	if (m_IconID!=-1)
		OnDrawIcon(pDC, CPoint(rect->left+GetInnerBorder()+(p_ParentPopup->GetGutter()-GetInnerBorder()-m_IconSize.cx)/2, rect->top+(rect->bottom-rect->top-m_IconSize.cy)/2), Selected, Themed);

	// Pfeil
	if (m_Submenu)
	{
		COLORREF col = Themed==2 ? FMGetApp()->OSVersion!=OS_Eight ? 0x6E1500 : GetSysColor(COLOR_MENUTEXT) : Selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT);

		CRect rectArrow(rect);
		rectArrow.left = rectArrow.right-ARROWWIDTH-2*GetInnerBorder();
		rectArrow.DeflateRect(GetInnerBorder(), GetInnerBorder());

		const INT mid = (rectArrow.top+rectArrow.bottom)/2;
		for (INT a=0; a<ARROWWIDTH; a++)
		{
			pDC->FillSolidRect(rectArrow.left, mid-a, ARROWWIDTH-a, 1, col);
			pDC->FillSolidRect(rectArrow.left, mid+a, ARROWWIDTH-a, 1, col);
		}
	}

	// Text
	CRect rectText(rect);
	rectText.left += p_ParentPopup->GetGutter()+MARGIN;
	rectText.right -= m_Submenu ? 3*GetInnerBorder()+ARROWWIDTH : GetInnerBorder();
	rectText.DeflateRect(0, GetInnerBorder());

	if (m_PreferredSize==CDMB_LARGE)
	{
		CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

		pDC->DrawText(m_Caption, rectText, DT_TOP | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
		rectText.top += GetInnerBorder()/2+pDC->GetTextExtent(m_Caption).cy;

		if (pOldFont)
			pDC->SelectObject(pOldFont);

		pDC->DrawText(m_Hint, rectText, DT_NOPREFIX | DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);
	}
	else
	{
		pDC->DrawText(m_Caption, rectText, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);
	}
}

void CDialogMenuCommand::OnDrawIcon(CDC* pDC, CPoint pt, BOOL /*Selected*/, BOOL /*Themed*/)
{
	CMFCToolBarImages* pIcons = (m_IconSize.cx==16) ? &p_ParentPopup->m_SmallIcons : &p_ParentPopup->m_LargeIcons;

	CAfxDrawState ds;
	pIcons->PrepareDrawImage(ds);
	pIcons->Draw(pDC, pt.x, pt.y, m_IconID);
	pIcons->EndDrawImage(ds);
}

void CDialogMenuCommand::OnSelect(BOOL Keyboard, BOOL /*FromTop*/)
{
	if (Keyboard)
		m_HoverOverCommand = TRUE;
}

void CDialogMenuCommand::OnDeselect()
{
	if (m_pSubmenu)
	{
		p_ParentPopup->TrackSubmenu(NULL, FALSE);

		m_pSubmenu->DestroyWindow();
		delete m_pSubmenu;
		m_pSubmenu = NULL;

		p_ParentPopup->SetFocus();
	}
}

BOOL CDialogMenuCommand::OnButtonDown(CPoint point)
{
	return PtOnSubmenuArrow(point) ? TrackSubmenu(FALSE) : FALSE;
}

BOOL CDialogMenuCommand::OnButtonUp(CPoint point)
{
	if (PtOnSubmenuArrow(point))
		return TrackSubmenu(FALSE);

	if (m_Enabled)
		Execute();

	return FALSE;
}

BOOL CDialogMenuCommand::OnMouseMove(CPoint point)
{
	m_HoverOverCommand = !PtOnSubmenuArrow(point);

	return m_Split;
}

BOOL CDialogMenuCommand::OnMouseLeave()
{
	m_HoverOverCommand = FALSE;

	return m_Split;
}

BOOL CDialogMenuCommand::OnHover(CPoint point)
{
	m_HoverOverCommand = !PtOnSubmenuArrow(point);

	return m_Submenu ? TrackSubmenu(FALSE) : FALSE;
}

BOOL CDialogMenuCommand::OnKeyDown(UINT nChar)
{
	switch (nChar)
	{
	case VK_RETURN:
	case VK_EXECUTE:
		if (!m_Split)
			if (TrackSubmenu(TRUE))
				return TRUE;

		if (m_Enabled)
		{
			Execute();
			return TRUE;
		}
		break;
	case VK_RIGHT:
		m_HoverOverCommand = FALSE;
		return m_Submenu ? TrackSubmenu(TRUE) : FALSE;
	}

	return FALSE;
}


// CDialogMenuFileType
//

CDialogMenuFileType::CDialogMenuFileType(CDialogMenuPopup* pParentPopup, UINT CmdID, CString FileType, UINT PreferredSize, BOOL RetainCaption)
	: CDialogMenuCommand(pParentPopup, CmdID, -1, PreferredSize)
{
	LOGFONT lf;
	FMGetApp()->m_DefaultFont.GetLogFont(&lf);

	p_Icons = (PreferredSize==CDMB_SMALL) ? &FMGetApp()->m_SystemImageListSmall : &FMGetApp()->m_SystemImageListLarge;

	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);
	ImageList_GetIconSize(*p_Icons, &cx, &cy);

	m_IconSize.cx = cx;
	m_IconSize.cy = cy;

	FileType.Insert(0, L'.');

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo(FileType, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
	{
		if (!RetainCaption)
			if (sfi.iIcon!=3)
			{
				m_Caption.Format(_T("&%s (%s)"), sfi.szTypeName, FileType);
			}
			else
			{
				m_Caption.Append(_T(" (")+FileType+_T(")"));
			}

		m_IconID = sfi.iIcon;
	}
}

void CDialogMenuFileType::OnDrawIcon(CDC* pDC, CPoint pt, BOOL /*Selected*/, BOOL /*Themed*/)
{
	CImageList* pIcons = (m_IconSize.cx<32) ? &FMGetApp()->m_SystemImageListSmall : &FMGetApp()->m_SystemImageListLarge;
	pIcons->DrawEx(pDC, m_IconID, pt, m_IconSize, CLR_NONE, CLR_NONE, ILD_NORMAL);
}


// CDialogMenuFile
//

CDialogMenuFile::CDialogMenuFile(CDialogMenuPopup* pParentPopup, UINT CmdID, CString Path, UINT PreferredSize)
	: CDialogMenuFileType(pParentPopup, 0, Path, PreferredSize, TRUE)
{
	// Filename
	m_CmdID = CmdID;
	m_AutoFlow = FALSE;

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

				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stLocal, NULL, tmpStr, 256);
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


// CDialogMenuCheckbox
//

CDialogMenuCheckbox::CDialogMenuCheckbox(CDialogMenuPopup* pParentPopup, UINT CmdID, BOOL Radio, BOOL CloseOnExecute)
	: CDialogMenuCommand(pParentPopup, CmdID, 0, CDMB_SMALL, FALSE, FALSE, CloseOnExecute)
{
	m_Checked = m_Pressed = FALSE;
	m_Radio = Radio;

	pParentPopup->GetCheckSize(m_IconSize);
}

INT CDialogMenuCheckbox::GetMinHeight()
{
	INT h = CDialogMenuCommand::GetMinHeight();
	return h+((h-m_IconSize.cy) & 1);
}

BOOL CDialogMenuCheckbox::IsChecked()
{
	CDialogCmdUI cmdUI;
	cmdUI.m_nID = m_CmdID;
	cmdUI.DoUpdate(p_ParentPopup->GetOwner(), TRUE);

	return m_Checked = cmdUI.m_Checked;
}

void CDialogMenuCheckbox::OnDrawIcon(CDC* pDC, CPoint pt, BOOL Selected, BOOL /*Themed*/)
{
	CRect rectButton(pt.x, pt.y, pt.x+m_IconSize.cx, pt.y+m_IconSize.cy);

	p_ParentPopup->DrawButton(pDC, rectButton, m_Radio, m_Checked, m_Enabled, Selected, m_Pressed);
}

void CDialogMenuCheckbox::OnDeselect()
{
	m_Pressed = FALSE;

	CDialogMenuCommand::OnDeselect();
}

BOOL CDialogMenuCheckbox::OnButtonDown(CPoint point)
{
	m_Pressed = TRUE;

	CDialogMenuCommand::OnButtonDown(point);
	return TRUE;
}

BOOL CDialogMenuCheckbox::OnButtonUp(CPoint point)
{
	m_Pressed = FALSE;

	CDialogMenuCommand::OnButtonUp(point);
	return TRUE;
}


// CDialogMenuColor
//

CDialogMenuColor::CDialogMenuColor(CDialogMenuPopup* pParentPopup, UINT CmdID, COLORREF* pColor)
	: CDialogMenuCommand(pParentPopup, CmdID, 0, CDMB_SMALL)
{
	m_Caption.Append(_T("..."));
	p_Color = pColor;

	pParentPopup->GetCheckSize(m_IconSize);
}

void CDialogMenuColor::Execute()
{
	p_ParentPopup->GetOwner()->PostMessage(WM_CLOSEPOPUP);

	CString Caption = m_Caption.Left(m_Caption.GetLength()-3);
	Caption.Remove(L'&');
	FMGetApp()->ChooseColor(p_Color, p_ParentPopup->GetOwner(), Caption);
}

void CDialogMenuColor::OnDrawIcon(CDC* pDC, CPoint pt, BOOL /*Selected*/, BOOL Themed)
{
	CRect rect(pt.x, pt.y, pt.x+m_IconSize.cx, pt.y+m_IconSize.cy);

	COLORREF clr = (Themed && !m_Enabled) ? 0xB1B1B1 : 0x000000;
	pDC->Draw3dRect(rect, clr, clr);

	rect.DeflateRect(1, 1);
	pDC->FillSolidRect(rect, *p_Color);
}


// CDialogMenuResolution
//

CDialogMenuResolution::CDialogMenuResolution(CDialogMenuPopup* pParentPopup, UINT CmdID, INT IconID, UINT* pWidth, UINT* pHeight)
	: CDialogMenuCommand(pParentPopup, CmdID, IconID, CDMB_SMALL)
{
	CString tmpStr;
	tmpStr.Format(_T("%s (%u×%u)..."), m_Caption, *pWidth, *pHeight);
	m_Caption = tmpStr;

	p_Width = pWidth;
	p_Height = pHeight;
}

void CDialogMenuResolution::Execute()
{
	CWnd* pOwner = p_ParentPopup->GetOwner();
	pOwner->PostMessage(WM_CLOSEPOPUP);

	FMResolutionDlg dlg(p_Width, p_Height, pOwner);
	dlg.DoModal();
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

INT CDialogMenuSeparator::GetOuterBorder()
{
	return 0;
}

void CDialogMenuSeparator::OnPaint(CDC* pDC, LPRECT rect, BOOL /*Selected*/, UINT Themed)
{
	INT l = rect->right-rect->left;

	if (!m_ForBlueArea)
	{
		INT left = rect->left+p_ParentPopup->GetGutter()+BORDERPOPUP+MARGIN;
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
		if (FMGetApp()->OSVersion==OS_Eight)
		{
			pDC->FillSolidRect(rect->left, rect->top+1, l, 2, GetSysColor(COLOR_3DFACE));
		}
		else
			if (Themed)
			{
				pDC->FillSolidRect(rect->left, rect->top, l, 1, 0xF1E1DA);
				pDC->FillSolidRect(rect->left, rect->top+1, l, 1, 0xF4EAE3);
				pDC->FillSolidRect(rect->left, rect->top+2, l, 1, 0xF9F0EC);
			}
}


// CDialogMenuCaption
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

INT CDialogMenuCaption::GetOuterBorder()
{
	return 0;
}

void CDialogMenuCaption::OnPaint(CDC* pDC, LPRECT rect, BOOL /*Selected*/, UINT Themed)
{
	CFont* pOldFont = p_ParentPopup->SelectCaptionFont(pDC);

	CRect rectText(rect);
	rectText.DeflateRect(0, BORDERPOPUP);

	if (Themed && FMGetApp()->OSVersion!=OS_Eight)
	{
		INT l = rectText.Width();
		INT h = rectText.Height();

		pDC->FillSolidRect(rectText.left, rectText.top, l, 1, 0xC5C5C5);
		pDC->FillSolidRect(rectText.left, rectText.top+1, l, h-2, 0xEEE7DD);
		pDC->FillSolidRect(rectText.left, rectText.top+h-1, l, 1, 0xC5C5C5);
	}

	rectText.DeflateRect(BORDER+BORDERPOPUP, Themed ? 1 : 0);

	pDC->SetTextColor(Themed ? FMGetApp()->OSVersion!=OS_Eight ? 0x6E1500 : GetSysColor(COLOR_MENUTEXT) : GetSysColor(COLOR_HIGHLIGHT));
	pDC->DrawText(m_Caption, rectText, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE);

	pDC->SelectObject(pOldFont);
}
