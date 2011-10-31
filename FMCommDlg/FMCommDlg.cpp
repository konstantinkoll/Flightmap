
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
	Integer n("745495196278906388636775394847083621342948184497620571684486911233963026358348142924980767925246631723125776567861840016140759057887626699111750486589518844265093743018380979709327527515518976285922706516923147828076538170972730183425557516081175385650534524185881211094278086683594714172177608841889993609400198766281044688600596754569489192345101979343468669802344086907480228591789172201629911453850773648840583343122891763767764228796196156401170554177938285696830486894331437834556251102634591813052294727051913850611273897873094152049052538993868633785883333899830540017013351013051436649700047349078185669990895492280131774298910733408039488338775031855217004993409862255738766029617966149166800537682141977654630013676816397200968712319762658485930029154225302095517962261669873874532952773591788024202484800434032440378140651213784614438189252406134607226451954778487476382220064125800227678929995859762762265522856822435862521744384622820138233752235289143337592718212618381294424731866372596352871531111041688119666919042905495747876323829528637851924273124345938360066547750112529335899447558317824780247359979724026700097382563761302560657179092084838455014801002071816886727980707589178515801870998113718231400298837471.");
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
	return true;
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
