
// CColorIndicator: Schnittstelle der Klasse CColorIndicator
//

#pragma once


// CColorIndicator
//

class CColorIndicator : public CStatic
{
public:
	CColorIndicator();

	virtual void PreSubclassWindow();

	void SetColor(COLORREF clr);

protected:
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	COLORREF m_Color;
};
