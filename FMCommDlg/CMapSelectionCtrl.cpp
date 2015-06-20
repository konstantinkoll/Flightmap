
// CMapSelectionCtrl.cpp: Implementierung der Klasse CMapSelectionCtrl
//

#include "stdafx.h"
#include "FMCommDlg.h"


// CMapSelectionCtrl
//

CMapSelectionCtrl::CMapSelectionCtrl()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = FMGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CMapSelectionCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CMapSelectionCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_Coord.Latitude = m_Coord.Longitude = 0;
	m_BackBufferL = m_BackBufferH = 0;
	m_Blink = TRUE;
	m_RemainVisible = 0;
}

void CMapSelectionCtrl::OnBlink()
{
	if (m_RemainVisible)
	{
		m_RemainVisible--;
	}
	else
	{
		m_Blink = !m_Blink;
		Invalidate();
	}
}

void CMapSelectionCtrl::SetGeoCoordinates(const FMGeoCoordinates Coord)
{
	m_Coord = Coord;
	SendUpdateMsg();
}

void CMapSelectionCtrl::UpdateLocation(CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	DOUBLE Latitude = (((point.y-1)*180.0)/rect.Height())-90.0;
	if (Latitude<-90.0)
		Latitude = -90.0;
	if (Latitude>90.0)
		Latitude = 90.0;
	DOUBLE Longitude = (((point.x-1)*360.0)/rect.Width())-180.0;
	if (Longitude<-180.0)
		Longitude = -180.0;
	if (Longitude>180.0)
		Longitude = 180.0;

	m_Coord.Latitude = Latitude;
	m_Coord.Longitude = Longitude;
	SendUpdateMsg();
}

void CMapSelectionCtrl::SendUpdateMsg()
{
	m_Blink = TRUE;
	m_RemainVisible = 1;
	Invalidate();

	tagGPSDATA tag;
	tag.hdr.code = MAP_UPDATE_LOCATION;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.pCoord = &m_Coord;

	GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
}


BEGIN_MESSAGE_MAP(CMapSelectionCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CMapSelectionCtrl::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap* pOldBitmap;
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&m_BackBuffer);

		CGdiPlusBitmap* pMap = FMGetApp()->GetCachedResourceImage(IDB_EARTHMAP, _T("JPG"));
		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.DrawImage(pMap->m_pBitmap, 0, 0, rect.Width(), rect.Height());
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void CMapSelectionCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

void CMapSelectionCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	CBrush brush(&m_BackBuffer);
	dc.FillRect(rect, &brush);

	if (m_Blink)
	{
		INT cx = (INT)((m_Coord.Longitude+180)*rect.Width()/360)+1;
		INT cy = (INT)((m_Coord.Latitude+90)*rect.Height()/180)+1;

		CGdiPlusBitmap* pIndicator = FMGetApp()->GetCachedResourceImage(IDB_LOCATIONINDICATOR_8, _T("PNG"));
		INT h = pIndicator->m_pBitmap->GetHeight();
		INT l = pIndicator->m_pBitmap->GetWidth();

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.DrawImage(pIndicator->m_pBitmap, cx-l/2, cy-h/2);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CMapSelectionCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	UpdateLocation(point);
}

void CMapSelectionCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags & MK_LBUTTON)
		UpdateLocation(point);
}
