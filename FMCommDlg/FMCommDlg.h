#pragma once
#include "..\FMCommDlg\CDialogMenuBar.h"
#include "..\FMCommDlg\CExplorerList.h"
#include "..\FMCommDlg\CGdiPlusBitmap.h"
#include "..\FMCommDlg\CGridHeader.h"
#include "..\FMCommDlg\CGroupBox.h"
#include "..\FMCommDlg\CImageListTransparent.h"
#include "..\FMCommDlg\CMainWindow.h"
#include "..\FMCommDlg\CMapSelectionCtrl.h"
#include "..\FMCommDlg\CMapPreviewCtrl.h"
#include "..\FMCommDlg\CPictureCtrl.h"
#include "..\FMCommDlg\CRatingCtrl.h"
#include "..\FMCommDlg\CTaskBar.h"
#include "..\FMCommDlg\CTaskButton.h"
#include "..\FMCommDlg\CTooltipHeader.h"
#include "..\FMCommDlg\CTooltipList.h"
#include "..\FMCommDlg\DynArray.h"
#include "..\FMCommDlg\FMApplication.h"
#include "..\FMCommDlg\FMColorDlg.h"
#include "..\FMCommDlg\FMDialog.h"
#include "..\FMCommDlg\FMLicenseDlg.h"
#include "..\FMCommDlg\FMRegisterDlg.h"
#include "..\FMCommDlg\FMResolutionDlg.h"
#include "..\FMCommDlg\FMSelectLocationGPSDlg.h"
#include "..\FMCommDlg\FMSelectLocationIATADlg.h"
#include "..\FMCommDlg\FMTooltip.h"
#include "..\FMCommDlg\IATA.h"
#include "..\FMCommDlg\License.h"

#define FMGetApp() ((FMApplication*)AfxGetApp())

void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
BOOL IsCtrlThemed();
void DrawControlBorder(CWnd* pWnd);
void FMErrorBox(UINT nResID, HWND hWnd=NULL);


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
