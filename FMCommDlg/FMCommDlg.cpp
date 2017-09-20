
// FMCommDlg.cpp : Definiert die exportierten Funktionen f�r die DLL-Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <cmath>
#include <sstream>
#include <winhttp.h>

#pragma warning(push, 3)
#pragma warning(disable: 4702)
#pragma warning(disable: 4706)
#include "integer.h"
#include "files.h"
#include "osrng.h"
#include "pssr.h"
#include "rsa.h"
#include "filters.h"
#include "cryptlib.h"
#include "sha.h"
#include "base64.h"
#pragma warning(pop)

using namespace CryptoPP;


void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright)
{
	Version.Empty();

	if (Copyright)
		Copyright->Empty();

	CString modFilename;
	if (GetModuleFileName(hModule, modFilename.GetBuffer(MAX_PATH), MAX_PATH)>0)
	{
		modFilename.ReleaseBuffer(MAX_PATH);
		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(modFilename, &dwHandle);
		if (dwSize>0)
		{
			LPBYTE lpInfo = new BYTE[dwSize];
			ZeroMemory(lpInfo, dwSize);

			if (GetFileVersionInfo(modFilename, 0, dwSize, lpInfo))
			{
				UINT valLen = 0;
				LPVOID valPtr = NULL;
				LPCWSTR valData = NULL;

				if (VerQueryValue(lpInfo, _T("\\"), &valPtr, &valLen))
				{
					VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
					Version.Format(_T("%u.%u.%u"), 
						(UINT)((pFinfo->dwProductVersionMS >> 16) & 0xFF),
						(UINT)((pFinfo->dwProductVersionMS) & 0xFF),
						(UINT)((pFinfo->dwProductVersionLS >> 16) & 0xFF));
				}

				if (Copyright)
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("� liquidFOLDERS");
			}

			delete[] lpInfo;
		}
	}
}


// Draw

BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

void CreateRoundRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	INT d = Radius*2+1;
	INT r = lpRect->right-d-1;
	INT b = lpRect->bottom-d-1;

	Path.Reset();
	Path.AddArc(lpRect->left, lpRect->top, d, d, 180, 90);
	Path.AddArc(r, lpRect->top, d, d, 270, 90);
	Path.AddArc(r-1, b-1, d+1, d+1, 0, 90);
	Path.AddArc(lpRect->left, b-1, d+1, d+1, 90, 90);
	Path.CloseFigure();
}

void CreateRoundTop(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	INT d = Radius*2+1;
	INT r = lpRect->right-d-1;

	Path.Reset();
	Path.AddArc(lpRect->left, lpRect->top, d, d, 180, 90);
	Path.AddArc(r, lpRect->top, d, d, 270, 90);
}

void CreateReflectionRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	Path.Reset();

	INT d = Radius*2+1;
	INT h = lpRect->bottom-lpRect->top-1;
	INT w = min((INT)(h*1.681), lpRect->right-lpRect->left-1);

	Path.AddArc(lpRect->left, lpRect->top, d, d, 180, 90);
	Path.AddArc(lpRect->left, lpRect->top, 2*w, 2*h, 270, -90);
	Path.CloseFigure();
}

BOOL IsCtrlThemed()
{
	return FMGetApp()->m_ThemeLibLoaded ? FMGetApp()->zIsAppThemed() : FALSE;
}

HBITMAP CreateTransparentBitmap(LONG Width, LONG Height)
{
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 32;
	DIB.bmiHeader.biCompression = BI_RGB;

	return CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
}

HBITMAP CreateTruecolorBitmap(LONG Width, LONG Height)
{
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 24;
	DIB.bmiHeader.biCompression = BI_RGB;

	return CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
}

CBitmap* CreateTruecolorBitmapObject(LONG Width, LONG Height)
{
	CBitmap* pBitmap = new CBitmap();
	pBitmap->Attach(CreateTruecolorBitmap(Width, Height));
	pBitmap->SetBitmapDimension(Width, Height);

	return pBitmap;
}

void DrawLocationIndicator(Graphics& g, INT x, INT y, INT Size)
{
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	Gdiplus::REAL Radius = (Gdiplus::REAL)Size/8.0f;
	Rect rect(x+(INT)(0.5f*Radius), y+(INT)(0.5f*Radius), Size-(INT)Radius-1, Size-(INT)Radius-1);

	SolidBrush brush(Color(0xFFFF0000));
	g.FillEllipse(&brush, rect);

	Pen pen(Color(0xFFFFFFFF), Radius);
	g.DrawEllipse(&pen, rect);
}

