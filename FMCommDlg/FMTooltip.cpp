
// FMTooltip.cpp: Implementierung der Klasse FMTooltip
//

#include "stdafx.h"
#include "FMCommDlg.h"


void AppendAttribute(CString& dst, UINT ResID, CString Value)
{
	if (!Value.IsEmpty())
	{
		CString Name;
		ENSURE(Name.LoadString(ResID));

		dst.Append(Name);
		dst.Append(_T(": "));
		dst.Append(Value);
		dst.Append(_T("\n"));
	}
}

void AppendAttribute(CString& dst, UINT ResID, CHAR* Value)
{
	CString tmpStr(Value);
	AppendAttribute(dst, ResID, tmpStr);
}


// FMTooltip
//

FMTooltip::FMTooltip()
	: CWnd()
{
	m_Icon = NULL;
	m_Bitmap = NULL;
}

BOOL FMTooltip::Create(CWnd* pWndParent)
{
	UINT nClassStyle = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
	if (bDropShadow)
		nClassStyle |= CS_DROPSHADOW;

	CString className = AfxRegisterWndClass(nClassStyle, LoadCursor(NULL, IDC_ARROW));
	return CWnd::CreateEx(0, className, _T(""), WS_POPUP, 0, 0, 0, 0, pWndParent->GetSafeHwnd(), NULL);
}

BOOL FMTooltip::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message>=WM_MOUSEFIRST) && (pMsg->message<=WM_MOUSELAST))
	{
		if (pMsg->message!=WM_MOUSEMOVE)
			Hide();

		CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		MapWindowPoints(GetParent(), &pt, 1);
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);

		GetParent()->SendMessage(pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void FMTooltip::Track(CPoint point, HICON hIcon, HBITMAP hBitmap, CSize Size, const CString& strCaption, CString strText, BOOL DrawBorder)
{
	if (!GetSafeHwnd())
		return;

	if ((m_strText==strText) && (m_strCaption==strCaption))
		return;

	if (IsWindowVisible())
		Hide();
	if (m_Icon)
		DestroyIcon(m_Icon);

	m_Icon = hIcon;
	m_Bitmap = hBitmap;
	m_Size = Size;
	m_strCaption = strCaption;
	m_strText = strText;
	m_DrawBorder = DrawBorder;

	// Size
	CSize sz(0, 0);
	CClientDC dc(this);

	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		CSize szText = dc.GetTextExtent(strCaption);
		sz.cx = max(sz.cx, szText.cx);
		sz.cy += szText.cy;
		dc.SelectObject(pOldFont);

		if (!strText.IsEmpty())
			sz.cy += AFX_TEXT_MARGIN;
	}

	if (!strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		m_TextHeight = 0;

		while (!strText.IsEmpty())
		{
			CString Line;
			INT pos = strText.Find('\n');
			if (pos==-1)
			{
				Line = strText;
				strText.Empty();
			}
			else
			{
				Line = strText.Left(pos);
				strText.Delete(0, pos+1);
			}

			CSize szText = dc.GetTextExtent(Line);
			sz.cx = max(sz.cx, szText.cx);
			sz.cy += szText.cy;

			m_TextHeight = max(m_TextHeight, szText.cy);
		}

		dc.SelectObject(pOldFont);
	}

	if (hIcon || hBitmap)
	{
		sz.cx += Size.cx+2*AFX_TEXT_MARGIN;
		sz.cy = max(sz.cy, Size.cy);
	}

	sz.cx += 2*(AFX_TEXT_MARGIN+3);
	sz.cy += 2*(AFX_TEXT_MARGIN+2);
	if (sz.cx>m_TextHeight*25)
		sz.cx = m_TextHeight*25;

	// Position
	CRect rect;
	rect.top = point.y+18;
	rect.bottom = rect.top+sz.cy;

	if (GetParent()->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rect.left = point.x-sz.cx;
		rect.right = point.x;
	}
	else
	{
		rect.left = point.x;
		rect.right = point.x+sz.cx;
	}

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.Width()>rectScreen.Width())
	{
		rect.left = rectScreen.left;
		rect.right = rectScreen.right;
	}
	else 
		if (rect.right>rectScreen.right)
		{
			rect.right = rectScreen.right;
			rect.left = rect.right-sz.cx;
		}
		else
			if (rect.left<rectScreen.left)
			{
				rect.left = rectScreen.left;
				rect.right = rect.left+sz.cx;
			}

	if (rect.Height()>rectScreen.Height())
	{
		rect.top = rectScreen.top;
		rect.bottom = rectScreen.bottom;
	}
	else
		if (rect.bottom>rectScreen.bottom)
		{
			rect.bottom = point.y-1;
			rect.top = rect.bottom-sz.cy;
		}
		else
			if (rect.top<rectScreen.top)
			{
				rect.top = rectScreen.top;
				rect.bottom = rect.top+sz.cy;
			}

	CRgn rgn;
	m_Themed = IsCtrlThemed();
	if (m_Themed)
	{
		rgn.CreateRoundRectRgn(0, 0, sz.cx+1, sz.cy+1, 4, 4);
	}
	else
	{
		rgn.CreateRectRgn(0, 0, sz.cx, sz.cy);
	}
	SetWindowRgn(rgn, FALSE);

	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	ShowWindow(SW_SHOWNOACTIVATE);

	Invalidate();
	UpdateWindow();
}

