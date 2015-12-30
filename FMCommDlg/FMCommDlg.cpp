
// FMCommDlg.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
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

void DrawReflection(Graphics& g, LPCRECT lpRect)
{
	GraphicsPath pathReflection;
	CreateReflectionRectangle(lpRect, 2, pathReflection);

	LinearGradientBrush brush(Point(lpRect->left, lpRect->top), Point(lpRect->left+min((INT)((lpRect->bottom-lpRect->top)*1.681), lpRect->right-lpRect->left), lpRect->bottom), Color(0x28, 0xFF, 0xFF, 0xFF), Color(0x10, 0xFF, 0xFF, 0xFF));
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
				LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0x20, 0xA0, 0xFF), Color(0x10, 0x80, 0xE0));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
			}
			else
				if (Hover)
				{
					LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0xF9, 0xFC, 0xFF), Color(0xE0, 0xEB, 0xFA));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
				}

			g.SetPixelOffsetMode(PixelOffsetModeNone);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			if ((ShowFocusRect && WinFocused) || Hover || Selected)
				if ((Focused && ShowFocusRect && WinFocused) || Selected)
				{
					Pen pen1(Color(0x10, 0x80, 0xE0));
					g.DrawPath(&pen1, &pathOuter);
				}
				else
				{
					Pen pen1(Color(0x8A, 0xC0, 0xF0));
					g.DrawPath(&pen1, &pathOuter);
				}

			if (Hover || Selected)
			{
				Pen pen2(Color((Hover && !Selected) ? 0x60 : 0x48, 0xFF, 0xFF, 0xFF));
				g.DrawPath(&pen2, &pathInner);
			}
		}

		dc.SetTextColor(Selected ? 0xFFFFFF : TextColor!=(COLORREF)-1 ? TextColor : 0x000000);
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(WinFocused ? COLOR_HIGHLIGHT : COLOR_3DFACE));
			dc.SetTextColor(GetSysColor(WinFocused ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
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

void DrawSubitemBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal)
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

			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			if (Hover)
			{
				LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Color(0xFA, 0xFD, 0xFE), Color(0xE8, 0xF5, 0xFC));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());

				rect.DeflateRect(1, 1);
				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, y), Color(0xEA, 0xF6, 0xFD), Color(0xD7, 0xEF, 0xFC));
				g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), y-rect.top);

				LinearGradientBrush brush3(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xBD, 0xE6, 0xFD), Color(0xA6, 0xD9, 0xF4));
				g.FillRectangle(&brush3, rect.left, y, rect.Width(), rect.bottom-y);
			}
			else
			{
				dc.FillSolidRect(rect, 0xF6E4C2);

				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xA9, 0xD9, 0xF2), Color(0x90, 0xCB, 0xEB));
				g.FillRectangle(&brush2, rect.left, y, rect.Width(), rect.bottom-y);

				LinearGradientBrush brush3(Point(rect.left, rect.top), Point(rect.left, rect.top+2), Color(0x20, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
				g.FillRectangle(&brush3, rect.left, rect.top, rect.Width(), 2);

				LinearGradientBrush brush4(Point(rect.left, rect.top), Point(rect.left+2, rect.top), Color(0x20, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
				g.FillRectangle(&brush4, rect.left, rect.top, 2, rect.Height());
			}
		}
		else
		{
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

	dc.SetTextColor(Themed ? Selected || Hover ? 0x000000 : 0x404040 : GetSysColor(COLOR_WINDOWTEXT));
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
				SolidBrush brush(Color(0x20, 0x50, 0x57, 0x62));
				g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());
			}
			else
				if (Hover)
				{
					SolidBrush brush1(Color(0x40, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height()/2);

					SolidBrush brush2(Color(0x28, 0xA0, 0xAF, 0xC3));
					g.FillRectangle(&brush2, rect.left, rect.top+rect.Height()/2+1, rect.Width(), rect.Height()-rect.Height()/2);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			GraphicsPath path;

			if (!Selected)
			{
				CreateRoundRectangle(rect, 1, path);

				Pen pen(Color(0x80, 0xFF, 0xFF, 0xFF));
				g.DrawPath(&pen, &path);
			}

			// Outer Border
			rect.InflateRect(1, 1);
			CreateRoundRectangle(rect, 2, path);

			Pen pen(Color(0x70, 0x50, 0x57, 0x62));
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

void FMErrorBox(UINT nID, HWND hWnd)
{
	CString Caption((LPCSTR)IDS_ERROR);
	CString Message((LPCSTR)nID);

	MessageBox(hWnd, Message, Caption, MB_OK | MB_ICONERROR);
}


// IATA-Datenbank
//

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

BOOL FMIATAGetAirportByCode(const CHAR* Code, FMAirport** ppAirport)
{
	if (!Code)
		return FALSE;

	if (strlen(Code)!=3)
		return FALSE;

	INT First = 0;
	INT Last = (INT)FMIATAGetAirportCount()-1;

	while (First<=Last)
	{
		INT Mid = (First+Last)/2;

		*ppAirport = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];

		INT Result = strcmp((*ppAirport)->Code, Code);
		if (Result==0)
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

	BYTE* pbData = NULL;
	HBITMAP hBitmap = CreateDIBSection(dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pbData, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	// Draw
	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	Bitmap* pMap = FMGetApp()->GetCachedResourceImage(IDB_EARTHMAP);
	Bitmap* pIndicator = FMGetApp()->GetCachedResourceImage(IDB_LOCATIONINDICATOR_16);

	INT L = pMap->GetWidth();
	INT H = pMap->GetHeight();
	INT LocX = (INT)(((pAirport->Location.Longitude+180.0)*L)/360.0);
	INT LocY = (INT)(((pAirport->Location.Latitude+90.0)*H)/180.0);
	INT PosX = -LocX+Width/2;
	INT PosY = -LocY+Height/2;
	if (PosY>1)
	{
		PosY = 1;
	}
	else
		if (PosY<(INT)Height-H)
		{
			PosY = Height-H;
		}

	ImageAttributes ImgAttr;
	ImgAttr.SetWrapMode(WrapModeTile);

	g.DrawImage(pMap, Rect(0, 0, Width, Height), -PosX, -PosY, Width, Height, UnitPixel, &ImgAttr);

	LocX += PosX-pIndicator->GetWidth()/2+1;
	LocY += PosY-pIndicator->GetHeight()/2+1;
	g.DrawImage(pIndicator, LocX, LocY);

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

	INT FntX = LocX+pIndicator->GetWidth();
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
	m.Translate((Gdiplus::REAL)FntX, (Gdiplus::REAL)FntY-1.0f);
	TextPath.Transform(&m);

	// Text
	Pen pen(Color(0x00, 0x00, 0x00), 3.5);
	pen.SetLineJoin(LineJoinRound);
	g.DrawPath(&pen, &TextPath);

	SolidBrush brush(Color(0xFF, 0xFF, 0xFF));
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

void FMGeoCoordinateToString(const DOUBLE c, CHAR* tmpStr, UINT cCount, BOOL IsLatitude, BOOL FillZero)
{
	sprintf_s(tmpStr, cCount, FillZero ? "%03u°%02u\'%02u\"%c" : "%u°%u\'%u\"%c",
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

void FMGeoCoordinatesToString(const FMGeoCoordinates c, CHAR* tmpStr, UINT cCount, BOOL FillZero)
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

void FMGeoCoordinatesToString(const FMGeoCoordinates c, CString& tmpStr, BOOL FillZero)
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


// Lizensierung
//

static BOOL LicenseRead = FALSE;
static BOOL ExpireRead = FALSE;
static FMLicense LicenseBuffer = { 0 };
static FILETIME ExpireBuffer = { 0 };

#define BUFSIZE    4096

void ParseInput(CHAR* pStr, FMLicense* pLicense)
{
	ASSERT(pStr);
	ASSERT(pLicense);

	ZeroMemory(pLicense, sizeof(FMLicense));

	while (*pStr)
	{
		CHAR* Ptr = strstr(pStr, "\n");
		*Ptr = '\0';

		CHAR* Trenner = strchr(pStr, '=');
		if (Trenner)
		{
			*(Trenner++) = '\0';

			if (strcmp(pStr, LICENSE_ID)==0)
			{
				strcpy_s(pLicense->PurchaseID, 256, Trenner);
			}
			else
				if (strcmp(pStr, LICENSE_PRODUCT)==0)
				{
					strcpy_s(pLicense->ProductID, 256, Trenner);
				}
				else
					if (strcmp(pStr, LICENSE_DATE)==0)
					{
						strcpy_s(pLicense->PurchaseDate, 256, Trenner);
					}
					else
						if (strcmp(pStr, LICENSE_QUANTITY)==0)
						{
							strcpy_s(pLicense->Quantity, 256, Trenner);
						}
						else
							if (strcmp(pStr, LICENSE_NAME)==0)
							{
								strcpy_s(pLicense->RegName, 256, Trenner);
							}
							else
								if (strcmp(pStr, LICENSE_VERSION)==0)
								{
									sscanf_s(Trenner, "%u.%u.%u", &pLicense->Version.Major, &pLicense->Version.Minor, &pLicense->Version.Build);
								}
		}

		pStr = Ptr+1;
	}
}

__forceinline BOOL ReadCodedLicense(CHAR* pStr, SIZE_T cCount)
{
	BOOL Result = FALSE;

	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Flightmap", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = (DWORD)cCount;
		Result = (RegQueryValueExA(hKey, "License", 0, NULL, (BYTE*)pStr, &dwSize)==ERROR_SUCCESS);

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
					new ArraySink((BYTE*)Recovered, BUFSIZE-1),
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
		ExpireRead = TRUE;

		BOOL Result = FALSE;

		HKEY k;
		if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Flightmap"), &k)==ERROR_SUCCESS)
		{
			DWORD sz = sizeof(DWORD);
			if (RegQueryValueEx(k, _T("Seed"), 0, NULL, (BYTE*)&ExpireBuffer.dwHighDateTime, &sz)==ERROR_SUCCESS)
			{
				sz = sizeof(DWORD);
				if (RegQueryValueEx(k, _T("Envelope"), 0, NULL, (BYTE*)&ExpireBuffer.dwLowDateTime, &sz)==ERROR_SUCCESS)
					Result = TRUE;
			}

			if (!Result)
			{
				GetSystemTimeAsFileTime(&ExpireBuffer);
				RegSetValueEx(k, _T("Seed"), 0, REG_DWORD, (BYTE*)&ExpireBuffer.dwHighDateTime, sizeof(DWORD));
				RegSetValueEx(k, _T("Envelope"), 0, REG_DWORD, (BYTE*)&ExpireBuffer.dwLowDateTime, sizeof(DWORD));
			}

			RegCloseKey(k);
		}
	}

	// Check
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER FirstInstall;
	FirstInstall.HighPart = ExpireBuffer.dwHighDateTime;
	FirstInstall.LowPart = ExpireBuffer.dwHighDateTime;

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


// Update
//

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
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("© liquidFOLDERS");
			}

			delete[] lpInfo;
		}
	}
}

CString GetLatestVersion(CString CurrentVersion)
{
	CString VersionIni;

	// Licensed?
	if (FMIsLicensed())
	{
		CurrentVersion += _T(" (licensed)");
	}
	else
		if (FMIsSharewareExpired())
		{
			CurrentVersion += _T(" (expired)");
		}

	// Get available version
	HINTERNET hSession = WinHttpOpen(_T("Flightmap/")+CurrentVersion, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, L"update.flightmap.net", INTERNET_DEFAULT_HTTP_PORT, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/version.ini", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (hRequest)
			{
				if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
					if (WinHttpReceiveResponse(hRequest, NULL))
					{
						DWORD dwSize;

						do
						{
							dwSize = 0;
							if (WinHttpQueryDataAvailable(hRequest, &dwSize))
							{
								CHAR* pBuffer = new CHAR[dwSize+1];

								DWORD dwDownloaded;
								if (WinHttpReadData(hRequest, pBuffer, dwSize, &dwDownloaded))
								{
									pBuffer[dwDownloaded] = '\0';
									CString tmpStr(pBuffer);
									VersionIni += tmpStr;
								}

								delete[] pBuffer;
							}
						}
						while (dwSize>0);
					}

				WinHttpCloseHandle(hRequest);
			}

			WinHttpCloseHandle(hConnect);
		}

		WinHttpCloseHandle(hSession);
	}

	return VersionIni;
}

CString GetIniValue(CString Ini, const CString& Name)
{
	while (!Ini.IsEmpty())
	{
		INT Pos = Ini.Find(L"\n");
		if (Pos==-1)
			Pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, Pos-1);
		Ini.Delete(0, Pos+1);

		Pos = Line.Find(L"=");
		if (Pos!=-1)
			if (Line.Mid(0, Pos)==Name)
				return Line.Mid(Pos+1, Line.GetLength()-Pos);
	}

	return _T("");
}

void ParseVersion(const CString& tmpStr, FMVersion* pVersion)
{
	ASSERT(pVersion);

	swscanf_s(tmpStr, L"%u.%u.%u", &pVersion->Major, &pVersion->Minor, &pVersion->Build);
}

BOOL IsVersionLater(const FMVersion& LatestVersion, const FMVersion& CurrentVersion)
{
	return ((LatestVersion.Major>CurrentVersion.Major) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor>CurrentVersion.Minor)) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor==CurrentVersion.Minor) && (LatestVersion.Build>CurrentVersion.Build)));
}

