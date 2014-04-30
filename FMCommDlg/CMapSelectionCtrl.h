
// CMapSelectionCtrl.h: Schnittstelle der Klasse CMapSelectionCtrl
//

#pragma once
#include "IATA.h"


// CMapSelectionCtrl
//

#define MAP_UPDATE_LOCATION     1

struct tagGPSDATA
{
	NMHDR hdr;
	FMGeoCoordinates* pCoord;
};

class CMapSelectionCtrl : public CWnd
{
public:
	CMapSelectionCtrl();

	void OnBlink();
	void SetGeoCoordinates(const FMGeoCoordinates Coord);

protected:
	FMGeoCoordinates m_Coord;

	void UpdateLocation(CPoint point);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	BOOL m_Blink;
	UINT m_RemainVisible;

	void SendUpdateMsg();
};
