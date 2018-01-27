
// CMapWnd.h: Schnittstelle der Klasse CMapWnd
//

#pragma once
#include "CKitchen.h"
#include "CMapView.h"
#include "FMCommDlg.h"


// CMapWnd
//

class CMapWnd : public CBackstageWnd
{
public:
	CMapWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);

	BOOL Create();
	void SetBitmap(CBitmap* pBitmap, CItinerary* pItinerary);

protected:
	void PrintMap(PRINTDLGEX pdex);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnMapWndSaveAs();
	afx_msg void OnMapWndPrint();
	afx_msg void OnMapWndCopy();
	afx_msg void OnUpdateMapWndCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CTaskbar m_wndTaskbar;
	CMapView m_wndMapView;
	CBitmap* m_pBitmap;

private:
	CString m_Title;
};
