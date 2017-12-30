
// CPictureCtrl.h: Schnittstelle der Klasse CPictureCtrl
//

#pragma once


// CPictureCtrl
//

#define PC_COLOR                  0
#define PC_PICTURE_NORMAL         1
#define PC_PICTURE_SCALETOFIT     2

class CPictureCtrl : public CFrontstageWnd
{
public:
	CPictureCtrl();

	BOOL Create(CWnd* pParentWnd, const CRect& rect, UINT nID, UINT nPictureID, UINT nTooltipID=0);
	void SetPicture(Bitmap* pPicture, const CString& Caption, const CString& Hint, BOOL ScaleToFit=FALSE);
	void SetPicture(UINT nPictureID, const CString& Caption, const CString& Hint, BOOL ScaleToFit=FALSE);
	void SetPicture(UINT nPictureID, UINT nTooltipID=0, BOOL ScaleToFit=FALSE);
	void SetColor(COLORREF clr, const CString& Caption, const CString& Hint);
	void SetColor(COLORREF clr, UINT nTooltipID=0);

protected:
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void ShowTooltip(const CPoint& point);

	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	UINT m_DisplayMode;
	COLORREF m_DisplayColor;
	Bitmap* p_Picture;
	CString m_Caption;
	CString m_Hint;
};
