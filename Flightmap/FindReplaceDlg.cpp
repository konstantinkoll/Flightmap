
// FindReplaceDlg.cpp: Implementierung der Klasse FindReplaceDlg
//

#include "stdafx.h"
#include "FindReplaceDlg.h"
#include "Flightmap.h"


// FindReplaceDlg
//

#define ONTAB(nID) (Index==nID) ? SW_SHOW : SW_HIDE

UINT FindReplaceDlg::m_LastTab = 0;

FindReplaceDlg::FindReplaceDlg(UINT Attr, CWnd* pParentWnd, INT SelectTab)
	: FMTabbedDialog(IDS_FINDREPLACE, pParentWnd, &m_LastTab)
{
	m_FindReplaceSettings = theApp.m_FindReplaceSettings;

	m_AllowColumnOnly[0] = FMAttributes[Attr].Searchable;
	m_AllowColumnOnly[1] = FMAttributes[Attr].Searchable && FMAttributes[Attr].Editable;

	if (SelectTab!=-1)
		m_LastTab = SelectTab;
}

void FindReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	FMTabbedDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SEARCHTERM, m_wndSearchTerm);
	DDX_Control(pDX, IDC_REPLACETERM, m_wndReplaceTerm);
	DDX_Control(pDX, IDC_MATCHCASE, m_wndMatchCase);
	DDX_Control(pDX, IDC_MATCHENTIRECELL, m_wndMatchEntireCell);
	DDX_Control(pDX, IDC_MATCHCOLUMNONLY, m_wndMatchColumnOnly);
	DDX_Control(pDX, IDC_REPLACEALL, m_wndReplaceAll);

	if (pDX->m_bSaveAndValidate)
	{
		m_FindReplaceSettings.DoReplace = (m_CurrentTab==1);
		m_FindReplaceSettings.FirstAction = TRUE;

		m_wndSearchTerm.GetWindowText(m_FindReplaceSettings.SearchTerm, 256);

		m_FindReplaceSettings.Flags &= ~(FRS_MATCHCASE | FRS_MATCHENTIRECELL | FRS_MATCHCOLUMNONLY);

		if (m_wndMatchCase.GetCheck())
			m_FindReplaceSettings.Flags |= FRS_MATCHCASE;

		if (m_wndMatchEntireCell.GetCheck())
			m_FindReplaceSettings.Flags |= FRS_MATCHENTIRECELL;

		if (m_wndMatchColumnOnly.GetCheck())
			m_FindReplaceSettings.Flags |= FRS_MATCHCOLUMNONLY;

		if (m_FindReplaceSettings.DoReplace)
		{
			m_wndReplaceTerm.GetWindowText(m_FindReplaceSettings.ReplaceTerm, 256);

			m_FindReplaceSettings.Flags &= ~FRS_REPLACEALL;

			if (m_wndReplaceAll.GetCheck())
				m_FindReplaceSettings.Flags |= FRS_REPLACEALL;
		}
	}
	else
	{
		m_wndSearchTerm.SetWindowText(m_FindReplaceSettings.SearchTerm);
		for (POSITION p=theApp.m_RecentSearchTerms.GetHeadPosition(); p; )
			m_wndSearchTerm.AddString(theApp.m_RecentSearchTerms.GetNext(p));

		m_wndReplaceTerm.SetWindowText(m_FindReplaceSettings.ReplaceTerm);
		for (POSITION p=theApp.m_RecentReplaceTerms.GetHeadPosition(); p; )
			m_wndReplaceTerm.AddString(theApp.m_RecentReplaceTerms.GetNext(p));

		m_wndMatchCase.SetCheck(m_FindReplaceSettings.Flags & FRS_MATCHCASE);
		m_wndMatchEntireCell.SetCheck(m_FindReplaceSettings.Flags & FRS_MATCHENTIRECELL);
		m_wndMatchColumnOnly.SetCheck(m_FindReplaceSettings.Flags & FRS_MATCHCOLUMNONLY);
		m_wndReplaceAll.SetCheck(m_FindReplaceSettings.Flags & FRS_REPLACEALL);
	}
}

void FindReplaceDlg::ShowTab(UINT Index)
{
	ASSERT(Index<=1);

	m_wndCategory[0].ShowWindow(ONTAB(0));
	m_wndCategory[1].ShowWindow(ONTAB(1));

	GetDlgItem(IDC_REPLACECAPTION)->ShowWindow(ONTAB(1));
	m_wndReplaceTerm.ShowWindow(ONTAB(1));
	m_wndReplaceAll.ShowWindow(ONTAB(1));

	m_wndMatchColumnOnly.EnableWindow(m_AllowColumnOnly[Index]);

	m_wndSearchTerm.SetFocus();
}

BOOL FindReplaceDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!FMTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_FIND, pszTabArea);
	AddTab(IDD_REPLACE, pszTabArea);

	return TRUE;
}
