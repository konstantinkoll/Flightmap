#pragma once
#include "..\FMCommDlg\CDialogMenuBar.h"
#include "..\FMCommDlg\CGdiPlusBitmap.h"
#include "..\FMCommDlg\CGroupBox.h"
#include "..\FMCommDlg\CMainWindow.h"
#include "..\FMCommDlg\CPictureCtrl.h"
#include "..\FMCommDlg\CStripCtrl.h"
#include "..\FMCommDlg\DynArray.h"
#include "..\FMCommDlg\FMApplication.h"
#include "..\FMCommDlg\FMDialog.h"
#include "..\FMCommDlg\FMLicenseDlg.h"
#include "..\FMCommDlg\FMProgressDlg.h"
#include "..\FMCommDlg\FMRegisterDlg.h"
#include "..\FMCommDlg\License.h"

#define FMHOVERTIME     850

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

FMCommDlg_API BOOL FMIsLicensed(FMLicense* License=NULL, BOOL Reload=FALSE);
FMCommDlg_API BOOL FMIsSharewareExpired();


// Update

FMCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
