
// FindReplacePage.cpp: Implementierung der Klasse FindReplacePage
//

#include "stdafx.h"
#include "FindReplacePage.h"
#include "Flightmap.h"


// FindReplacePage
//

FindReplacePage::FindReplacePage(FindReplaceSettings* pFindReplaceSettings, BOOL AllowColumnOnly)
	: CPropertyPage()
{
	ASSERT(pFindReplaceSettings);

	p_FindReplaceSettings = pFindReplaceSettings;
	m_AllowColumnOnly = AllowColumnOnly;
}

void FindReplacePage::DoDataExchange(CDataExchange* pDX)
{
	BOOL Replace = (m_psp.pszTemplate==MAKEINTRESOURCE(IDD_REPLACE));

	DDX_Control(pDX, IDC_SEARCHTERM, m_wndSearchTerm);
	DDX_Control(pDX, IDC_MATCHCASE, m_wndMatchCase);
	DDX_Control(pDX, IDC_MATCHENTIRECELL, m_wndMatchEntireCell);
	DDX_Control(pDX, IDC_MATCHCOLUMNONLY, m_wndMatchColumnOnly);

	if (Replace)
	{
		DDX_Control(pDX, IDC_REPLACETERM, m_wndReplaceTerm);
		DDX_Control(pDX, IDC_REPLACEALL, m_wndReplaceAll);
	}

	if (pDX->m_bSaveAndValidate)
	{
		m_wndSearchTerm.GetWindowText(p_FindReplaceSettings->SearchTerm, 256);

		p_FindReplaceSettings->Flags &= ~(FRS_MATCHCASE | FRS_MATCHENTIRECELL | FRS_MATCHCOLUMNONLY);
		if (m_wndMatchCase.GetCheck())
			p_FindReplaceSettings->Flags |= FRS_MATCHCASE;
		if (m_wndMatchEntireCell.GetCheck())
			p_FindReplaceSettings->Flags |= FRS_MATCHENTIRECELL;
		if (m_wndMatchColumnOnly.GetCheck())
			p_FindReplaceSettings->Flags |= FRS_MATCHCOLUMNONLY;

		if (Replace)
		{
			m_wndReplaceTerm.GetWindowText(p_FindReplaceSettings->ReplaceTerm, 256);

			p_FindReplaceSettings->Flags &= ~FRS_REPLACEALL;
			if (m_wndReplaceAll.GetCheck())
				p_FindReplaceSettings->Flags |= FRS_REPLACEALL;
		}

		p_FindReplaceSettings->DoReplace = Replace;
		p_FindReplaceSettings->FirstAction = TRUE;
	}
	else
	{
		m_wndSearchTerm.SetWindowText(p_FindReplaceSettings->SearchTerm);
		for (POSITION p=theApp.m_RecentSearchTerms.GetHeadPosition(); p; )
			m_wndSearchTerm.AddString(theApp.m_RecentSearchTerms.GetNext(p));

		m_wndMatchCase.SetCheck(p_FindReplaceSettings->Flags & FRS_MATCHCASE);
		m_wndMatchEntireCell.SetCheck(p_FindReplaceSettings->Flags & FRS_MATCHENTIRECELL);
		m_wndMatchColumnOnly.SetCheck(m_AllowColumnOnly && (p_FindReplaceSettings->Flags & FRS_MATCHCOLUMNONLY));

		if (Replace)
		{
			m_wndReplaceTerm.SetWindowText(p_FindReplaceSettings->ReplaceTerm);
			for (POSITION p=theApp.m_RecentReplaceTerms.GetHeadPosition(); p; )
				m_wndReplaceTerm.AddString(theApp.m_RecentReplaceTerms.GetNext(p));

			m_wndReplaceAll.SetCheck(p_FindReplaceSettings->Flags & FRS_REPLACEALL);
		}
	}
}


BEGIN_MESSAGE_MAP(FindReplacePage, CPropertyPage)
END_MESSAGE_MAP()

BOOL FindReplacePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndMatchColumnOnly.EnableWindow(m_AllowColumnOnly);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
