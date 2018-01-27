
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
	ItemData Hdr;
	GLfloat World[3];
	INT ScreenPoint[2];
	GLfloat Alpha;
	CHAR CoordString[32];

	LPCAIRPORT lpcAirport;
	CHAR NameString[130];
	WCHAR CountStringLarge[64];
	WCHAR CountStringSmall[8];
};


// CGlobeView
//

class CGlobeView sealed : public CFrontstageItemView
{
public:
	CGlobeView();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetFlights(CKitchen* pKitchen);
	void UpdateViewOptions(BOOL Force=FALSE);

protected:
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void ShowTooltip(const CPoint& point);
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual void GetNothingMessage(CString& strMessage, COLORREF& clrMessage, BOOL Themed) const;
	virtual BOOL DrawNothing() const;

	void CalcAndDrawSpots(const GLmatrix& ModelView, const GLmatrix& Projection);
	void CalcAndDrawLabel(BOOL Themed);
	void DrawLabel(GlobeItemData* pData, LPCSTR Caption, LPCSTR Subcaption, LPCSTR Coordinates, LPCWSTR Description, BOOL Selected, BOOL Hot, BOOL Themed);
	BOOL UpdateScene(BOOL Redraw=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	afx_msg void OnJumpToLocation();
	afx_msg void OnShowLocations();
	afx_msg void OnShowAirportIATA();
	afx_msg void OnShowAirportNames();
	afx_msg void OnShowCoordinates();
	afx_msg void OnShowDescriptions();
	afx_msg void OnDarkBackground();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnAutosize();
	afx_msg void OnSaveAs();
	afx_msg void OnGoogleEarth();
	afx_msg void OnLiquidFolders();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CKitchen* m_pKitchen;
	BOOL m_HasRoutes;

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
	GlobeItemData* GetGlobeItemData(INT Index) const;
	BOOL CursorOnGlobe(const CPoint& point) const;
	void UpdateCursor();
	void RenderScene();

	CString m_DisplayName;
	static CString m_strFlightCountSingular;
	static CString m_strFlightCountPlural;
	static const GLcolor m_lAmbient;
	static const GLcolor m_lDiffuse;
	static const GLcolor m_lSpecular;
	static const GLcolor m_FogColor;

	LPCWSTR lpszCursorName;
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

inline GlobeItemData* CGlobeView::GetGlobeItemData(INT Index) const
{
	return (GlobeItemData*)GetItemData(Index);
}