BOOL IsLaterFeature(const CString VersionIni, const CString Name, FMVersion& CurrentVersion)
{
	FMVersion FeatureVersion = { 0 };

	ParseVersion(GetIniValue(VersionIni, Name), &FeatureVersion);

	return IsVersionLater(FeatureVersion, CurrentVersion);
}

DWORD GetFeatures(const CString& VersionIni, FMVersion& CurrentVersion)
{
	DWORD Features = 0;

	if (IsLaterFeature(VersionIni, _T("SecurityPatch"), CurrentVersion))
		Features |= UPDATE_SECUTIRYPATCH;

	if (IsLaterFeature(VersionIni, _T("ImportantBugfix"), CurrentVersion))
		Features |= UPDATE_IMPORTANTBUGFIX;

	if (IsLaterFeature(VersionIni, _T("NetworkAPI"), CurrentVersion))
		Features |= UPDATE_NETWORKAPI;

	if (IsLaterFeature(VersionIni, _T("NewFeature"), CurrentVersion))
		Features |= UPDATE_NEWFEATURE;

	if (IsLaterFeature(VersionIni, _T("NewVisualization"), CurrentVersion))
		Features |= UPDATE_NEWVISUALIZATION;

	if (IsLaterFeature(VersionIni, _T("UI"), CurrentVersion))
		Features |= UPDATE_UI;

	if (IsLaterFeature(VersionIni, _T("SmallBugfix"), CurrentVersion))
		Features |= UPDATE_SMALLBUGFIX;

	if (IsLaterFeature(VersionIni, _T("IATA"), CurrentVersion))
		Features |= UPDATE_IATA;

	if (IsLaterFeature(VersionIni, _T("Performance"), CurrentVersion))
		Features |= UPDATE_PERFORMANCE;

	return Features;
}

void FMCheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	// Obtain current version from instance version resource
	CString CurrentVersion;
	GetFileVersion(AfxGetResourceHandle(), CurrentVersion);

	FMVersion CV = { 0 };
	ParseVersion(CurrentVersion, &CV);

	// Check due?
	BOOL Check = Force;
	if (!Check)
		Check = FMGetApp()->IsUpdateCheckDue();

	// Perform check
	CString LatestVersion = FMGetApp()->GetString(_T("LatestUpdateVersion"));
	CString LatestMSN = FMGetApp()->GetString(_T("LatestUpdateMSN"));
	DWORD LatestFeatures = FMGetApp()->GetInt(_T("LatestUpdateFeatures"));

	if (Check)
	{
		CWaitCursor wait;
		CString VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));
			LatestFeatures = GetFeatures(VersionIni, CV);

			FMGetApp()->WriteString(_T("LatestUpdateVersion"), LatestVersion);
			FMGetApp()->WriteString(_T("LatestUpdateMSN"), LatestMSN);
			FMGetApp()->WriteInt(_T("LatestUpdateFeatures"), LatestFeatures);
		}
	}

	// Update available?
	BOOL UpdateAvailable = FALSE;
	if (!LatestVersion.IsEmpty())
	{
		FMVersion LV = { 0 };
		ParseVersion(LatestVersion, &LV);

		CString IgnoreMSN = FMGetApp()->GetString(_T("IgnoreUpdateMSN"));

		UpdateAvailable = ((IgnoreMSN!=LatestMSN) || (Force)) && IsVersionLater(LV, CV);
	}

	// Result
	if (UpdateAvailable)
	{
		if (pParentWnd)
		{
			if (FMGetApp()->m_pUpdateNotification)
				FMGetApp()->m_pUpdateNotification->DestroyWindow();

			FMUpdateDlg dlg(LatestVersion, LatestMSN, LatestFeatures, pParentWnd);
			dlg.DoModal();
		}
		else
			if (FMGetApp()->m_pUpdateNotification)
			{
				FMGetApp()->m_pUpdateNotification->SendMessage(WM_COMMAND, IDM_UPDATE_RESTORE);
			}
			else
			{
				FMGetApp()->m_pUpdateNotification = new FMUpdateDlg(LatestVersion, LatestMSN, LatestFeatures);
				FMGetApp()->m_pUpdateNotification->Create(IDD_UPDATE, CWnd::GetDesktopWindow());
				FMGetApp()->m_pUpdateNotification->ShowWindow(SW_SHOW);
			}
	}
	else
		if (Force)
		{
			CString Caption((LPCSTR)IDS_UPDATE);
			CString Text((LPCSTR)IDS_UPDATENOTAVAILABLE);

			MessageBox(pParentWnd->GetSafeHwnd(), Text, Caption, MB_ICONINFORMATION | MB_OK);
		}
}