void DrawControlBorder(CWnd* pWnd)
{
	CRect rect;
	pWnd->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(pWnd);

	if (FMGetApp()->m_ThemeLibLoaded)
		if (FMGetApp()->zIsAppThemed())
		{
			HTHEME hTheme = FMGetApp()->zOpenThemeData(pWnd->GetSafeHwnd(), VSCLASS_LISTBOX);
			if (hTheme)
			{
				CRect rectClient(rect);
				rectClient.DeflateRect(2, 2);
				dc.ExcludeClipRect(rectClient);

				FMGetApp()->zDrawThemeBackground(hTheme, dc, LBCP_BORDER_NOSCROLL, pWnd->IsWindowEnabled() ? GetFocus()==pWnd->GetSafeHwnd() ? LBPSN_FOCUSED : LBPSN_NORMAL : LBPSN_DISABLED, rect, rect);
				FMGetApp()->zCloseThemeData(hTheme);

				return;
			}
		}

	dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));

	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, 0x000000, GetSysColor(COLOR_3DFACE));
}

void DrawCategory(CDC& dc, CRect rect, LPCWSTR Caption, LPCWSTR Hint, BOOL Themed)
{
	ASSERT(Caption);

	rect.DeflateRect(0, FMCATEGORYPADDING);

	CFont* pOldFont = dc.SelectObject(&FMGetApp()->m_LargeFont);
	dc.SetTextColor(Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(Caption, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

	CRect rectLine(rect);
	dc.DrawText(Caption, rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX);
	rectLine.right += 2*FMCATEGORYPADDING;

	if (rectLine.right<=rect.right)
		if (Themed)
		{
			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);
	
			LinearGradientBrush brush(Point(rectLine.right, rectLine.top), Point(rect.right, rectLine.top), Color(0xFFE2E2E2), Color(0x00E2E2E2));
			g.FillRectangle(&brush, rectLine.right, rectLine.top+(rectLine.Height()+1)/2, rect.right-rectLine.right, 1);
		}
		else
		{
			CPen pen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

			CPen* pOldPen = dc.SelectObject(&pen);
			dc.MoveTo(rectLine.right, rect.top+(rectLine.Height()+1)/2);
			dc.LineTo(rect.right, rect.top+(rectLine.Height()+1)/2);
			dc.SelectObject(pOldPen);
		}

	if (Hint)
		if (Hint[0]!=L'\0')
		{
			dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW));

			rect.top += rectLine.Height();

			dc.SelectObject(&FMGetApp()->m_DefaultFont);
			dc.DrawText(Hint, rect, DT_LEFT | DT_TOP | DT_NOPREFIX | (rect.Height()>=2*FMGetApp()->m_DefaultFont.GetFontHeight() ? DT_WORDBREAK : DT_END_ELLIPSIS));
		}

	dc.SelectObject(pOldFont);
}

void DrawReflection(Graphics& g, LPCRECT lpRect)
{
	GraphicsPath pathReflection;
	CreateReflectionRectangle(lpRect, 2, pathReflection);

	LinearGradientBrush brush(Point(lpRect->left, lpRect->top), Point(lpRect->left+min((INT)((lpRect->bottom-lpRect->top)*1.681), lpRect->right-lpRect->left), lpRect->bottom), Color(0x28FFFFFF), Color(0x10FFFFFF));
	g.FillPath(&brush, &pathReflection);
}

void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect)
{
	if (Themed)
	{
		if (Hover || Focused || Selected)
		{
			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			GraphicsPath pathOuter;
			CreateRoundRectangle(rectItem, 3, pathOuter);

			CRect rect(rectItem);
			rect.DeflateRect(1, 1);

			GraphicsPath pathInner;
			CreateRoundRectangle(rect, 2, pathInner);

			if (Selected)
			{
				LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0xFF20A0FF), Color(0xFF1080E0));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
			}
			else
				if (Hover)
				{
					LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0xFFF9FCFF), Color(0xFFE0EBFA));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
				}

			g.SetPixelOffsetMode(PixelOffsetModeNone);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			if ((ShowFocusRect && WinFocused) || Hover || Selected)
				if ((Focused && ShowFocusRect && WinFocused) || Selected)
				{
					Pen pen1(Color(0xFF1080E0));
					g.DrawPath(&pen1, &pathOuter);
				}
				else
				{
					Pen pen1(Color(0xFF8AC0F0));
					g.DrawPath(&pen1, &pathOuter);
				}

			if (Hover || Selected)
			{
				Pen pen2(Color(((Hover && !Selected) ? 0x60000000 : 0x48000000) | 0xFFFFFF));
				g.DrawPath(&pen2, &pathInner);
			}
		}

		dc.SetTextColor(Selected ? 0xFFFFFF : TextColor!=(COLORREF)-1 ? TextColor : 0x000000);
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		}

		if (WinFocused && Focused)
			dc.DrawFocusRect(rectItem);

		if (TextColor!=(COLORREF)-1 && !Selected)
			dc.SetTextColor(TextColor);
	}
}

