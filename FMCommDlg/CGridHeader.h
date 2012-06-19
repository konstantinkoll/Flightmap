
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
	DECLARE_MESSAGE_MAP()
};
