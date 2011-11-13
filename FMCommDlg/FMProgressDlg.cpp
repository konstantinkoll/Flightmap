
// FMProgressDlg.cpp: Implementierung der Klasse FMProgressDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMProgressDlg
//

FMProgressDlg::FMProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, FMWorkerParameters* pParameters, CWnd* pParent)
	: FMDialog(IDD_PROGRESS, FMDS_White, pParent)
{
	m_Abort = FALSE;

	p_ThreadProc = pThreadProc;
	p_Parameters = pParameters;

	ENSURE(m_XofY_Singular.LoadString(IDS_XOFY_SINGULAR));
	ENSURE(m_XofY_Plural.LoadString(IDS_XOFY_PLURAL));
}

void FMProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROGRESSBAR, m_wndProgress);
}


BEGIN_MESSAGE_MAP(FMProgressDlg, FMDialog)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_UPDATEPROGRESS, OnUpdateProgress)
END_MESSAGE_MAP()

BOOL FMProgressDlg::OnInitDialog()
{
	FMDialog::OnInitDialog();

	// Strip
	if (!((FMApplication*)AfxGetApp())->m_ReduceVisuals)
	{
		if (!ProgressStrip)
		{
			ProgressStrip = new CGdiPlusBitmapResource();
			ENSURE(ProgressStrip->Load(IDB_PROGRESSSTRIP, _T("JPG"), AfxGetResourceHandle()));
		}

		CRect rectDialog;
		GetWindowRect(rectDialog);

		CRect rectProgress;
		m_wndProgress.GetWindowRect(rectProgress);
		ScreenToClient(rectProgress);

		rectProgress.top += rectProgress.Height()/2;

		if (m_wndStrip.Create(this, IDC_PROGRESSSTRIP))
		{
			const INT AddWidth = 250;
			INT StripWidth =  rectDialog.Width()+AddWidth;
			INT StripHeight = ProgressStrip->m_pBitmap->GetHeight();
			INT AddHeight = rectProgress.top+StripHeight;
			m_wndStrip.SetWindowPos(NULL, 0, rectProgress.top, StripWidth, StripHeight, SWP_NOACTIVATE | SWP_NOZORDER);
			m_wndStrip.SetBitmap(ProgressStrip);

			// Resize dialog
			rectDialog.right += AddWidth;
			rectDialog.bottom += AddHeight;
			SetWindowPos(NULL, rectDialog.left, rectDialog.top, rectDialog.Width(), rectDialog.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

			// Resize caption line
			CRect rect;
			GetDlgItem(IDC_PROGRESSCOUNT)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowPos(NULL, rect.left, rect.top+AddHeight, rect.Width()+AddWidth, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

			// Resize cancel button
			GetDlgItem(IDCANCEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDCANCEL)->SetWindowPos(NULL, rect.left+AddWidth, rect.top+AddHeight, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

			// Resize progress bar
			m_wndProgress.SetWindowPos(NULL, rectProgress.left, rectProgress.top+AddHeight, rectProgress.Width()+AddWidth, rectProgress.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	// Text
	if (p_Parameters)
		SetWindowText(p_Parameters->Caption);

	GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowText(_T(""));

	// Thread
	if (p_ThreadProc)
	{
		ASSERT(p_Parameters);
		p_Parameters->hWnd = GetSafeHwnd();

		CreateThread(NULL, 0, p_ThreadProc, p_Parameters, 0, NULL);
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void FMProgressDlg::OnCancel()
{
	if (p_ThreadProc)
	{
		m_Abort = TRUE;
		GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	}
	else
	{
		EndDialog(IDCANCEL);
	}
}

LRESULT FMProgressDlg::OnUpdateProgress(WPARAM wParam, LPARAM /*lParam*/)
{
	FMProgress* pProgress = (FMProgress*)wParam;

	if (m_Abort)
	{
		pProgress->UserAbort = true;
		if (pProgress->ProgressState==FMProgressWorking)
			pProgress->ProgressState = FMProgressCancelled;
	}

	// Progress bar
	ASSERT(pProgress->ProgressState>=FMProgressWorking);
	ASSERT(pProgress->ProgressState<=FMProgressCancelled);
	m_wndProgress.SendMessage(0x410, pProgress->ProgressState);

	ASSERT(pProgress->Count>0);
	ASSERT(pProgress->Current>=0);
	ASSERT(pProgress->Current<=pProgress->Count);

	UINT nCurrent = pProgress->Current+1;
	UINT nOf = pProgress->NoCounter ? 0 : pProgress->Count;

	m_wndProgress.SetRange32(0, pProgress->Count);
	m_wndProgress.SetPos(pProgress->Current);

	// Counter
	CString tmpStr(_T(""));
	if ((nCurrent<=nOf) && (!m_Abort))
		tmpStr.Format(nOf==1 ? m_XofY_Singular : m_XofY_Plural, nCurrent, nOf);

	if (m_LastCounter!=tmpStr)
	{
		GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowText(tmpStr);
		m_LastCounter = tmpStr;
	}

	return m_Abort;
}
