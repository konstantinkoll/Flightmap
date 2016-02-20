
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
	UINT Index;
};

class FMTabbedDialog : public FMDialog
{
public:
	FMTabbedDialog(UINT nCaptionID, CWnd* pParentWnd=NULL, UINT* pLastTab=NULL);

protected:
	virtual void ShowTab(UINT Index);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	BOOL AddTab(const CString& Caption);
	BOOL AddTab(UINT nResID, LPSIZE pszTabArea);
	void SelectTab(UINT Index);

	afx_msg void OnDestroy();
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnSelectTab(UINT nCmdID);
	afx_msg void OnUpdateTabCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	UINT m_CurrentTab;

private:
	CBackstageSidebar m_wndSidebar;
	CString m_Caption;
	UINT m_TabCount;
	UINT* p_LastTab;
	FMDynArray<ControlOnTab, 16, 16>m_ControlsOnTab;
	WCHAR m_TabHints[MAXTABS][4096];
};
