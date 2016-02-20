
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "CKitchen.h"
#include "FMCommDlg.h"


// Item Data

struct GlobeParameters
{
	GLfloat Latitude;
	GLfloat Longitude;
	INT Zoom;
};

struct GlobeItemData
{
	RECT Rect;
	GLfloat World[3];
	INT ScreenPoint[2];
	GLfloat Alpha;
	FMAirport* pAirport;
	CHAR NameString[130];
	CHAR CoordString[32];
	WCHAR CountStringLarge[64];
	WCHAR CountStringSmall[8];
};


// CGlobeView
//

class CGlobeView : public CFrontstageWnd
{
public:
	CGlobeView();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetFlights(CKitchen* pKitchen, BOOL DeleteKitchen=TRUE);
	void UpdateViewOptions(BOOL Force=FALSE);

protected:
	INT ItemAtPosition(CPoint point) const;
	void InvalidateItem(INT Index);
	void SelectItem(INT Index, BOOL Select);
	void CalcAndDrawSpots(const GLfloat ModelView[4][4], const GLfloat Projection[4][4]);
	void CalcAndDrawLabel(BOOL Themed);
	void DrawLabel(GlobeItemData* pData, CHAR* Caption, CHAR* Subcaption, CHAR* Coordinates, WCHAR* Description, BOOL Focused, BOOL Hot, BOOL Themed);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnJumpToLocation();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnSaveAs();
	afx_msg void OnGoogleEarth();
	afx_msg void OnLiquidFolders();
	afx_msg void OnSettings();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	FMDynArray<GlobeItemData, 64, 64> m_Airports;
	FMDynArray<FlightSegments*, 128, 128> m_Routes;
	UINT m_MinRouteCount;
	UINT m_MaxRouteCount;

	INT m_FocusItem;
	INT m_HotItem;
	BOOL m_IsSelected;
	BOOL m_Hover;

	GlobeParameters m_GlobeTarget;
	GlobeParameters m_GlobeCurrent;
	GLRenderContext m_RenderContext;

	GLuint m_nHaloModel;
	GLuint m_nGlobeModel;
	GLuint m_nRouteModel;
	GLuint m_nTextureBlueMarble;
	GLuint m_nTextureClouds;
	GLuint m_nTextureLocationIndicator;

	GLFont m_Fonts[2];

	GLcolor m_AttrColor;
	GLcolor m_BottomColorHot;
	GLcolor m_BottomColorSelected;
	GLcolor m_CaptionColor;
	GLcolor m_SelectedColor;
	GLcolor m_TextColor;
	GLcolor m_TopColorHot;
	GLcolor m_TopColorSelected;

private:
	BOOL CursorOnGlobe(const CPoint& point) const;
	void UpdateCursor();
	void RenderScene(BOOL Themed);

	CString m_DisplayName;
	static CString m_strFlightCountSingular;
	static CString m_FlightCount_Plural;

	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;

	GLfloat m_GlobeRadius;
	UINT m_AnimCounter;
	GLfloat m_AnimStartLatitude;
	GLfloat m_AnimStartLongitude;
	UINT m_MoveCounter;
	GLfloat m_LastMove;
	GLfloat m_Momentum;

	CPoint m_GrabPoint;
	BOOL m_Grabbed;
};
