#pragma once
#include "..\FMCommDlg\CGdiPlusBitmap.h"
#include "..\FMCommDlg\CGlasWindow.h"
#include "..\FMCommDlg\CGroupBox.h"
#include "..\FMCommDlg\CPictureCtrl.h"
#include "..\FMCommDlg\FMApplication.h"
#include "..\FMCommDlg\FMDialog.h"
#include "..\FMCommDlg\FMLicenseDlg.h"
#include "..\FMCommDlg\FMRegisterDlg.h"
#include "..\FMCommDlg\License.h"


#ifdef FMCommDlg_EXPORTS
#define FMCommDlg_API
#else
#define FMCommDlg_API
#endif


FMCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
FMCommDlg_API BOOL IsCtrlThemed();
FMCommDlg_API void DrawControlBorder(CWnd* pWnd);


// IATA database

struct FMCountry
{
	UINT ID;
	CHAR Name[64];
};

struct FMGeoCoordinates
{
	DOUBLE Latitude;
	DOUBLE Longitude;
};

struct FMAirport
{
	INT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[64];
	FMGeoCoordinates Location;
};


FMCommDlg_API UINT FMIATAGetCountryCount();
FMCommDlg_API UINT FMIATAGetAirportCount();
FMCommDlg_API FMCountry* FMIATAGetCountry(UINT ID);
FMCommDlg_API INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer);
FMCommDlg_API INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer);
FMCommDlg_API BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer);


// Lizensierung
/*
struct FMLicenseVersion
{
	UINT Major;
	UINT Minor;
	UINT Release;
};

struct FMLicense
{
	WCHAR PurchaseID[256];
	WCHAR ProductID[256];
	WCHAR PurchaseDate[16];			// Either DD/MM/YYYY or DD.MM.YYYY
	WCHAR Quantity[8];
	WCHAR RegName[256];
	FMLicenseVersion Version;
};
*/
FMCommDlg_API BOOL FMIsLicensed(FMLicense* License=NULL, BOOL Reload=FALSE);
FMCommDlg_API BOOL FMIsSharewareExpired();


// Update

FMCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
