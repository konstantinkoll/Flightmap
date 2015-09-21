
// ChooseDetailsDlg.cpp: Implementierung der Klasse ChooseDetailsDlg
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"
#include "Flightmap.h"


// ChooseDetailsDlg
//

ChooseDetailsDlg::ChooseDetailsDlg(ViewParameters* pViewParameters, CWnd* pParentWnd)
	: CDialog(IDD_CHOOSEDETAILS, pParentWnd)
{
	p_ViewParameters = pViewParameters;
}

void ChooseDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_ShowAttributes);

	if (pDX->m_bSaveAndValidate)
	{
		// Breite
		INT OldWidth[FMAttributeCount];
		memcpy(&OldWidth, &p_ViewParameters->ColumnWidth, sizeof(OldWidth));
		ZeroMemory(&p_ViewParameters->ColumnWidth, sizeof(p_ViewParameters->ColumnWidth));

		for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		{
			const UINT Attr = (UINT)m_ShowAttributes.GetItemData(a);
			const BOOL show = m_ShowAttributes.GetCheck(a) || (Attr==0) || (Attr==3);
			p_ViewParameters->ColumnWidth[Attr] = show ? OldWidth[Attr] ? OldWidth[Attr] : FMAttributes[Attr].RecommendedWidth : 0;
		}

		// Reihenfolge
		UINT cnt = 0;
		for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		{
			const UINT Attr = (UINT)m_ShowAttributes.GetItemData(a);
			if (m_ShowAttributes.GetCheck(a) || (Attr==0) || (Attr==3))
				p_ViewParameters->ColumnOrder[cnt++] = Attr;
		}

		for (INT a=0; a<FMAttributeCount; a++)
			if (!p_ViewParameters->ColumnWidth[a])
				p_ViewParameters->ColumnOrder[cnt++] = a;
	}
}

void ChooseDetailsDlg::AddAttribute(UINT Attr)
{
	CString tmpStr((LPCSTR)FMAttributes[Attr].nNameID);

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.lParam = (LPARAM)Attr;
	lvi.pszText = tmpStr.GetBuffer();
	lvi.iItem = m_ShowAttributes.GetItemCount();

	m_ShowAttributes.SetCheck(m_ShowAttributes.InsertItem(&lvi), p_ViewParameters->ColumnWidth[Attr]!=0);
}

void ChooseDetailsDlg::SwapItems(INT FocusItem, INT NewPos)
{
	TCHAR text1[256];
	LVITEM i1;
	ZeroMemory(&i1, sizeof(LVITEM));
	i1.pszText = text1;
	i1.cchTextMax = sizeof(text1)/sizeof(TCHAR);
	i1.iItem = FocusItem;
	i1.mask = LVIF_TEXT | LVIF_PARAM;
	m_ShowAttributes.GetItem(&i1);

	TCHAR text2[256];
	LVITEM i2;
	ZeroMemory(&i2, sizeof(LVITEM));
	i2.pszText = text2;
	i2.cchTextMax = sizeof(text2)/sizeof(TCHAR);
	i2.iItem = NewPos;
	i2.mask = LVIF_TEXT | LVIF_PARAM;
	m_ShowAttributes.GetItem(&i2);

	INT iItem = i1.iItem;
	i1.iItem = i2.iItem;
	i2.iItem = iItem;

	m_ShowAttributes.SetItem(&i1);
	m_ShowAttributes.SetItem(&i2);

	BOOL Check1 = m_ShowAttributes.GetCheck(FocusItem);
	BOOL Check2 = m_ShowAttributes.GetCheck(NewPos);
	m_ShowAttributes.SetCheck(FocusItem, Check2);
	m_ShowAttributes.SetCheck(NewPos, Check1);

	m_ShowAttributes.SetItemState(NewPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}


BEGIN_MESSAGE_MAP(ChooseDetailsDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChange)
	ON_COMMAND(IDC_MOVEUP, OnMoveUp)
	ON_COMMAND(IDC_MOVEDOWN, OnMoveDown)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

BOOL ChooseDetailsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Kontrollelemente einstellen
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS | LVS_EX_CHECKBOXES;
	m_ShowAttributes.SetExtendedStyle(m_ShowAttributes.GetExtendedStyle() | dwExStyle);

	for (UINT a=0; a<FMAttributeCount; a++)
		if (p_ViewParameters->ColumnWidth[p_ViewParameters->ColumnOrder[a]])
			AddAttribute(p_ViewParameters->ColumnOrder[a]);

	for (UINT a=0; a<FMAttributeCount; a++)
		if (!p_ViewParameters->ColumnWidth[p_ViewParameters->ColumnOrder[a]])
			AddAttribute(p_ViewParameters->ColumnOrder[a]);

	m_ShowAttributes.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_ShowAttributes.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ChooseDetailsDlg::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	INT Index = (INT)pNMListView->iItem;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		GetDlgItem(IDC_MOVEUP)->EnableWindow(m_ShowAttributes.IsWindowEnabled() && (Index>0));
		GetDlgItem(IDC_MOVEDOWN)->EnableWindow(m_ShowAttributes.IsWindowEnabled() && (Index<m_ShowAttributes.GetItemCount()-1));
	}

	*pResult = 0;
}

void ChooseDetailsDlg::OnMoveUp()
{
	INT Index = m_ShowAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index>0)
		SwapItems(Index, Index-1);
}

void ChooseDetailsDlg::OnMoveDown()
{
	INT Index = m_ShowAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index<m_ShowAttributes.GetItemCount()-1)
		SwapItems(Index, Index+1);
}

void ChooseDetailsDlg::OnCheckAll()
{
	for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		m_ShowAttributes.SetCheck(a);
}

void ChooseDetailsDlg::OnUncheckAll()
{
	for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		m_ShowAttributes.SetCheck(a, FALSE);
}
