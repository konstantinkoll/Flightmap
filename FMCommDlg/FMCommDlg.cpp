
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


void CreateRoundRectangle(CRect rect, INT Radius, GraphicsPath& Path)
{
	Path.Reset();

	INT l = rect.left;
	INT t = rect.top;
	INT w = rect.Width();
	INT h = rect.Height();
	INT d = Radius<<1;

	Path.AddArc(l, t, d, d, 180, 90);
	Path.AddLine(l+Radius, t, l+w-Radius, t);
	Path.AddArc(l+w-d, t, d, d, 270, 90);
	Path.AddLine(l+w, t+Radius, l+w, t+h-Radius);
	Path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	Path.AddLine(l+w-Radius, t+h, l+Radius, t+h);
	Path.AddArc(l, t+h-d, d, d, 90, 90);
	Path.AddLine(l, t+h-Radius, l, t+Radius);
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

	HDC hDC = GetDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hDC, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
	ReleaseDC(NULL, hDC);

	return hBitmap;
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

FMCountry* FMIATAGetCountry(UINT CountryID)
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

BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** ppAirport)
{
	if (!Code)
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
	HGDIOBJ hOldBitmap = dc.SelectObject(hBitmap);

	// Draw
	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	CGdiPlusBitmap* pMap = FMGetApp()->GetCachedResourceImage(IDB_EARTHMAP, _T("JPG"));
	CGdiPlusBitmap* pIndicator = FMGetApp()->GetCachedResourceImage(IDB_LOCATIONINDICATOR_16, _T("PNG"));

	INT L = pMap->m_pBitmap->GetWidth();
	INT H = pMap->m_pBitmap->GetHeight();
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

	if (PosX>1)
		g.DrawImage(pMap->m_pBitmap, PosX-L, PosY, L, H);
	g.DrawImage(pMap->m_pBitmap, PosX, PosY, L, H);
	if (PosX<(INT)Width-L)
		g.DrawImage(pMap->m_pBitmap, PosX+L, PosY, L, H);

	LocX += PosX-pIndicator->m_pBitmap->GetWidth()/2+1;
	LocY += PosY-pIndicator->m_pBitmap->GetHeight()/2+1;
	g.DrawImage(pIndicator->m_pBitmap, LocX, LocY);

	// Pfad erstellen
	FontFamily fontFamily(FMGetApp()->GetDefaultFontFace());
	WCHAR pszBuf[4];
	MultiByteToWideChar(CP_ACP, 0, pAirport->Code, -1, pszBuf, 4);

	StringFormat strformat;
	GraphicsPath TextPath;
	TextPath.Reset();
	TextPath.AddString(pszBuf, (INT)wcslen(pszBuf), &fontFamily, FontStyleRegular, 21, Gdiplus::Point(0, 0), &strformat);

	// Pfad verschieben
	Rect tr;
	TextPath.GetBounds(&tr);

	INT FntX = LocX+pIndicator->m_pBitmap->GetWidth();
	INT FntY = LocY-tr.Y;

	if (FntY<10)
	{
		FntY = 10;
	}
	else
		if (FntY+tr.Height+10>(INT)Height)
		{
			FntY = Height-tr.Height-10;
		}
	Matrix m;
	m.Translate((Gdiplus::REAL)FntX, (Gdiplus::REAL)FntY-1.25f);
	TextPath.Transform(&m);

	// Text
	Pen pen(Color(0, 0, 0), 3.0);
	pen.SetLineJoin(LineJoinRound);
	g.DrawPath(&pen, &TextPath);
	SolidBrush brush(Color(255, 255, 255));
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
		DWORD dwSize = cCount;
		Result = (RegQueryValueExA(hKey, "License", 0, NULL, (BYTE*)pStr, &dwSize)==ERROR_SUCCESS);

		RegCloseKey(hKey);
	}

	return Result;
}

BOOL GetLicense(FMLicense* pLicense)
{
	ASSERT(pLicense);

	CHAR Message[BUFSIZE];
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

void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright)
{
	if (Version)
		Version->Empty();
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

				if (Version)
					if (VerQueryValue(lpInfo, _T("\\"), &valPtr, &valLen))
					{
						VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
						Version->Format(_T("%u.%u.%u"), 
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
		CurrentVersion += _T(" (licensed");
	}
	else
		if (FMIsSharewareExpired())
		{
			CurrentVersion += _T(" (expired");
		}

	CurrentVersion += _T(")");

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

CString GetIniValue(CString Ini, CString Name)
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

void ParseVersion(CString tmpStr, FMVersion* pVersion)
{
	ASSERT(pVersion);

	swscanf_s(tmpStr, L"%u.%u.%u", &pVersion->Major, &pVersion->Minor, &pVersion->Build);
}

void FMCheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	// Obtain current version from instance version resource
	CString CurrentVersion;
	GetFileVersion(AfxGetResourceHandle(), &CurrentVersion);

	// Check due?
	BOOL Check = Force;
	if (!Check)
		Check = FMGetApp()->IsUpdateCheckDue();

	// Perform check
	CString LatestVersion = FMGetApp()->GetString(_T("LatestUpdateVersion"));
	CString LatestMSN = FMGetApp()->GetString(_T("LatestUpdateMSN"));

	if (Check)
	{
		CWaitCursor wait;
		CString VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));

			FMGetApp()->WriteString(_T("LatestUpdateVersion"), LatestVersion);
			FMGetApp()->WriteString(_T("LatestUpdateMSN"), LatestMSN);
		}
	}

	// Update available?
	BOOL UpdateAvailable = FALSE;
	if (!LatestVersion.IsEmpty())
	{
		FMVersion CV = { 0 };
		ParseVersion(CurrentVersion, &CV);

		FMVersion LV = { 0 };
		ParseVersion(LatestVersion, &LV);

		CString IgnoreMSN = FMGetApp()->GetString(_T("IgnoreUpdateMSN"));

		UpdateAvailable = ((IgnoreMSN!=LatestMSN) || (Force)) &&
			((LV.Major>CV.Major) ||
			((LV.Major==CV.Major) && (LV.Minor>CV.Minor)) ||
			((LV.Major==CV.Major) && (LV.Minor==CV.Minor) && (LV.Build>CV.Build)));
	}

	// Result
	if (UpdateAvailable)
	{
		if (pParentWnd)
		{
			if (FMGetApp()->m_pUpdateNotification)
				FMGetApp()->m_pUpdateNotification->DestroyWindow();

			FMUpdateDlg dlg(LatestVersion, LatestMSN, pParentWnd);
			dlg.DoModal();
		}
		else
			if (FMGetApp()->m_pUpdateNotification)
			{
				FMGetApp()->m_pUpdateNotification->SendMessage(WM_COMMAND, IDM_UPDATE_RESTORE);
			}
			else
			{
				FMGetApp()->m_pUpdateNotification = new FMUpdateDlg(LatestVersion, LatestMSN);
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
