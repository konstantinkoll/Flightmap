
// CPictureCtrl.h: Schnittstelle der Klasse CPictureCtrl
//

#pragma once
#include "CGdiPlusBitmap.h"


// CPictureCtrl
//

class CPictureCtrl : public CWnd
{
public:
	CPictureCtrl();

	void SetPicture(UINT nResID, LPCTSTR Type);

protected:
	CGdiPlusBitmapResource m_Picture;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
