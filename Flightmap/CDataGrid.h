
// CDataGrid.h: Schnittstelle der Klasse CDataGrid
//


#pragma once
#include "FMCommDlg.h"


// CDataGrid
//

class CDataGrid : public CWnd
{
public:
	CDataGrid();

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
