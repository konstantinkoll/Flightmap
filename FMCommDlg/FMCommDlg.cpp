
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
using namespace std;


void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path)
{
	path.Reset();

	INT l = rect.left;
	INT t = rect.top;
	INT w = rect.Width();
	INT h = rect.Height();
	INT d = rad<<1;

	path.AddArc(l, t, d, d, 180, 90);
	path.AddLine(l+rad, t, l+w-rad, t);
	path.AddArc(l+w-d, t, d, d, 270, 90);
	path.AddLine(l+w, t+rad, l+w, t+h-rad);
	path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	path.AddLine(l+w-rad, t+h, l+rad, t+h);
	path.AddArc(l, t+h-d, d, d, 90, 90);
	path.AddLine(l, t+h-rad, l, t+rad);
	path.CloseFigure();
}

BOOL IsCtrlThemed()
{
	FMApplication* pApp = FMGetApp();
	if (pApp)
		if (pApp->m_ThemeLibLoaded)
			return pApp->zIsAppThemed();

	return FALSE;
}

void DrawControlBorder(CWnd* pWnd)
{
	CRect rect;
	pWnd->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(pWnd);

	FMApplication* pApp = FMGetApp();
	if (pApp->m_ThemeLibLoaded)
		if (pApp->zIsAppThemed())
		{
			HTHEME hTheme = pApp->zOpenThemeData(pWnd->GetSafeHwnd(), VSCLASS_LISTBOX);
			if (hTheme)
			{
				CRect rectClient(rect);
				rectClient.DeflateRect(2, 2);
				dc.ExcludeClipRect(rectClient);

				pApp->zDrawThemeBackground(hTheme, dc, LBCP_BORDER_NOSCROLL, pWnd->IsWindowEnabled() ? GetFocus()==pWnd->GetSafeHwnd() ? LBPSN_FOCUSED : LBPSN_NORMAL : LBPSN_DISABLED, rect, rect);
				pApp->zCloseThemeData(hTheme);

				return;
			}
		}

	dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, 0x000000, GetSysColor(COLOR_3DFACE));
}

void FMErrorBox(UINT nResID, HWND hWnd)
{
	CString caption;
	CString message;
	ENSURE(caption.LoadString(IDS_ERROR));
	ENSURE(message.LoadString(nResID));

	MessageBox(hWnd, message, caption, MB_OK | MB_ICONERROR);
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

FMCountry* FMIATAGetCountry(UINT ID)
{
	return UseGermanDB ? &Countries_DE[ID] : &Countries_EN[ID];
}

INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer)
{
	if (Last>=(INT)FMIATAGetAirportCount()-1)
		return -1;

	*pBuffer = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	return Last;
}

INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer)
{
	INT Count = (INT)FMIATAGetAirportCount();

	do
	{
		if (Last>=Count-1)
			return -1;

		*pBuffer = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	}
	while ((*pBuffer)->CountryID!=CountryID);

	return Last;
}

BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer)
{
	if (!Code)
		return FALSE;
	if (*Code=='\0')
		return FALSE;

	INT First = 0;
	INT Last = (INT)FMIATAGetAirportCount()-1;

	while (First<=Last)
	{
		INT Mid = (First+Last)/2;

		*pBuffer = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];
		INT Res = strcmp((*pBuffer)->Code, Code);
		if (Res==0)
			return TRUE;

		if (Res<0)
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

	CGdiPlusBitmap* pMap = FMGetApp()->GetCachedResourceImage(IDB_EARTHMAP_2048, _T("JPG"));
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
static FILETIME ExpireBuffer = { 0, 0 };

void ParseVersion(string& tmpStr, FMLicenseVersion* Version)
{
	CHAR Point;

	stringstream ss(tmpStr);
	ss >> Version->Major;
	ss >> Point;
	ss >> Version->Minor;
	ss >> Point;
	ss >> Version->Release;
}

void ParseInput(string& tmpStr, FMLicense* License)
{
	ZeroMemory(License, sizeof(FMLicense));

	stringstream ss(tmpStr);
	string line;

	while (!ss.eof())
	{
		getline(ss, line);
		std::string::size_type delimiterPos = line.find_first_of("=");

		if (std::string::npos!=delimiterPos)
		{
			std::string name = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos+1);

			if (name==LICENSE_ID)
			{
				MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->PurchaseID, 256);
			}
			else
				if (name==LICENSE_PRODUCT)
				{
					MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->ProductID, 256);
				}
				else
					if (name==LICENSE_DATE)
					{
						MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->PurchaseDate, 16);
					}
					else
						if (name==LICENSE_QUANTITY)
						{
							MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->Quantity, 8);
						}
						else
							if (name==LICENSE_NAME)
							{
								MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, License->RegName, 256);
							}
							else
								if (name==LICENSE_VERSION)
								{
									ParseVersion(value, &License->Version);
								}
		}
	}
}

BOOL ReadCodedLicense(string& Message)
{
	BOOL res = FALSE;

	HKEY k;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Flightmap", &k)==ERROR_SUCCESS)
	{
		CHAR tmpStr[4096];
		DWORD sz = sizeof(tmpStr);
		if (RegQueryValueExA(k, "License", 0, NULL, (BYTE*)&tmpStr, &sz)==ERROR_SUCCESS)
		{
			Message = tmpStr;
			res = TRUE;
		}

		RegCloseKey(k);
	}

	return res;
}

BOOL GetLicense(FMLicense* License)
{
	string Message;
	string Recovered;

	if (!ReadCodedLicense(Message))
		return FALSE;

	// Setup
	Integer n("677085883040394331688570333377767695119671712512083434059528353754560033694591049061201209797395551894503819202786118921144167773531480249549334860535587729188461269633368144074142410191991825789317089507732335118005174575981046999650747204904573316311747574418394100647266984314883856762401810850517725369369312442712786949893638812875664428840233397180906478896311138092374550604342908484026901612764076340393090750130869987901928694525115652071061067946427802582682353995030622395549260092920885717079018306793778696931528069177410572722700379823625160283051668274004965875876083908201130177783610610417898321219849233028817122323965938052450525299474409115105471423275517732060548499857454724731949257103279342856512067188778813745346304689332770001576020711940974480383875829689815572555429459919998181453447896952351950105505906202024278770099672075754601074409510918531448288487849102192484100291069098446047492850214953085906226731086863049147460384108831179220519130075352506339330781986225289808262743848011070853033928165863801245010514393309413470116317612433324938050068689790531474030013439742900179443199754755961937530375097971295589285864719559221786871735111334987792944096096937793086861538051306485745703623856809.");
	Integer e("17.");

	// Verify and recover
	RSASS<PSSR, SHA256>::Verifier Verifier(n, e);

	try
	{
		StringSource(Message, true,
			new Base64Decoder(
				new SignatureVerificationFilter(Verifier,
					new StringSink(Recovered),
					SignatureVerificationFilter::THROW_EXCEPTION | SignatureVerificationFilter::PUT_MESSAGE)));
	}
	catch(CryptoPP::Exception /*&e*/)
	{
		return FALSE;
	}

	ParseInput(Recovered, License);
	return TRUE;
}

BOOL FMIsLicensed(FMLicense* License, BOOL Reload)
{
	// Setup
	if (!LicenseRead || Reload)
	{
		LicenseRead = TRUE;

		if (!GetLicense(&LicenseBuffer))
			return FALSE;
	}

	if (License)
		*License = LicenseBuffer;

	return (wcsncmp(LicenseBuffer.ProductID, _T("Flightmap"), 9)==0) && (LicenseBuffer.Version.Major>=0);
}

