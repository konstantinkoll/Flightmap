
// FMProgressDlg.h: Schnittstelle der Klasse FMProgressDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "FMDialog.h"


// FMProgressDlg
//

#define WM_UPDATEPROGRESS     WM_USER

#define FMProgressWorking     1
#define FMProgressError       2
#define FMProgressCancelled   3

struct FMProgress
{
	UCHAR ProgressState;
	UINT Current;						// Starting from 0, must not exceed max(0, MajorCount-1)
	UINT Count;							// Must be 1 or higher
	BOOL UserAbort;						// Set true if aborted by user
	BOOL NoCounter;						// Set true if thread does not wish progress counter to be displayed
};

struct FMWorkerParameters
{
	HWND hWnd;
	WCHAR Caption[256];
};

static CGdiPlusBitmapResource* ProgressStrip = NULL;

class FMProgressDlg : public FMDialog
{
public:
	FMProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, FMWorkerParameters* pParameters, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CString m_XofY_Singular;
	CString m_XofY_Plural;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Abort;
	LPTHREAD_START_ROUTINE p_ThreadProc;
	FMWorkerParameters* p_Parameters;
	CStripCtrl m_wndStrip;
	CProgressCtrl m_wndProgress;
	CString m_LastCounter;
};
