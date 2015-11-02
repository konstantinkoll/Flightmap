
// FMResolutionDlg.h: Schnittstelle der Klasse FMResolutionDlg
//

#pragma once
#include "CExplorerList.h"


// FMResolutionDlg
//

class FMResolutionDlg : public CDialog
{
public:
	FMResolutionDlg(UINT* pWidth, UINT* pHeight, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUserDefinedRes();
	DECLARE_MESSAGE_MAP()

	CImageList m_ResolutionIcons;
	CExplorerList m_wndResolutionList;
	CMFCMaskedEdit m_wndWidth;
	CMFCMaskedEdit m_wndHeight;
	UINT* p_Width;
	UINT* p_Height;
};
