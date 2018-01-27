
// ChooseDetailsDlg.cpp: Implementierung der Klasse ChooseDetailsDlg
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"


// ChooseDetailsDlg
//

ChooseDetailsDlg::ChooseDetailsDlg(ViewParameters* pViewParameters, CWnd* pParentWnd)
	: FMDialog(IDD_CHOOSEDETAILS, pParentWnd)
{
	p_ViewParameters = pViewParameters;
}

void ChooseDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_wndAttributes);

	if (pDX->m_bSaveAndValidate)
	{
		// Width
		INT OldWidth[FMAttributeCount];
		memcpy(&OldWidth, &p_ViewParameters->ColumnWidth, sizeof(OldWidth));
		ZeroMemory(&p_ViewParameters->ColumnWidth, sizeof(p_ViewParameters->ColumnWidth));

		UINT Index = 0;

		for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		{
			const UINT Attr = (UINT)m_wndAttributes.GetItemData(a);

			if (m_wndAttributes.GetCheck(a) || (Attr==0) || (Attr==3))
			{
				p_ViewParameters->ColumnWidth[Attr] = OldWidth[Attr] ? OldWidth[Attr] : FMAttributes[Attr].DefaultColumnWidth;
				p_ViewParameters->ColumnOrder[Index++] = Attr;
			}
			else
			{
				p_ViewParameters->ColumnWidth[Attr] = 0;
			}
		}

		// Order
		for (INT a=0; a<FMAttributeCount; a++)
			if (!p_ViewParameters->ColumnWidth[a])
				p_ViewParameters->ColumnOrder[Index++] = a;
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
	lvi.iItem = m_wndAttributes.GetItemCount();

	m_wndAttributes.SetCheck(m_wndAttributes.InsertItem(&lvi), p_ViewParameters->ColumnWidth[Attr]!=0);
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
	m_wndAttributes.GetItem(&i1);

	TCHAR text2[256];
	LVITEM i2;
	ZeroMemory(&i2, sizeof(LVITEM));
	i2.pszText = text2;
	i2.cchTextMax = sizeof(text2)/sizeof(TCHAR);
	i2.iItem = NewPos;
	i2.mask = LVIF_TEXT | LVIF_PARAM;
	m_wndAttributes.GetItem(&i2);

	INT iItem = i1.iItem;
	i1.iItem = i2.iItem;
	i2.iItem = iItem;

	m_wndAttributes.SetItem(&i1);
	m_wndAttributes.SetItem(&i2);

	BOOL Check1 = m_wndAttributes.GetCheck(FocusItem);
	BOOL Check2 = m_wndAttributes.GetCheck(NewPos);
	m_wndAttributes.SetCheck(FocusItem, Check2);
	m_wndAttributes.SetCheck(NewPos, Check1);

	m_wndAttributes.SetItemState(NewPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

BOOL ChooseDetailsDlg::InitDialog()
{
	// Kontrollelemente einstellen
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS | LVS_EX_CHECKBOXES;
	m_wndAttributes.SetExtendedStyle(m_wndAttributes.GetExtendedStyle() | dwExStyle);

	m_wndAttributes.AddColumn(0);

	for (UINT a=0; a<FMAttributeCount; a++)
		if (p_ViewParameters->ColumnWidth[p_ViewParameters->ColumnOrder[a]])
			AddAttribute(p_ViewParameters->ColumnOrder[a]);

	for (UINT a=0; a<FMAttributeCount; a++)
		if (!p_ViewParameters->ColumnWidth[p_ViewParameters->ColumnOrder[a]])
			AddAttribute(p_ViewParameters->ColumnOrder[a]);

	m_wndAttributes.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndAttributes.SetColumnWidth(0, LVSCW_AUTOSIZE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(ChooseDetailsDlg, FMDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChange)
	ON_COMMAND(IDC_MOVEUP, OnMoveUp)
	ON_COMMAND(IDC_MOVEDOWN, OnMoveDown)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

void ChooseDetailsDlg::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		GetDlgItem(IDC_MOVEUP)->EnableWindow(pNMListView->iItem>0);
		GetDlgItem(IDC_MOVEDOWN)->EnableWindow(pNMListView->iItem<m_wndAttributes.GetItemCount()-1);
	}

	*pResult = 0;
}

void ChooseDetailsDlg::OnMoveUp()
{
	const INT Index = m_wndAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index>0)
		SwapItems(Index, Index-1);
}

void ChooseDetailsDlg::OnMoveDown()
{
	const INT Index = m_wndAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index<m_wndAttributes.GetItemCount()-1)
		SwapItems(Index, Index+1);
}

void ChooseDetailsDlg::OnCheckAll()
{
	for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		m_wndAttributes.SetCheck(a);
}

void ChooseDetailsDlg::OnUncheckAll()
{
	for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		m_wndAttributes.SetCheck(a, FALSE);
}
