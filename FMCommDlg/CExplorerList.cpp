
// CExplorerList.cpp: Implementierung der Klasse CExplorerList
//

#include "stdafx.h"
#include "CExplorerList.h"


// CExplorerList
//

CExplorerList::CExplorerList()
	: CListCtrl()
{
	p_App = (FMApplication*)AfxGetApp();
	hTheme = NULL;
}

void CExplorerList::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CExplorerList::Init()
{
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}
}

void CExplorerList::AddCategory(INT ID, CString Name, CString Hint, BOOL Collapsable)
{
	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));

	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.iGroupId = ID;
	lvg.pszHeader = Name.GetBuffer();
	if (p_App->OSVersion>=OS_Vista)
	{
		if (!Hint.IsEmpty())
		{
			lvg.pszSubtitle = Hint.GetBuffer();
			lvg.mask |= LVGF_SUBTITLE;
		}
		if (Collapsable)
		{
			lvg.stateMask = LVGS_COLLAPSIBLE;
			lvg.mask |= LVGF_STATE;
		}
	}

	InsertGroup(ID, &lvg);
}

void CExplorerList::AddColumn(INT ID, CString Name)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = Name.GetBuffer();
	lvc.iSubItem = ID;
	
	InsertColumn(ID, &lvc);
}


BEGIN_MESSAGE_MAP(CExplorerList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

INT CExplorerList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CExplorerList::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CListCtrl::OnDestroy();
}

LRESULT CExplorerList::OnThemeChanged()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return TRUE;
}
