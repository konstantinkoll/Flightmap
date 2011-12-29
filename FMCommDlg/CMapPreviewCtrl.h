
// CMapPreviewCtrl.h: Schnittstelle der Klasse CMapPreviewCtrl
//

#pragma once
#include "IATA.h"
#include "CGdiPlusBitmap.h"


// CMapPreviewCtrl
//

class CMapPreviewCtrl : public CWnd
{
public:
	CMapPreviewCtrl();
	~CMapPreviewCtrl();

	void Update(FMAirport* pAirport);

protected:
	FMAirport* p_Airport;
	FMGeoCoordinates m_Location;
	HBITMAP hMap;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
