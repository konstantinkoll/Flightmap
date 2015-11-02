
#pragma once
#include "CDialogMenuBar.h"
#include "CExplorerList.h"
#include "CFrontstageWnd.h"
#include "CGroupBox.h"
#include "CHoverButton.h"
#include "CIcons.h"
#include "CMainWindow.h"
#include "CRatingCtrl.h"
#include "CTaskBar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "FMDynArray.h"
#include "FMApplication.h"
#include "FMColorDlg.h"
#include "FMDialog.h"
#include "FMFont.h"
#include "FMLicenseDlg.h"
#include "FMRegisterDlg.h"
#include "FMResolutionDlg.h"
#include "FMSelectLocationGPSDlg.h"
#include "FMSelectLocationIATADlg.h"
#include "FMUpdateDlg.h"
#include "IATA.h"
#include "License.h"

#define FMGetApp() ((FMApplication*)AfxGetApp())

struct FMVersion
{
	UINT Major;
	UINT Minor;
	UINT Build;
};

struct FMLicense
{
	CHAR PurchaseID[256];
	CHAR ProductID[256];
	CHAR PurchaseDate[16];			// Either DD/MM/YYYY or DD.MM.YYYY
	CHAR Quantity[8];
	CHAR RegName[256];
	FMVersion Version;
};

extern BLENDFUNCTION BF;

void CreateRoundRectangle(LPCRECT pRect, INT Radius, GraphicsPath& Path);
void CreateRelectionRectangle(LPCRECT pRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
void DrawControlBorder(CWnd* pWnd);
void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected);
void DrawSubitemBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal=FALSE);
void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover);
void FMErrorBox(UINT nID, HWND hWnd=NULL);


// IATA database

UINT FMIATAGetCountryCount();
UINT FMIATAGetAirportCount();
const FMCountry* FMIATAGetCountry(UINT ID);
INT FMIATAGetNextAirport(INT Last, FMAirport** ppAirport);
INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** ppAirport);
BOOL FMIATAGetAirportByCode(const CHAR* Code, FMAirport** ppAirport);
HBITMAP FMIATACreateAirportMap(FMAirport* pAirport, UINT Width, UINT Height);
void FMGeoCoordinateToString(const DOUBLE c, CHAR* tmpStr, UINT cCount, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinateToString(const DOUBLE c, CString& tmpStr, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates c, CHAR* tmpStr, UINT cCount, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates c, CString& tmpStr, BOOL FillZero=FALSE);


// Lizensierung

BOOL FMIsLicensed(FMLicense* pLicense=NULL, BOOL Reload=FALSE);
BOOL FMIsSharewareExpired();


// Update

void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright=NULL);
void FMCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);