void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL /*WinFocused*/, BOOL Hover, BOOL /*Focused*/, BOOL Selected)
{
	if (Themed && (Hover || Selected))
	{
		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		CRect rect(rectItem);
		rect.DeflateRect(1, 1);

		DrawReflection(g, rect);
	}
}

void DrawSubitemBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal)
{
	if (Hover || Selected)
		if (Themed)
		{
			COLORREF clr = Hover ? 0xB17F3C : 0x8B622C;

			if (ClipHorizontal)
			{
				dc.FillSolidRect(rect.left, rect.top, 1, rect.Height(), clr);
				dc.FillSolidRect(rect.right-1, rect.top, 1, rect.Height(), clr);
				rect.DeflateRect(1, 0);
			}
			else
			{
				dc.Draw3dRect(rect, clr, clr);
				rect.DeflateRect(1, 1);
			}

			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			if (Hover)
			{
				LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Color(0xFFFAFDFE), Color(0xFFE8F5FC));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());

				rect.DeflateRect(1, 1);
				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, y), Color(0xFFEAF6FD), Color(0xFFD7EFFC));
				g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), y-rect.top);

				LinearGradientBrush brush3(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xFFBDE6FD), Color(0xFFA6D9F4));
				g.FillRectangle(&brush3, rect.left, y, rect.Width(), rect.bottom-y);
			}
			else
			{
				dc.FillSolidRect(rect, 0xF6E4C2);

				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xFFA9D9F2), Color(0xFF90CBEB));
				g.FillRectangle(&brush2, rect.left, y, rect.Width(), rect.bottom-y);

				LinearGradientBrush brush3(Point(rect.left, rect.top), Point(rect.left, rect.top+2), Color(0x20163145), Color(0x00163145));
				g.FillRectangle(&brush3, rect.left, rect.top, rect.Width(), 2);

				LinearGradientBrush brush4(Point(rect.left, rect.top), Point(rect.left+2, rect.top), Color(0x20163145), Color(0x00163145));
				g.FillRectangle(&brush4, rect.left, rect.top, 2, rect.Height());
			}
		}
		else
		{
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

	dc.SetTextColor(Themed ? Selected || Hover ? 0x000000 : 0x404040 : GetSysColor(COLOR_WINDOWTEXT));
}

void DrawBackstageBorder(Graphics& g, CRect rect)
{
	rect.left++;
	rect.top++;

	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath path;
	CreateRoundRectangle(rect, 3, path);

	SolidBrush brush1(Color(0x80000000));
	g.FillPath(&brush1, &path);

	rect.left--;
	rect.top--;

	g.SetPixelOffsetMode(PixelOffsetModeNone);
	CreateRoundTop(rect, 4, path);

	LinearGradientBrush brush2(Point(0, rect.top-1), Point(0, rect.top+5), Color(0xF0000000), Color(0x00000000));

	Pen pen(Color(0xFF000000));
	pen.SetBrush(&brush2);
	g.DrawPath(&pen, &path);

	CreateRoundRectangle(rect, 4, path);

	LinearGradientBrush brush3(Point(0, rect.top-1), Point(0, rect.bottom), Color(0x00FFFFFF), Color(0x38FFFFFF));

	pen.SetBrush(&brush3);
	g.DrawPath(&pen, &path);
}

void DrawBackstageSelection(CDC& dc, Graphics& g, const CRect& rect, BOOL Selected, BOOL Enabled, BOOL Themed)
{
	if (Selected && Enabled)
	{
		if (Themed)
		{
			LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom-1), Color(0xC020A0FF), Color(0xC01078FF));
			g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height()-1);

			SolidBrush brush2(Color(0x40FFFFFF));
			g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), 1);

			SolidBrush brush3(Color(0x18FFFFFF));
			g.FillRectangle(&brush3, rect.left, rect.top+1, 1, rect.Height()-3);
			g.FillRectangle(&brush3, rect.right-1, rect.top+1, 1, rect.Height()-3);
			g.FillRectangle(&brush3, rect.left, rect.bottom-2, rect.Width(), 1);

			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
	}
	else
	{
		dc.SetTextColor(Enabled ? Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE) : Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW));
	}
}

void DrawBackstageButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Hover, BOOL Pressed, BOOL Enabled, BOOL Themed, BOOL Red)
{
	if (Pressed && Enabled)
	{
		if (Themed)
		{
			g.SetPixelOffsetMode(PixelOffsetModeNone);

			GraphicsPath path;
			CreateRoundRectangle(rect, 3, path);

			LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Red ? Color(0xB0FF0000) : Color(0xE020A0FF), Red ? Color(0xA0FF0000) : Color(0xE01078FF));
			g.FillPath(&brush1, &path);

			Pen pen(Red ? Color(0x80600000) : Color(0x80062D60));
			g.DrawPath(&pen, &path);

			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			rect.left++;
			rect.top++;
			rect.bottom = rect.top+rect.Height()/2;

			if (Red)
			{
				CreateRoundRectangle(rect, 2, path);

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Color(0x60FFFFFF), Color(0x20FFFFFF));
				g.FillPath(&brush2, &path);
			}
			else
			{
				g.SetPixelOffsetMode(PixelOffsetModeNone);

				rect.right--;
				CreateRoundTop(rect, 2, path);

				Pen pen(Color(0x40FFFFFF));
				g.DrawPath(&pen, &path);
			}

			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			rect.top++;
			dc.FillSolidRect(rect, Red ? 0x0000FF : GetSysColor(COLOR_HIGHLIGHT));

			dc.SetTextColor(Red ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
	}
	else
	{
		dc.SetTextColor(Enabled ? Hover ? 0xFFFFFF : Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE) : Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW));
	}
}

void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover)
{
	if (Themed)
	{
		if (Focused || Selected || Hover)
		{
			Graphics g(dc);

			// Inner Border
			rect.DeflateRect(1, 1);

			if (Selected)
			{
				SolidBrush brush(Color(0x20505762));
				g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());
			}
			else
				if (Hover)
				{
					SolidBrush brush1(Color(0x40FFFFFF));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height()/2);

					SolidBrush brush2(Color(0x28A0AFC3));
					g.FillRectangle(&brush2, rect.left, rect.top+rect.Height()/2+1, rect.Width(), rect.Height()-rect.Height()/2);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			GraphicsPath path;

			if (!Selected)
			{
				CreateRoundRectangle(rect, 1, path);

				Pen pen(Color(0x80FFFFFF));
				g.DrawPath(&pen, &path);
			}

			// Outer Border
			rect.InflateRect(1, 1);
			CreateRoundRectangle(rect, 2, path);

			Pen pen(Color(0x70505762));
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		if (Selected || Hover)
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}
}

void DrawWhiteButtonBorder(Graphics& g, LPCRECT lpRect, BOOL IncludeBottom)
{
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath path;
	CreateRoundRectangle(lpRect, 3, path);

	LinearGradientBrush brush1(Point(0, lpRect->top-1), Point(0, lpRect->bottom), Color(0x0C000000), Color(0x00000000));

	Pen pen(Color(0x00000000));
	pen.SetBrush(&brush1);

	g.DrawPath(&pen, &path);

	if (IncludeBottom)
	{
		LinearGradientBrush brush2(Point(0, lpRect->top-1), Point(0, lpRect->bottom), Color(0x00FFFFFF), Color(0xFFFFFFFF));

		pen.SetBrush(&brush2);

		g.DrawPath(&pen, &path);
	}
}

void DrawWhiteButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled, BOOL DrawBorder)
{
	if (Themed)
	{
		if (DrawBorder)
			DrawWhiteButtonBorder(g, rect, FALSE);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(SmoothingModeNone);

		// Inner border
		rect.DeflateRect(2, 2);

		if (Selected)
		{
			dc.FillSolidRect(rect, 0xEDEAE9);

			LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.top+2), Color(0x20000000), Color(0x00000000));
			g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), 2);

			LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left+2, rect.top), Color(0x20000000), Color(0x00000000));
			g.FillRectangle(&brush2, rect.left, rect.top, 1, rect.Height());
		}
		else
		{
			dc.FillSolidRect(rect, Hover ? 0xEFECEC : 0xF7F4F4);

			LinearGradientBrush brush1(Point(0, rect.top+1), Point(0, (rect.top+rect.bottom)/2+1), Color(0xFFFFFFFF), Color((Disabled ? 0x00000000 : 0x40000000) | 0xFFFFFF));
			g.FillRectangle(&brush1, rect.left, rect.top+1, rect.Width(), rect.Height()/2);

			if (!Disabled)
			{
				LinearGradientBrush brush2(Point(0, rect.bottom-3), Point(0, rect.bottom), Color(0x00000000), Color(Hover ? 0x20000000 : 0x10000000));
				g.FillRectangle(&brush2, rect.left, rect.bottom-3, rect.Width(), 3);
			}
		}

		g.SetPixelOffsetMode(PixelOffsetModeNone);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		rect.InflateRect(1, 1);

		GraphicsPath pathOuter;
		CreateRoundRectangle(rect, 2, pathOuter);

		if (Focused || Selected)
		{
			if (!Selected && !Hover)
			{
				rect.DeflateRect(1, 1);

				GraphicsPath pathInner;
				CreateRoundRectangle(rect, 1, pathInner);

				Pen pen(Color(0xC0FFFFFF));
				g.DrawPath(&pen, &pathInner);
			}

			Pen pen(Color(0xFF808397));
			g.DrawPath(&pen, &pathOuter);
		}
		else
		{
			Pen pen(Color(Hover ? 0xFFA6ABB2 : 0xFFBCBDBE));
			g.DrawPath(&pen, &pathOuter);
		}
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}

	dc.SetTextColor(Themed ? Disabled ? 0xA0A0A0 : Focused || Selected || Hover ? 0x000000 : 0x404040 : GetSysColor(Disabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));
}

