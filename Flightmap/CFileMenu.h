
// CFileMenu.h: Schnittstelle der Klasse CFileMenu
//

#pragma once
#include "FMCommDlg.h"
#include "CItinerary.h"
#include "CRecentFilesPane.h"


// CFileMenu
//

class CFileMenu : public CFrontstageWnd
{
public:
	CFileMenu();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL Create(CWnd* pParentWnd, UINT nID, CItinerary* pItinerary=NULL);
	LPCWSTR GetSelectedFilePath() const;

protected:
	void Update();
	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSettingChange(UINT nFlags, LPCTSTR lpszSection);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnAdjustLayout();

	afx_msg void OnFileViewInFolder();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintQuick();
	afx_msg void OnFileProperties();
	afx_msg void OnFileAttachments();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CItinerary* p_Itinerary;
	CHeaderArea m_wndHeaderArea;
	CFloatButtons m_wndFloatButtons;
	CRecentFilesPane m_wndRecentFilesPane;

private:
	BOOL m_DefaultPrinterAvailable;
	BOOL m_Resizing;
};

inline LPCWSTR CFileMenu::GetSelectedFilePath() const
{
	return m_wndRecentFilesPane.GetSelectedFilePath();
}

