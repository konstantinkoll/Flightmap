
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "FMCommDlg.h"


// CFileView
//

class CFileView : public CWnd
{
public:
	CFileView();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

protected:
	CTaskbar m_wndTaskbar;
	CExplorerList m_wndExplorerList;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()

private:
	void Init();
};