void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL ShowKeyboardCues)
{
	CRect rect(lpDrawItemStruct->rcItem);
	rect.DeflateRect(6, 4);

	WCHAR Caption[256];
	::GetWindowText(lpDrawItemStruct->hwndItem, Caption, 256);

	UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS;
	if (!ShowKeyboardCues)
		nFormat |= DT_HIDEPREFIX;

	HFONT hOldFont = (HFONT)dc.SelectObject((HFONT)::SendMessage(lpDrawItemStruct->hwndItem, WM_GETFONT, NULL, NULL));
	dc.DrawText(Caption, rect, nFormat);
	dc.SelectObject(hOldFont);
}

void DrawColor(CDC& dc, CRect rect, BOOL Themed, COLORREF clr, BOOL Enabled, BOOL Focused, BOOL Hover)
{
	if (Themed)
	{
		Graphics g(dc);

		DrawWhiteButtonBorder(g, rect, FALSE);

		rect.top++;
		rect.left++;

		if (clr!=(COLORREF)-1)
		{
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			GraphicsPath PathInner;
			CreateRoundRectangle(rect, 2, PathInner);

			SolidBrush brush(Color(COLORREF2RGB(clr) & (Enabled ? 0xFFFFFFFF : 0x3FFFFFFF)));
			g.FillPath(&brush, &PathInner);

			if (Enabled)
			{
				// Boost reflection for light colors
				UINT Alpha = (((clr>>16)*(clr>>16)+6*((clr & 0x00FF00)>>8)*((clr & 0x00FF00)>>8)+5*(clr & 0xFF)*(clr & 0xFF))<<10) & 0xFF000000;

				LinearGradientBrush brush1(Point(0, rect.top), Point(0, (rect.top+rect.bottom)/2+1), Color(Alpha+0x30FFFFFF), Color(Alpha+0x20FFFFFF));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width()-1, rect.Height()/2);

				LinearGradientBrush brush2(Point(0, rect.bottom-6), Point(0, rect.bottom), Color(0x00000000), Color(Hover ? 0x40000000 : 0x20000000));
				g.FillRectangle(&brush2, rect.left+1, rect.bottom-6, rect.Width()-3, 5);
			}

			g.SetPixelOffsetMode(PixelOffsetModeNone);
		}

		rect.right--;
		rect.bottom--;

		GraphicsPath PathOuter;
		CreateRoundRectangle(rect, 2, PathOuter);

		Pen pen(Color(Enabled ? Focused || Hover ? 0x40000000 : (clr!=(COLORREF)-1) ? 0x20000000 : 0x08000000 : 0x10000000));
		g.DrawPath(&pen, &PathOuter);
	}
	else
	{
		if (clr!=(COLORREF)-1)
			dc.FillSolidRect(rect, clr);

		dc.DrawEdge(rect, EDGE_SUNKEN, BF_RECT);

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}
}


// IATA

#include "IATA_DE.h"
#include "IATA_EN.h"

#define ROUNDOFF 0.00000001

static BOOL UseGermanDB = (GetUserDefaultUILanguage() & 0x1FF)==LANG_GERMAN;

UINT FMIATAGetCountryCount()
{
	return UseGermanDB ? CountryCount_DE : CountryCount_EN;
}

UINT FMIATAGetAirportCount()
{
	return UseGermanDB ? AirportCount_DE : AirportCount_EN;
}