void FMTooltip::Track(CPoint point, FMAirport* pAirport, CString strText)
{
	CString Caption(pAirport->Code);
	CString Text(_T(""));
	CString tmpStr;

	AppendAttribute(Text, IDS_AIRPORT_NAME, pAirport->Name);
	AppendAttribute(Text, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
	FMGeoCoordinatesToString(pAirport->Location, tmpStr);
	AppendAttribute(Text, IDS_AIRPORT_LOCATION, tmpStr);

	if (strText.IsEmpty())
		Text.Append(strText);

	Track(point, NULL, NULL, 128, Caption, Text, TRUE);
}

void FMTooltip::Track(CPoint point, CHAR* Code, CString strText)
{
	FMAirport* pAirport = NULL;
	if (FMIATAGetAirportByCode(Code, &pAirport))
		Track(point, pAirport, strText);
}

void FMTooltip::Hide()
{
	if (IsWindow(m_hWnd))
		ShowWindow(SW_HIDE);
}

void FMTooltip::Deactivate()
{
	m_strCaption.Empty();
	m_strText.Empty();
	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	Hide();
}


BEGIN_MESSAGE_MAP(FMTooltip, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void FMTooltip::OnDestroy()
{
	if (m_Icon)
		DestroyIcon(m_Icon);
	if (m_Bitmap)
		DeleteObject(m_Bitmap);

	CWnd::OnDestroy();
}

BOOL FMTooltip::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void FMTooltip::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(1, 1);

	// Background
	if (m_Themed)
	{
		Graphics g(dc);
		LinearGradientBrush brush(Point(0, rect.top), Point(0, rect.bottom+1), Color(0xFF, 0xFF, 0xFF), Color(0xE4, 0xE5, 0xF0));
		g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_INFOBK));
	}

	// Border
	COLORREF clrLine = m_Themed ? 0x767676 : GetSysColor(COLOR_INFOTEXT);
	COLORREF clrText = m_Themed ? 0x4C4C4C : GetSysColor(COLOR_INFOTEXT);

	CPen penLine(PS_SOLID, 1, clrLine);
	CPen* pOldPen = dc.SelectObject(&penLine);
	rect.InflateRect(1, 1);

	if (m_Themed)
	{
		const INT nOffset = 2;

		dc.MoveTo(rect.left+nOffset, rect.top);
		dc.LineTo(rect.right-nOffset-1, rect.top);

		dc.LineTo(rect.right-1, rect.top+nOffset);
		dc.LineTo(rect.right-1, rect.bottom-1-nOffset);

		dc.LineTo(rect.right-nOffset-1, rect.bottom-1);
		dc.LineTo(rect.left+nOffset, rect.bottom-1);

		dc.LineTo(rect.left, rect.bottom-1-nOffset);
		dc.LineTo(rect.left, rect.top+nOffset);

		dc.LineTo(rect.left+nOffset, rect.top);
	}
	else
	{
		dc.Draw3dRect(rect, clrLine, clrLine);
	}

	dc.SelectObject(pOldPen);

	// Interior
	rect.DeflateRect(AFX_TEXT_MARGIN+3, AFX_TEXT_MARGIN+2);
	dc.SetTextColor(clrText);

	if (m_Icon)
	{
		DrawIconEx(dc, rect.left, rect.top, m_Icon, m_Size.cx, m_Size.cy, 0, NULL, DI_NORMAL);
		rect.left += m_Size.cx+2*AFX_TEXT_MARGIN;
	}

	if (!m_strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		dc.DrawText(m_strCaption, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		rect.top += dc.GetTextExtent(m_strCaption).cy+AFX_TEXT_MARGIN;
		dc.SelectObject(pOldFont);
	}

	if (!m_strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		CString strText = m_strText;

		while (!strText.IsEmpty())
		{
			CString Line;
			INT pos = strText.Find('\n');
			if (pos==-1)
			{
				Line = strText;
				strText.Empty();
			}
			else
			{
				Line = strText.Left(pos);
				strText.Delete(0, pos+1);
			}

			dc.DrawText(Line, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
			rect.top += m_TextHeight;
		}

		dc.SelectObject(pOldFont);
	}
}
