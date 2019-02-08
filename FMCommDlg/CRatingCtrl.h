
// CRatingCtrl.h: Schnittstelle der Klasse CRatingCtrl
//

#pragma once
#include "CFrontstageWnd.h"


// CRatingCtrl
//

#define WM_RATINGCHANGED     WM_USER+3

class CRatingCtrl : public CFrontstageWnd
{
public:
	CRatingCtrl();

	void SetInitialRating(UCHAR Rating);
	UCHAR GetRating() const;

protected:
	void SetRating(UCHAR Rating);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()

	UCHAR m_Rating;
};

inline UCHAR CRatingCtrl::GetRating() const
{
	return m_Rating;
}

inline void CRatingCtrl::SetRating(UCHAR Rating)
{
	ASSERT(Rating<=MAXRATING);

	m_Rating = Rating;

	Invalidate();
	GetOwner()->SendMessage(WM_RATINGCHANGED);
}