const FMCountry* FMIATAGetCountry(UINT CountryID)
{
	return UseGermanDB ? &Countries_DE[CountryID] : &Countries_EN[CountryID];
}

INT FMIATAGetNextAirport(INT Last, FMAirport** ppAirport)
{
	if (Last>=(INT)FMIATAGetAirportCount()-1)
		return -1;

	*ppAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];

	return Last;
}

INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** ppAirport)
{
	UINT Count = FMIATAGetAirportCount();

	do
	{
		if (Last>=(INT)Count-1)
			return -1;

		*ppAirport = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	}
	while ((*ppAirport)->CountryID!=CountryID);

	return Last;
}

BOOL FMIATAGetAirportByCode(LPCSTR pCode, FMAirport** ppAirport)
{
	if (!pCode)
		return FALSE;

	if (strlen(pCode)!=3)
		return FALSE;

	INT First = 0;
	INT Last = (INT)FMIATAGetAirportCount()-1;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;

		*ppAirport = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];

		const INT Result = strcmp((*ppAirport)->Code, pCode);
		if (!Result)
			return TRUE;

		if (Result<0)
		{
			First = Mid+1;
		}
		else
		{
			Last = Mid-1;
		}
	}

	return FALSE;
}

HBITMAP FMIATACreateAirportMap(FMAirport* pAirport, UINT Width, UINT Height)
{
	ASSERT(pAirport);

	// Create bitmap
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	BITMAPINFOHEADER bmi = { sizeof(bmi) };
	bmi.biWidth = Width;
	bmi.biHeight = Height;
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;

	LPBYTE pbData = NULL;
	HBITMAP hBitmap = CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pbData, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	// Draw
	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	Bitmap* pMap = FMGetApp()->GetCachedResourceImage(IDB_BLUEMARBLE_2048);
	const CSize szMap(pMap->GetWidth(), pMap->GetHeight());

	INT LocX = (INT)(((pAirport->Location.Longitude+180.0)*szMap.cx)/360.0);
	INT LocY = (INT)(((pAirport->Location.Latitude+90.0)*szMap.cy)/180.0);

	INT PosX = -LocX+Width/2;
	INT PosY = -LocY+Height/2;

	if (PosY>0)
	{
		PosY = 0;
	}
	else
		if (PosY<(INT)Height-szMap.cy)
		{
			PosY = (INT)Height-szMap.cy;
		}

	ImageAttributes ImgAttr;
	ImgAttr.SetWrapMode(WrapModeTile);

	g.DrawImage(pMap, Rect(0, 0, Width, Height), -PosX, -PosY, Width, Height, UnitPixel, &ImgAttr);

	// Location indicator
	LocX += PosX-8;
	LocY += PosY-8;
	DrawLocationIndicator(g, LocX, LocY, 16);

	// Pfad erstellen
	FontFamily fontFamily(_T("Arial"));
	WCHAR pszBuf[4];
	MultiByteToWideChar(CP_ACP, 0, pAirport->Code, -1, pszBuf, 4);

	StringFormat StrFormat;
	GraphicsPath TextPath;
	TextPath.AddString(pszBuf, -1, &fontFamily, FontStyleRegular, 21, Point(0, 0), &StrFormat);

	// Pfad verschieben
	Rect rt;
	TextPath.GetBounds(&rt);

	INT FntX = LocX+16;
	INT FntY = LocY-rt.Y;

	if (FntY<10)
	{
		FntY = 10;
	}
	else
		if (FntY+rt.Height+10>(INT)Height)
		{
			FntY = Height-rt.Height-10;
		}

	Matrix m;
	m.Translate((Gdiplus::REAL)FntX, (Gdiplus::REAL)(FntY-1));
	TextPath.Transform(&m);

	// Text
	Pen pen(Color(0xFF000000), 3.5);
	pen.SetLineJoin(LineJoinRound);
	g.DrawPath(&pen, &TextPath);

	SolidBrush brush(Color(0xFFFFFFFF));
	g.FillPath(&brush, &TextPath);

	dc.SelectObject(hOldBitmap);

	return hBitmap;
}

__forceinline DOUBLE GetMinutes(DOUBLE c)
{
	c = fabs(c)+ROUNDOFF;

	return (c-(DOUBLE)(INT)c)*60.0;
}

__forceinline DOUBLE GetSeconds(DOUBLE c)
{
	c = fabs(c)*60.0+ROUNDOFF;

	return (c-(DOUBLE)(INT)c)*60.0;
}

