#pragma once
#include "..\FMCommDlg\CDialogMenuBar.h"
#include "..\FMCommDlg\CExplorerList.h"
#include "..\FMCommDlg\CGdiPlusBitmap.h"
#include "..\FMCommDlg\CGroupBox.h"
#include "..\FMCommDlg\CMainWindow.h"
#include "..\FMCommDlg\CMapSelectionCtrl.h"
#include "..\FMCommDlg\CMapPreviewCtrl.h"
#include "..\FMCommDlg\CPictureCtrl.h"
#include "..\FMCommDlg\CStripCtrl.h"
#include "..\FMCommDlg\DynArray.h"
#include "..\FMCommDlg\FMApplication.h"
#include "..\FMCommDlg\FMColorDlg.h"
#include "..\FMCommDlg\FMDialog.h"
#include "..\FMCommDlg\FMLicenseDlg.h"
#include "..\FMCommDlg\FMProgressDlg.h"
#include "..\FMCommDlg\FMRegisterDlg.h"
#include "..\FMCommDlg\FMResolutionDlg.h"
#include "..\FMCommDlg\FMSelectLocationGPSDlg.h"
#include "..\FMCommDlg\FMSelectLocationIATADlg.h"
#include "..\FMCommDlg\FMTooltip.h"
#include "..\FMCommDlg\IATA.h"
#include "..\FMCommDlg\License.h"

#ifdef FMCommDlg_EXPORTS
#define FMCommDlg_API
#else
#define FMCommDlg_API
#endif

FMCommDlg_API void CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path);
FMCommDlg_API BOOL IsCtrlThemed();
FMCommDlg_API void DrawControlBorder(CWnd* pWnd);
FMCommDlg_API void FMErrorBox(UINT nResID, HWND hWnd=NULL);


// IATA database

FMCommDlg_API UINT FMIATAGetCountryCount();
FMCommDlg_API UINT FMIATAGetAirportCount();
FMCommDlg_API FMCountry* FMIATAGetCountry(UINT ID);
FMCommDlg_API INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer);
FMCommDlg_API INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer);
FMCommDlg_API BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer);
FMCommDlg_API HBITMAP FMIATACreateAirportMap(FMAirport* pAirport, UINT Width, UINT Height);
FMCommDlg_API void FMGeoCoordinateToString(const DOUBLE c, CHAR* tmpStr, UINT cCount, BOOL IsLatitude, BOOL FillZero);
FMCommDlg_API void FMGeoCoordinatesToString(const FMGeoCoordinates c, CHAR* tmpStr, UINT cCount, BOOL FillZero);
FMCommDlg_API void FMGeoCoordinatesToString(const FMGeoCoordinates c, CString& tmpStr, BOOL FillZero=FALSE);


// Lizensierung

FMCommDlg_API BOOL FMIsLicensed(FMLicense* License=NULL, BOOL Reload=FALSE);
FMCommDlg_API BOOL FMIsSharewareExpired();


// Update

FMCommDlg_API void GetFileVersion(HMODULE hModule, CString* Version, CString* Copyright=NULL);
FMCommDlg_API void FMCheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);
