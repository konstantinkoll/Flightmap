
// CAchievementNotification.h: Schnittstelle der Klasse CAchievementNotification
//

#pragma once
#include "FMCommDlg.h"


// CAchievementNotification
//

class CAchievementNotification : public CWnd
{
public:
	CAchievementNotification();

	BOOL Create();

protected:
	void UpdateFrame();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void PostNcDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()
};
