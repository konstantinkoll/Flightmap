
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "CItinerary.h"
#include "FMCommDlg.h"


// CFileView
//

class CFileView : public CWnd
{
public:
	CFileView();

	virtual void PreSubclassWindow();
	virtual void AdjustLayout();

	void SetData(CItinerary* pItinerary, AIRX_Flight* pFlight=NULL);

protected:
	CItinerary* p_Itinerary;
	AIRX_Flight* p_Flight;
	CTaskbar m_wndTaskbar;
	CExplorerList m_wndExplorerList;

	void Reload();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);

	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	void Init();
};
