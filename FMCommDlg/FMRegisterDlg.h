
// FMRegisterDlg.h: Schnittstelle der Klasse FMRegisterDlg
//

#pragma once
#include "CPictureCtrl.h"
#include "FMDialog.h"
#include "FMFont.h"


// FMRegisterDlg
//

#define MAXREGISTERITEMS     7

class FMRegisterDlg : public FMDialog
{
public:
	FMRegisterDlg(CWnd* pParentWnd=NULL);

	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);
	virtual BOOL InitDialog();

protected:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnterLicenseKey(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPurchase();
	DECLARE_MESSAGE_MAP()

	CString m_RegisterItems[MAXREGISTERITEMS];
	FMFont m_ArrowFont;
	INT m_ItemHeight;
	INT m_PictureHeight;
	INT m_Indent;

private:
	void CheckInternetConnection();

	CPictureCtrl m_wndPictureUnregistered;
	CPictureCtrl m_wndPictureRegistered;
	CRect m_ArrowRect;
};
