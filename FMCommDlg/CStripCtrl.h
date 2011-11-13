
// CStripCtrl.h: Schnittstelle der Klasse CStripCtrl
//

#pragma once
#include "CGdiPlusBitmap.h"


// CStripCtrl
//

class CStripCtrl : public CWnd
{
public:
	CStripCtrl();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetBitmap(CGdiPlusBitmap* pStrip);

protected:
	CGdiPlusBitmap* p_Strip;
	INT m_Offset;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};