void FMGeoCoordinateToString(const DOUBLE c, LPSTR tmpStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero)
{
	sprintf_s(tmpStr, cCount, FillZero ? "%03u�%02u\'%02u\"%c" : "%u�%u\'%u\"%c",
		(UINT)(fabs(c)+ROUNDOFF),
		(UINT)GetMinutes(c),
		(UINT)(GetSeconds(c)+0.5),
		c>0 ? IsLatitude ? 'S' : 'E' : IsLatitude ? 'N' : 'W');
}

void FMGeoCoordinateToString(const DOUBLE c, CString& tmpStr, BOOL IsLatitude, BOOL FillZero)
{
	CHAR Coordinate[16];
	FMGeoCoordinateToString(c, Coordinate, 16, IsLatitude, FillZero);

	tmpStr = Coordinate;
}

void FMGeoCoordinatesToString(const FMGeoCoordinates& c, LPSTR tmpStr, SIZE_T cCount, BOOL FillZero)
{
	if ((c.Latitude==0) && (c.Longitude==0))
	{
		*tmpStr = '\0';
	}
	else
	{
		FMGeoCoordinateToString(c.Latitude, tmpStr, cCount, TRUE, FillZero);

		CHAR Longitude[16];
		FMGeoCoordinateToString(c.Longitude, Longitude, 16, FALSE, FillZero);

		strcat_s(tmpStr, cCount, ", ");
		strcat_s(tmpStr, cCount, Longitude);
	}
}

void FMGeoCoordinatesToString(const FMGeoCoordinates& c, CString& tmpStr, BOOL FillZero)
{
	if ((c.Latitude==0) && (c.Longitude==0))
	{
		tmpStr.Empty();
	}
	else
	{
		CHAR Latitude[16];
		FMGeoCoordinateToString(c.Latitude, Latitude, 16, TRUE, FillZero);

		CHAR Longitude[16];
		FMGeoCoordinateToString(c.Longitude, Longitude, 16, FALSE, FillZero);
		CString L(Longitude);

		tmpStr = Latitude;
		tmpStr.Append(_T(", "));
		tmpStr.Append(L);
	}
}


// License

static BOOL LicenseRead = FALSE;
static BOOL ExpireRead = FALSE;
static FMLicense LicenseBuffer = { 0 };
static FILETIME ExpireBuffer = { 0 };

#define BUFSIZE    4096

void ParseInput(LPSTR pStr, FMLicense* pLicense)
{
	ASSERT(pStr);
	ASSERT(pLicense);

	ZeroMemory(pLicense, sizeof(FMLicense));

	while (*pStr)
	{
		CHAR* pChar = strstr(pStr, "\n");
		if (pChar)
			*pChar = '\0';

		CHAR* pSeparator = strchr(pStr, '=');
		if (pSeparator)
		{
			*(pSeparator++) = '\0';

			if (strcmp(pStr, LICENSE_ID)==0)
			{
				strcpy_s(pLicense->PurchaseID, 256, pSeparator);
			}
			else
				if (strcmp(pStr, LICENSE_PRODUCT)==0)
				{
					strcpy_s(pLicense->ProductID, 256, pSeparator);
				}
				else
					if (strcmp(pStr, LICENSE_DATE)==0)
					{
						strcpy_s(pLicense->PurchaseDate, 256, pSeparator);
					}
					else
						if (strcmp(pStr, LICENSE_QUANTITY)==0)
						{
							strcpy_s(pLicense->Quantity, 256, pSeparator);
						}
						else
							if (strcmp(pStr, LICENSE_NAME)==0)
							{
								strcpy_s(pLicense->RegName, 256, pSeparator);
							}
							else
								if (strcmp(pStr, LICENSE_VERSION)==0)
								{
									sscanf_s(pSeparator, "%u.%u.%u", &pLicense->Version.Major, &pLicense->Version.Minor, &pLicense->Version.Build);
								}
		}

		pStr = pChar+1;
	}
}

__forceinline BOOL ReadCodedLicense(LPCSTR pStr, SIZE_T cCount)
{
	BOOL Result = FALSE;

	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Flightmap", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = (DWORD)cCount;
		Result = (RegQueryValueExA(hKey, "License", 0, NULL, (LPBYTE)pStr, &dwSize)==ERROR_SUCCESS);

		RegCloseKey(hKey);
	}

	return Result;
}

