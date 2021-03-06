
// FMTabbedDialog.h: Schnittstelle der Klasse FMTabbedDialog
//

#pragma once
#include "CBackstageSidebar.h"
#include "FMDialog.h"
#include "FMDynArray.h"


// FMTabbedDialog
//

#define MAXTABS     16

struct ControlOnTab
{
	HWND hWnd;
	USHORT TabMask;
};

class FMTabbedDialog : public FMDialog
{
public:
	FMTabbedDialog(UINT nCaptionID, CWnd* pParentWnd=NULL, UINT* pLastTab=NULL, BOOL WantsBitmap=FALSE);

protected:
	virtual void ShowTab(UINT Index);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	BOOL AddTab(const CString& Caption);
	BOOL AddTab(UINT nResID, LPSIZE pszTabArea);
	void AddControl(const ControlOnTab& Ctrl);
	void AddControl(HWND hWnd, UINT Index);
	void ShowControlOnTabs(HWND hWnd, USHORT Mask);
	void SelectTab(UINT Index);

	afx_msg void OnDestroy();
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnSelectTab(UINT nCmdID);
	afx_msg void OnUpdateTabCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CString m_DialogCaption;
	UINT m_CurrentTab;

private:
	CBackstageSidebar m_wndSidebar;
	UINT m_TabCount;
	UINT* p_LastTab;
	FMDynArray<ControlOnTab, 16, 16>m_ControlsOnTab;
	WCHAR m_TabHints[MAXTABS][4096];
};

inline void FMTabbedDialog::AddControl(const ControlOnTab& Ctrl)
{
	m_ControlsOnTab.AddItem(Ctrl);
}
