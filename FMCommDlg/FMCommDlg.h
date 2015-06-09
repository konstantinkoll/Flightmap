
#pragma once
#include "CDialogMenuBar.h"
#include "CExplorerList.h"
#include "CGdiPlusBitmap.h"
#include "CGridHeader.h"
#include "CGroupBox.h"
#include "CImageListTransparent.h"
#include "CMainWindow.h"
#include "CMapSelectionCtrl.h"
#include "CMapPreviewCtrl.h"
#include "CRatingCtrl.h"
#include "CTaskBar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "CTooltipList.h"
#include "FMDynArray.h"
#include "FMApplication.h"
#include "FMColorDlg.h"
#include "FMDialog.h"
#include "FMLicenseDlg.h"
#include "FMRegisterDlg.h"
#include "FMResolutionDlg.h"
#include "FMSelectLocationGPSDlg.h"
#include "FMSelectLocationIATADlg.h"
#include "FMTooltip.h"
#include "FMUpdateDlg.h"
#include "IATA.h"
#include "License.h"

#define FMGetApp() ((FMApplication*)AfxGetApp())

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

void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
BOOL IsCtrlThemed();
void DrawControlBorder(CWnd* pWnd);
void FMErrorBox(UINT nID, HWND hWnd=NULL);


// IATA database

UINT FMIATAGetCountryCount();
UINT FMIATAGetAirportCount();
FMCountry* FMIATAGetCountry(UINT ID);
INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer);
INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer);
BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer);
HBITMAP FMIATACreateAirportMap(FMAirport* pAirport, UINT Width, UINT Height);
void FMGeoCoordinateToString(const DOUBLE c, CHAR* tmpStr, UINT cCount, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates c, CHAR* tmpStr, UINT cCount, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates c, CString& tmpStr, BOOL FillZero=FALSE);


// Lizensierung

BOOL FMIsLicensed(FMLicense* License=NULL, BOOL Reload=FALSE);
BOOL FMIsSharewareExpired();


// Update

void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
void FMCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);
