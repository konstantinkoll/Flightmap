
// CColorPicker.h: Schnittstelle der Klasse CColorPicker
//

#pragma once
#include "FMDynArray.h"


// CHueWheel
//

#define HUEWHEEL_UPDATE_HUE     1

struct NM_HUEDATA
{
	NMHDR hdr;
	DOUBLE Hue;
};

struct HueWheelBitmaps
{
	INT Size;
	COLORREF WindowColor;
	HBITMAP hBitmapWheel;
	HBITMAP hBitmapTop;
};

class CHueWheel : public CWnd
{
public:
	CHueWheel();

	BOOL Create(CWnd* pParentWnd, const CRect& rect, UINT nID);
	void SetColor(COLORREF clr);
	void SetHue(DOUBLE Hue);
	COLORREF GetColor() const;
	DOUBLE GetHue() const;

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	HueWheelBitmaps* GetBitmaps(BOOL Themed);
	static void DrawEtchedText(CDC& dc, CRect rect, LPCWSTR lpStr, BOOL Themed);
	BOOL PointInRing(const CPoint& point) const;
	void UpdateCursor();

	DOUBLE m_Hue;
	INT m_Size;
	static FMDynArray<HueWheelBitmaps, 2, 2> m_Bitmaps;

private:
	void CreateBitmaps(HueWheelBitmaps* pBitmaps);

	REAL m_OuterRadius;
	REAL m_OuterRadiusSq;
	REAL m_OuterRadiusSqDraw;
	REAL m_InnerRadius;
	REAL m_InnerRadiusSq;
	REAL m_InnerRadiusSqDraw;
	BOOL m_Grabbed;

	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;
};

inline COLORREF CHueWheel::GetColor() const
{
	return CDrawingManager::HLStoRGB_ONE(m_Hue/360.0, 0.5, 1.0);
}

inline DOUBLE CHueWheel::GetHue() const
{
	return m_Hue;
}


// CGradationPyramid
//

#define GRADATIONPYRAMID_DBLCLK     1

struct GradationPyramidBitmaps
{
	INT Height;
	INT Width;
	COLORREF WindowColor;
	HBITMAP hBitmapTop;
};

class CGradationPyramid : public CWnd
{
public:
	CGradationPyramid();

	BOOL Create(CWnd* pParentWnd, const CRect& rect, UINT nID);
	void SetHue(DOUBLE Hue);
	void SetColor(INT Row, INT Column);
	void SetColor(COLORREF clr);
	COLORREF GetColor() const;

protected:
	GradationPyramidBitmaps* GetBitmaps(BOOL Themed);
	static UINT ColumnsPerRow(UINT Row);
	COLORREF GetColor(UINT Row, UINT Column) const;
	void GetCoordinates(PointF* pPoints, UINT Row, UINT Column, REAL Widen=0.0f) const;
	void UpdateCursor();
	BOOL PointInPyramid(CPoint point) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	DOUBLE m_Hue;
	INT m_Height;
	INT m_Width;
	REAL m_BaseWidth;
	REAL m_RowHeight;
	REAL m_HalfWidth;
	REAL m_Slope;
	CPoint m_FocusItem;
	CPoint m_HotItem;
	BOOL m_Hover;
	static FMDynArray<GradationPyramidBitmaps, 2, 2> m_Bitmaps;

private:
	void CreateBitmaps(GradationPyramidBitmaps* pBitmaps);
	void ItemAtPosition(CPoint point, INT& Row, INT& Column) const;

	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;
};

inline COLORREF CGradationPyramid::GetColor() const
{
	return GetColor(m_FocusItem.y, m_FocusItem.x);
}

inline UINT CGradationPyramid::ColumnsPerRow(UINT Row)
{
	return 2*Row+1;
}


// CColorPicker
//

class CColorPicker : public CWnd
{
public:
	CColorPicker();

	virtual void PreSubclassWindow();

	COLORREF GetColor() const;
	void SetColor(COLORREF clr);
	static CString FormatColor(COLORREF clr);

protected:
	virtual void Init();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	afx_msg void OnUpdateHue(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CHueWheel m_wndHueWheel;
	CGradationPyramid m_wndGradationPyramid;
};

inline COLORREF CColorPicker::GetColor() const
{
	return m_wndGradationPyramid.GetColor();
}

inline CString CColorPicker::FormatColor(COLORREF clr)
{
	CString tmpStr;
	tmpStr.Format(_T("#%06X"), _byteswap_ulong(clr) >> 8);

	return tmpStr;
}
