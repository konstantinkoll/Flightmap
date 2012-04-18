
// CGlobeView.h: Schnittstelle der Klasse CGlobeView
//

#pragma once
#include "CKitchen.h"
#include "Flightmap.h"
#include "GLTexture.h"
#include "GLFont.h"


// Item data

struct GlobeParameters
{
	GLfloat Latitude;
	GLfloat Longitude;
	INT Zoom;
};

struct GlobeAirport
{
	RECT Rect;
	GLfloat World[3];
	INT ScreenPoint[2];
	GLfloat Alpha;
	FMAirport* pAirport;
	CHAR NameString[130];
	CHAR CoordString[32];
	WCHAR CountString[64];
};


// CGlobeView
//

class CGlobeView : public CWnd
{
public:
	CGlobeView();
	~CGlobeView();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetFlights(CKitchen* pKitchen, BOOL DeleteKitchen=TRUE);
	void UpdateViewOptions(BOOL Force=FALSE);

protected:
	GlobeParameters m_GlobeTarget;
	GlobeParameters m_GlobeCurrent;
	BOOL m_ShowSpots;
	BOOL m_ShowAirportIATA;
	BOOL m_ShowAirportNames;
	BOOL m_ShowGPS;
	BOOL m_ShowFlightCount;
	BOOL m_ShowViewport;
	BOOL m_ShowCrosshairs;
	BOOL m_UseColors;
	BOOL m_Clamp;

	DynArray<GlobeAirport> m_Airports;
	DynArray<FlightSegments*> m_Routes;

	CClientDC* m_pDC;
	HGLRC hRC;
	INT m_FocusItem;
	INT m_HotItem;
	INT m_Width;
	INT m_Height;
	BOOL m_IsSelected;
	BOOL m_Hover;
	GLTexture* m_pTextureGlobe;
	GLTexture* m_pTextureIcons;
	GLFont m_Fonts[2];

	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT idx);
	void SelectItem(INT idx, BOOL Select);
	void PrepareModel();
	void PrepareRoutes();
	void PrepareTexture();
	void Normalize();
	void CalcAndDrawSpots(GLfloat ModelView[4][4], GLfloat Projection[4][4]);
	void CalcAndDrawLabel();
	void DrawLabel(GlobeAirport* ga, CHAR* Caption, CHAR* Subcaption, CHAR* Coordinates, WCHAR* Description, BOOL Focused);
	void DrawStatusBar(INT Height);
	void DrawScene(BOOL InternalCall=FALSE);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnSaveAs();
	afx_msg void OnJumpToLocation();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnColors();
	afx_msg void OnClamp();
	afx_msg void OnSpots();
	afx_msg void OnAirportIATA();
	afx_msg void OnAirportNames();
	afx_msg void OnGPS();
	afx_msg void OnFlightCount();
	afx_msg void OnViewport();
	afx_msg void OnCrosshairs();
	afx_msg void On3DSettings();
	afx_msg void OnOpenGoogleEarth();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	LPCTSTR lpszCursorName;
	HCURSOR hCursor;
	CPoint m_CursorPos;

	GLint m_GlobeModel;
	GLint m_GlobeRoutes;
	BOOL m_IsHQModel;
	INT m_CurrentGlobeTexture;

	GLfloat m_Scale;
	GLfloat m_Radius;
	GLfloat m_FogStart;
	GLfloat m_FogEnd;
	UINT m_AnimCounter;
	GLfloat m_AnimStartLatitude;
	GLfloat m_AnimStartLongitude;
	UINT m_MoveCounter;
	GLfloat m_LastMove;
	GLfloat m_Momentum;

	CPoint m_GrabPoint;
	BOOL m_Grabbed;
	BOOL m_LockUpdate;
	CString m_YouLookAt;
	CString m_FlightCount_Singular;
	CString m_FlightCount_Plural;
	CString m_DisplayName;
	FMTooltip m_TooltipCtrl;

	BOOL CursorOnGlobe(CPoint point);
	void UpdateCursor();
};
