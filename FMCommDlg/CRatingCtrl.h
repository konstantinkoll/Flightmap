
// CRatingCtrl.h: Schnittstelle der Klasse CRatingCtrl
//

#pragma once
#include "CFrontstageWnd.h"


// CRatingCtrl
//

#define WM_RATINGCHANGED     WM_USER+9

class CRatingCtrl : public CFrontstageWnd
{
public:
	CRatingCtrl();

	void SetRating(UCHAR Rating, BOOL Prepare=TRUE);
	UCHAR GetRating() const;

protected:
	void SendChangeMessage() const;

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()

	UCHAR m_Rating;
};
