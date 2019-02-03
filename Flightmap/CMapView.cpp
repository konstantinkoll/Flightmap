
// CMapView.cpp: Implementierung der Klasse CMapView
//

#include "stdafx.h"
#include "CMapFactory.h"
#include "CMapView.h"
#include "Flightmap.h"


// CMapView
//

#define BORDER     3*CARDPADDING

CMapView::CMapView()
	: CFrontstageScroller(FRONTSTAGE_CARDBACKGROUND | FRONTSTAGE_DRAWSHADOW)
{
	p_BitmapOriginal = m_pBitmapScaled = NULL;

	m_ZoomFactor = theApp.m_MapZoomFactor;
}

BOOL CMapView::Create(CWnd* pParentWnd, UINT nID)
{
	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	return CFrontstageScroller::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}


// Bitmap

void CMapView::SetBitmap(CBitmap* pBitmap, const CString& Title)
{
	p_BitmapOriginal = pBitmap;
	m_Title = Title;

	ScaleBitmap();
}

void CMapView::DeleteScaledBitmap()
{
	HideTooltip();

	if (m_pBitmapScaled!=p_BitmapOriginal)
		delete m_pBitmapScaled;

	m_pBitmapScaled = NULL;
}

void CMapView::ScaleBitmap()
{
	DeleteScaledBitmap();

	if (p_BitmapOriginal)
	{
		const CSize Size = p_BitmapOriginal->GetBitmapDimension();

		const DOUBLE ZoomFactor = GetZoomFactor();
		const INT Width = (INT)((DOUBLE)Size.cx*ZoomFactor);
		const INT Height = (INT)((DOUBLE)Size.cy*ZoomFactor);

		if (ZoomFactor<1.0)
		{
			// Create a pre-scaled copy of the map
			m_pBitmapScaled = CreateTruecolorBitmapObject(Width, Height);

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			CBitmap* pOldBitmap = dc.SelectObject(m_pBitmapScaled);

			Graphics g(dc);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			Bitmap Map(*p_BitmapOriginal, NULL);
			g.DrawImage(&Map, Rect(-1, -1, Width+1, Height+1), 0, 0, Size.cx, Size.cy, UnitPixel);

			dc.SelectObject(pOldBitmap);
		}
		else
		{
			// Reference original map
			m_pBitmapScaled = p_BitmapOriginal;
		}

		// Add border pixels
		m_ScrollWidth = Width+2*BORDER;
		m_ScrollHeight = Height+2*BORDER;
	}
	else
	{
		m_ScrollWidth = m_ScrollHeight = 0;
	}

	AdjustLayout();
}

void CMapView::GetCardRect(const CRect& rectClient, CRect& rectCard) const
{
	const INT PosX = m_ScrollWidth<rectClient.Width() ? (rectClient.Width()-m_ScrollWidth)/2 : -m_HScrollPos;
	const INT PosY = m_ScrollHeight<rectClient.Height() ? (rectClient.Height()-m_ScrollHeight)/2 : -m_VScrollPos;

	rectCard.left = PosX+(BORDER-CARDPADDING);
	rectCard.right = PosX+m_ScrollWidth-(BORDER-CARDPADDING);
	rectCard.top = PosY+(BORDER-CARDPADDING);
	rectCard.bottom = PosY+m_ScrollHeight-(BORDER-CARDPADDING);
}


// Menus

BOOL CMapView::GetContextMenu(CMenu& Menu, INT /*Index*/)
{
	Menu.LoadMenu(IDM_MAPWND);

	return FALSE;
}


// Item handling

INT CMapView::ItemAtPosition(CPoint point) const
{
	CRect rectCard;
	GetCardRect(rectCard);

	return rectCard.PtInRect(point) ? 0 : -1;
}

void CMapView::ShowTooltip(const CPoint& point)
{
	if (p_BitmapOriginal)
	{
		NM_TOOLTIPDATA tag;
		ZeroMemory(&tag, sizeof(tag));

		tag.hdr.code = REQUEST_TOOLTIP_DATA;
		tag.hdr.hwndFrom = m_hWnd;
		tag.hdr.idFrom = GetDlgCtrlID();

		const CSize SizeOriginal = p_BitmapOriginal->GetBitmapDimension();
		CString tmpStr;

		if (m_pBitmapScaled && (m_pBitmapScaled!=p_BitmapOriginal))
		{
			const CSize SizeScaled = m_pBitmapScaled->GetBitmapDimension();

			tmpStr.Format(IDS_MAPDIMENSION_SCALED, SizeOriginal.cx, SizeOriginal.cy, SizeScaled.cx, SizeScaled.cy);
		}
		else
		{
			tmpStr.Format(IDS_MAPDIMENSION_ORIGINAL, SizeOriginal.cx, SizeOriginal.cy);
		}

		theApp.ShowTooltip(this, point, m_Title, tmpStr);
	}
}


