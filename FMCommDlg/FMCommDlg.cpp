
// FMCommDlg.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <sstream>

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

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)


FMCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path)
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

FMCommDlg_API BOOL IsCtrlThemed()
{
	FMApplication* pApp = (FMApplication*)AfxGetApp();
	if (pApp)
		if (pApp->m_ThemeLibLoaded)
			return pApp->zIsAppThemed();

	return FALSE;
}

FMCommDlg_API void DrawControlBorder(CWnd* pWnd)
{
	CRect rect;
	pWnd->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC dc(pWnd);

	FMApplication* pApp = (FMApplication*)AfxGetApp();
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


// IATA-Datenbank
//

#include "IATA_DE.h"
#include "IATA_EN.h"

static BOOL UseGermanDB = (GetUserDefaultUILanguage() & 0x1FF)==LANG_GERMAN;

FMCommDlg_API UINT FMIATAGetCountryCount()
{
	return UseGermanDB ? CountryCount_DE : CountryCount_EN;
}

FMCommDlg_API UINT FMIATAGetAirportCount()
{
	return UseGermanDB ? AirportCount_DE : AirportCount_EN;
}

FMCommDlg_API FMCountry* FMIATAGetCountry(UINT ID)
{
	return UseGermanDB ? &Countries_DE[ID] : &Countries_EN[ID];
}

FMCommDlg_API INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer)
{
	if (Last>=(INT)FMIATAGetAirportCount()-1)
		return -1;

	*pBuffer = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	return Last;
}

FMCommDlg_API INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer)
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

FMCommDlg_API BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer)
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

FMCommDlg_API BOOL FMIsLicensed(FMLicense* License, BOOL Reload)
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

FMCommDlg_API BOOL FMIsSharewareExpired()
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

FMCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright)
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
						Version->Format(_T("%d.%d.%d"), 
							(pFinfo->dwProductVersionMS >> 16) & 0xFF,
							(pFinfo->dwProductVersionMS) & 0xFF,
							(pFinfo->dwProductVersionLS >> 16) & 0xFF);
					}
				if (Copyright)
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("© liquidFOLDERS");
			}
			delete[] lpInfo;
		}
	}
}