BOOL FMIsSharewareExpired()
{
	if (FMIsLicensed())
		return FALSE;

	// Setup
	if (!ExpireRead)
	{
		ExpireRead = TRUE;

		BOOL res = FALSE;

		HKEY k;
		if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Flightmap"), &k)==ERROR_SUCCESS)
		{
			DWORD sz = sizeof(DWORD);
			if (RegQueryValueEx(k, _T("Seed"), 0, NULL, (BYTE*)&ExpireBuffer.dwHighDateTime, &sz)==ERROR_SUCCESS)
			{
				sz = sizeof(DWORD);
				if (RegQueryValueEx(k, _T("Envelope"), 0, NULL, (BYTE*)&ExpireBuffer.dwLowDateTime, &sz)==ERROR_SUCCESS)
					res = TRUE;
			}

			if (!res)
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

CString GetLatestVersion(CString& CurrentVersion)
{
	CString VersionIni;

	// Obtain current version from instance version resource
	GetFileVersion(AfxGetResourceHandle(), &CurrentVersion);

	// Variant
#ifdef _M_X64
#define ISET _T(" (x64")
#else
#define ISET _T(" (x86")
#endif

	CurrentVersion += ISET;

	// Licensed?
	if (FMIsLicensed())
	{
		CurrentVersion += _T("; licensed");
	}
	else
		if (FMIsSharewareExpired())
		{
			CurrentVersion += _T("; expired");
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
									delete[] pBuffer;
								}
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
		INT pos = Ini.Find(L"\n");
		if (pos==-1)
			pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, pos-1);
		Ini.Delete(0, pos+1);

		pos = Line.Find(L"=");
		if (pos!=-1)
			if (Line.Mid(0, pos)==Name)
				return Line.Mid(pos+1, Line.GetLength()-pos);
	}

	return _T("");
}

struct Version
{
	UINT Major, Minor, Build;
};

__forceinline INT ParseVersion(CString ver, Version* v)
{
	ZeroMemory(v, sizeof(Version));
	return swscanf_s(ver, L"%u.%u.%u", &v->Major, &v->Minor, &v->Build);
}

void FMCheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	BOOL UpdateFound = FALSE;
	BOOL Check = Force;

	// Check due?
	if (!Check)
		Check = FMGetApp()->IsUpdateCheckDue();

	// Perform check
	CString VersionIni;
	CString LatestVersion;
	CString LatestMSN;
	Check=TRUE;
	if (Check)
	{
		CWaitCursor wait;

		CString CurrentVersion;
		VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));
			if (!LatestVersion.IsEmpty())
			{
				Version CV;
				Version LV;
				ParseVersion(CurrentVersion, &CV);
				ParseVersion(LatestVersion, &LV);

				UpdateFound = (LV.Major>CV.Major) ||
					((LV.Major==CV.Major) && (LV.Minor>CV.Minor)) ||
					((LV.Major==CV.Major) && (LV.Minor==CV.Minor) && (LV.Build>CV.Build));
			}
		}
	}

	// Result
	UpdateFound=TRUE;
	if (UpdateFound)
	{
		if (pParentWnd)
		{
			FMUpdateDlg dlg(LatestVersion, LatestMSN, pParentWnd);
			dlg.DoModal();
		}
		else
		{
			FMUpdateDlg* pUpdateDlg = new FMUpdateDlg(LatestVersion, LatestMSN);
			pUpdateDlg->Create(IDD_UPDATE, CWnd::GetDesktopWindow());
			pUpdateDlg->ShowWindow(SW_SHOW);
		}
	}
	else
		if (Force)
		{
			CString Caption;
			CString Text;
			ENSURE(Caption.LoadString(IDS_UPDATE));
			ENSURE(Text.LoadString(IDS_UPDATENOTAVAILABLE));

			MessageBox(pParentWnd->GetSafeHwnd(), Text, Caption, MB_ICONINFORMATION | MB_OK);
		}
}