// Drawing

BOOL CMapView::DrawNothing() const
{
	return !m_pBitmapScaled;
}

void CMapView::DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& /*rectUpdate*/, BOOL Themed)
{
	CRect rectCard;
	GetCardRect(rect, rectCard);

	DrawCardForeground(dc, g, rectCard, Themed);

	// Map picture (either pre-scaled or streched)
	const CSize Size = m_pBitmapScaled->GetBitmapDimension();

	CRect rectMap(rectCard);
	rectMap.DeflateRect(CARDPADDING, CARDPADDING);

	CDC dcMap;
	dcMap.CreateCompatibleDC(&dc);

	CBitmap* pOldBitmap = dcMap.SelectObject(m_pBitmapScaled);

	if ((Size.cx<rectMap.Width()) || (Size.cy<rectMap.Height()))
	{
		dc.StretchBlt(rectMap.left, rectMap.top, rectMap.Width(), rectMap.Height(), &dcMap, 0, 0, Size.cx, Size.cy, SRCCOPY);
	}
	else
	{
		dc.BitBlt(rectMap.left, rectMap.top, rectMap.Width(), rectMap.Height(), &dcMap, 0, 0, SRCCOPY);
	}

	dcMap.SelectObject(pOldBitmap);
}


BEGIN_MESSAGE_MAP(CMapView, CFrontstageScroller)
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()

	ON_COMMAND(IDM_MAPWND_ZOOMIN, OnZoomIn)
	ON_COMMAND(IDM_MAPWND_ZOOMOUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPWND_ZOOMIN, IDM_MAPWND_ZOOMOUT, OnUpdateCommands)
END_MESSAGE_MAP()

void CMapView::OnDestroy()
{
	DeleteScaledBitmap();

	CFrontstageScroller::OnDestroy();
}

BOOL CMapView::OnMouseWheel(UINT /*nFlags*/, SHORT zDelta, CPoint /*pt*/)
{
	if (zDelta<0)
	{
		OnZoomOut();
	}
	else
	{
		OnZoomIn();
	}

	return TRUE;
}

void CMapView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch (nChar)
	{
	case VK_LEFT:
		OnHScroll(SB_LINEUP, 0, NULL);
		break;

	case VK_RIGHT:
		OnHScroll(SB_LINEDOWN, 0, NULL);
		break;

	case VK_UP:
		OnVScroll(SB_LINEUP, 0, NULL);
		break;

	case VK_DOWN:
		OnVScroll(SB_LINEDOWN, 0, NULL);
		break;

	case VK_HOME:
		if (GetKeyState(VK_CONTROL)<0)
			OnVScroll(SB_TOP, 0, NULL);

		OnHScroll(SB_TOP, 0, NULL);

		break;

	case VK_END:
		if (GetKeyState(VK_CONTROL)<0)
			OnVScroll(SB_BOTTOM, 0, NULL);

		OnHScroll(SB_BOTTOM, 0, NULL);

		break;

	case VK_ADD:
	case VK_OEM_PLUS:
	case VK_PRIOR:
		OnZoomIn();
		break;

	case VK_SUBTRACT:
	case VK_OEM_MINUS:
	case VK_NEXT:
		OnZoomOut();
	}
}


void CMapView::OnZoomIn()
{
	if (m_ZoomFactor<ZOOMFACTORS-1)
	{
		theApp.m_MapZoomFactor = ++m_ZoomFactor;

		ScaleBitmap();
	}
}

void CMapView::OnZoomOut()
{
	if (m_ZoomFactor>0)
	{
		theApp.m_MapZoomFactor = --m_ZoomFactor;

		ScaleBitmap();
	}
}

void CMapView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (p_BitmapOriginal!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_MAPWND_ZOOMIN:
		bEnable &= (m_ZoomFactor<ZOOMFACTORS);
		break;

	case IDM_MAPWND_ZOOMOUT:
		bEnable &= (m_ZoomFactor>0);
		break;
	}

	pCmdUI->Enable(bEnable);
}
