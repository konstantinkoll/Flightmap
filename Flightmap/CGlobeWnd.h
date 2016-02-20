
// CGlobeWnd.h: Schnittstelle der Klasse CGlobeWnd
//

#pragma once
#include "CGlobeView.h"
#include "CKitchen.h"
#include "FMCommDlg.h"


// CGlobeWnd
//

class CGlobeWnd : public CBackstageWnd
{
public:
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);

	BOOL Create();
	void SetFlights(CKitchen* pKitchen);

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void On3DSettingsChanged();
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CTaskbar m_wndTaskbar;
	CGlobeView m_wndGlobeView;
};