BOOL GetLicense(FMLicense* pLicense)
{
	ASSERT(pLicense);

	CHAR Message[BUFSIZE];
	ZeroMemory(Message, sizeof(Message));
	if (!ReadCodedLicense(Message, sizeof(Message)))
		return FALSE;

	// Setup
	Integer n("677085883040394331688570333377767695119671712512083434059528353754560033694591049061201209797395551894503819202786118921144167773531480249549334860535587729188461269633368144074142410191991825789317089507732335118005174575981046999650747204904573316311747574418394100647266984314883856762401810850517725369369312442712786949893638812875664428840233397180906478896311138092374550604342908484026901612764076340393090750130869987901928694525115652071061067946427802582682353995030622395549260092920885717079018306793778696931528069177410572722700379823625160283051668274004965875876083908201130177783610610417898321219849233028817122323965938052450525299474409115105471423275517732060548499857454724731949257103279342856512067188778813745346304689332770001576020711940974480383875829689815572555429459919998181453447896952351950105505906202024278770099672075754601074409510918531448288487849102192484100291069098446047492850214953085906226731086863049147460384108831179220519130075352506339330781986225289808262743848011070853033928165863801245010514393309413470116317612433324938050068689790531474030013439742900179443199754755961937530375097971295589285864719559221786871735111334987792944096096937793086861538051306485745703623856809.");
	Integer e("17.");

	CHAR Recovered[BUFSIZE];
	ZeroMemory(Recovered, sizeof(Recovered));

	// Verify and recover
	RSASS<PSSR, SHA256>::Verifier Verifier(n, e);

	try
	{
		StringSource(Message, TRUE,
			new Base64Decoder(
				new SignatureVerificationFilter(Verifier,
					new ArraySink((LPBYTE)Recovered, BUFSIZE-1),
					SignatureVerificationFilter::THROW_EXCEPTION | SignatureVerificationFilter::PUT_MESSAGE)));
	}
	catch(CryptoPP::Exception /*&e*/)
	{
		return FALSE;
	}

	ParseInput(Recovered, pLicense);

	return TRUE;
}

BOOL FMIsLicensed(FMLicense* pLicense, BOOL Reload)
{
	// Setup
	if (!LicenseRead || Reload)
	{
		LicenseRead = TRUE;

		if (!GetLicense(&LicenseBuffer))
			return FALSE;
	}

	if (pLicense)
		*pLicense = LicenseBuffer;

	return strncmp(LicenseBuffer.ProductID, "Flightmap", 9)==0;
}

BOOL FMIsSharewareExpired()
{
	if (FMIsLicensed())
		return FALSE;

	// Setup
	if (!ExpireRead)
	{
		GetSystemTimeAsFileTime(&ExpireBuffer);
		ExpireRead = TRUE;

		HKEY hKey;
		if (RegCreateKey(HKEY_CURRENT_USER, _T("Software\\Flightmap"), &hKey)==ERROR_SUCCESS)
		{
			BOOL Result = FALSE;

			DWORD dwSize = sizeof(DWORD);
			if (RegQueryValueEx(hKey, _T("Seed"), 0, NULL, (LPBYTE)&ExpireBuffer.dwHighDateTime, &dwSize)==ERROR_SUCCESS)
			{
				dwSize = sizeof(DWORD);
				if (RegQueryValueEx(hKey, _T("Envelope"), 0, NULL, (LPBYTE)&ExpireBuffer.dwLowDateTime, &dwSize)==ERROR_SUCCESS)
					Result = TRUE;
			}

			if (!Result)
			{
				RegSetValueEx(hKey, _T("Seed"), 0, REG_DWORD, (LPBYTE)&ExpireBuffer.dwHighDateTime, sizeof(DWORD));
				RegSetValueEx(hKey, _T("Envelope"), 0, REG_DWORD, (LPBYTE)&ExpireBuffer.dwLowDateTime, sizeof(DWORD));
			}

			RegCloseKey(hKey);
		}
	}

	// Check
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER FirstInstall;
	FirstInstall.HighPart = ExpireBuffer.dwHighDateTime;
	FirstInstall.LowPart = ExpireBuffer.dwLowDateTime;

	ULARGE_INTEGER Now;
	Now.HighPart = ft.dwHighDateTime;
	Now.LowPart = ft.dwLowDateTime;

#define SECOND ((ULONGLONG)10000000)
#define MINUTE (60*SECOND)
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)

	FirstInstall.QuadPart += 30*DAY;

	return Now.QuadPart>=FirstInstall.QuadPart;
}


// MessageBox

INT FMMessageBox(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type)
{
	FMMessageBoxDlg dlg(pParentWnd, Text, Caption, Type);

	return (INT)dlg.DoModal();
}

void FMErrorBox(CWnd* pParentWnd, UINT nID)
{
	FMMessageBox(pParentWnd, CString((LPCSTR)nID), CString((LPCSTR)IDS_ERROR), MB_OK | MB_ICONERROR);
}
