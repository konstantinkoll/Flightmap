
// CGridHeader: Schnittstelle der Klasse CGridHeader
//

#pragma once
#include "CTooltipHeader.h"


// CGridHeader
//

class CGridHeader : public CTooltipHeader
{
public:
	afx_msg void OnPaint();
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
