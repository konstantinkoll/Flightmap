
// MapSettingsDlg.h: Schnittstelle der Klasse MapSettingsDlg
//

#pragma once
#include "FMCommDlg.h"


// CResolutionList
//

#define RESOLUTIONCOLUMNS         2
#define RESOLUTIONPRESETCOUNT     34

struct ResolutionPreset
{
	UINT Width;
	UINT Height;
	WCHAR Hint[24];
	INT IconID;
};

class CResolutionList sealed : public CFrontstageItemView
{
public:
	CResolutionList();

	BOOL SetResolutions(UINT Width, UINT Height);
	BOOL GetSelectedResolution(UINT& Width, UINT& Height) const;

protected:
	virtual void UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const;
	virtual void AdjustLayout();
	virtual void DrawItemCell(CDC& dc, CRect& rectCell, INT Index, UINT Attr, BOOL Themed);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	void UpdateHeader();

	static CIcons m_ResolutionPresetIcons;
	static const ResolutionPreset m_ResolutionPresets[RESOLUTIONPRESETCOUNT];
	INT m_ColumnOrder[RESOLUTIONCOLUMNS];
	INT m_ColumnWidth[RESOLUTIONCOLUMNS];
};

inline void CResolutionList::UpdateHeader()
{
	CFrontstageItemView::UpdateHeader(m_ColumnOrder, m_ColumnWidth);
}

inline BOOL CResolutionList::GetSelectedResolution(UINT& Width, UINT& Height) const
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		Width = m_ResolutionPresets[Index].Width;
		Height = m_ResolutionPresets[Index].Height;

		return TRUE;
	}

	return FALSE;
}


// MapSettingsDlg
//

class MapSettingsDlg : public FMTabbedDialog
{
public:
	MapSettingsDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	void ChooseColor(COLORREF* pColor, BOOL AllowReset, CColorIndicator* pColorIndicator);
	BOOL GetSelectedResolution(UINT& Width, UINT& Height) const;

	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUserDefinedResolution();

	afx_msg void OnBackground();
	afx_msg void OnChooseColorBackground();

	afx_msg void OnChangeMergeMetropolitan();
	afx_msg void OnShowLocations();
	afx_msg void OnChooseColorLocationsInner();
	afx_msg void OnChooseColorLocationsOuter();
	afx_msg void OnShowIATACodes();
	afx_msg void OnChooseColorIATACodesInner();
	afx_msg void OnChooseColorIATACodesOuter();

	afx_msg void OnShowRoutes();
	afx_msg void OnChooseColorRoute();
	afx_msg void OnChooseColorNoteInner();
	afx_msg void OnChooseColorNoteOuter();
	DECLARE_MESSAGE_MAP()

	static UINT m_LastTab;
	COLORREF m_clrBackground;
	COLORREF m_clrLocationsInner;
	COLORREF m_clrLocationsOuter;
	COLORREF m_clrIATACodesInner;
	COLORREF m_clrIATACodesOuter;
	COLORREF m_clrRoute;
	COLORREF m_clrNoteInner;
	COLORREF m_clrNoteOuter;

private:
	CResolutionList m_wndResolutionList;
	CMFCMaskedEdit m_wndEditWidth;
	CMFCMaskedEdit m_wndEditHeight;
	CComboBox m_wndBackground;
	CColorIndicator m_wndColorIndicatorBackground;
	CPictureCtrl m_wndBackgroundPreview;
	CPictureCtrl m_wndMetropolitanPreview;
	CColorIndicator m_wndColorIndicatorLocationsInner;
	CColorIndicator m_wndColorIndicatorLocationsOuter;
	CColorIndicator m_wndColorIndicatorIATACodesInner;
	CColorIndicator m_wndColorIndicatorIATACodesOuter;
	CColorIndicator m_wndColorIndicatorRoute;
	CColorIndicator m_wndColorIndicatorNoteInner;
	CColorIndicator m_wndColorIndicatorNoteOuter;
};

inline void MapSettingsDlg::ChooseColor(COLORREF* pColor, BOOL AllowReset, CColorIndicator* pColorIndicator)
{
	ASSERT(pColor);
	ASSERT(pColorIndicator);

	theApp.ChooseColor(pColor, this, AllowReset);

	pColorIndicator->SetColor(*pColor);
}

inline BOOL MapSettingsDlg::GetSelectedResolution(UINT& Width, UINT& Height) const
{
	return m_wndResolutionList.GetSelectedResolution(Width, Height);
}
