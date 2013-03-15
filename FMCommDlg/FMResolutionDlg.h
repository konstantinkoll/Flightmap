
// FMResolutionDlg.h: Schnittstelle der Klasse FMResolutionDlg
//

#pragma once
#include "CExplorerList.h"
#include "CImageListTransparent.h"


// FMResolutionDlg
//

class FMResolutionDlg : public CDialog
{
public:
	FMResolutionDlg(UINT* pWidth, UINT* pHeight, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CExplorerList m_wndResolutionList;
	CMFCMaskedEdit m_wndWidth;
	CMFCMaskedEdit m_wndHeight;
	CImageListTransparent m_Icons;
	UINT* p_Width;
	UINT* p_Height;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUserDefinedRes();
	DECLARE_MESSAGE_MAP()

private:
	HICON hIconL;
	HICON hIconS;
};
