
// CMapPreviewCtrl.h: Schnittstelle der Klasse CMapPreviewCtrl
//

#pragma once
#include "IATA.h"


// CMapPreviewCtrl
//

class CMapPreviewCtrl : public CWnd
{
public:
	CMapPreviewCtrl();
	~CMapPreviewCtrl();

	void Update(FMAirport* pAirport);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	FMAirport* p_Airport;
	FMGeoCoordinates m_Location;
	HBITMAP hMap;
};
