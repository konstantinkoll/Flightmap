
// FMMessageBoxDlg.h: Schnittstelle der Klasse FMMessageBoxDlg
//

#pragma once
#include "FMDialog.h"


// FMMessageBoxDlg
//

class FMMessageBoxDlg : public FMDialog
{
public:
	FMMessageBoxDlg(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);

protected:
	virtual BOOL InitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnButtonClicked(UINT nID);
	DECLARE_MESSAGE_MAP()

	CString m_Text;
	CString m_Caption;
	UINT m_Type;

	CRect m_RectText;
	CPoint m_IconPos;
	INT m_IconSize;
	HICON m_hIcon;

private:
	void SetButton(UINT nResID, HINSTANCE hInstance, UINT nCommand, UINT& cButtons);
};
