
#pragma once
#include "CBackstageBar.h"
#include "CBackstageDropTarget.h"
#include "CBackstageShadow.h"
#include "CBackstageSidebar.h"
#include "CBackstageWidgets.h"
#include "CBackstageWnd.h"
#include "CCategory.h"
#include "CColorHistory.h"
#include "CColorIndicator.h"
#include "CColorPicker.h"
#include "CDesktopDimmer.h"
#include "CExplorerList.h"
#include "CFloatButtons.h"
#include "CFrontstageItemView.h"
#include "CFrontstagePane.h"
#include "CFrontstageScroller.h"
#include "CFrontstageWnd.h"
#include "CHeaderArea.h"
#include "CHeaderButton.h"
#include "CHoverButton.h"
#include "CIcons.h"
#include "CPictureCtrl.h"
#include "CRatingCtrl.h"
#include "CTaskBar.h"
#include "CTaskButton.h"
#include "CTooltipHeader.h"
#include "FMAboutDialog.h"
#include "FMDynArray.h"
#include "FMApplication.h"
#include "FMColorDlg.h"
#include "FMDialog.h"
#include "FMFont.h"
#include "FMLicenseDlg.h"
#include "FMMemorySort.h"
#include "FMMessageBoxDlg.h"
#include "FMRegisterDlg.h"
#include "FMSelectLocationGPSDlg.h"
#include "FMSelectLocationIATADlg.h"
#include "FMTabbedDialog.h"
#include "FMUpdateDlg.h"
#include "GLFont.h"
#include "GLRenderer.h"
#include "IATA.h"

#define FMGetApp() ((FMApplication*)AfxGetApp())

#define MB_ICONREADY      0x00000050L
#define MB_ICONSHIELD     0x00000060L

#define FMCATEGORYPADDING     2

#define COLORREF2RGB(clr)             (0xFF000000 | (((clr) & 0xFF)<<16) | ((clr) & 0xFF00) | ((clr)>>16))
#define COLORREF2ARGB(clr, alpha)     (((alpha)<<24) | (((clr) & 0xFF)<<16) | ((clr) & 0xFF00) | ((clr)>>16))

#define REQUEST_TOOLTIP_DATA             1
#define REQUEST_DRAWBUTTONFOREGROUND     2

struct NM_TOOLTIPDATA
{
	NMHDR hdr;
	INT Item;
	WCHAR Caption[256];
	WCHAR Hint[4096];
	HICON hIcon;
	HBITMAP hBitmap;
};

struct NM_DRAWBUTTONFOREGROUND
{
	NMHDR hdr;
	LPDRAWITEMSTRUCT lpDrawItemStruct;
	CDC* pDC;
	BOOL Hover;
	BOOL Themed;
};

struct PROGRESSDATA
{
	ULONGLONG ullCompleted;
	ULONGLONG ullTotal;
	TBPFLAG tbpFlags;
};

void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright=NULL);


// Draw

extern BLENDFUNCTION BF;

void CreateRoundRectangle(LPCRECT lpcRect, INT Radius, GraphicsPath& Path);
void CreateRoundTop(LPCRECT lpcRect, INT Radius, GraphicsPath& Path);
void CreateReflectionRectangle(LPCRECT lpcRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateMaskBitmap(LONG Width, LONG Height);
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
HBITMAP CreateTruecolorBitmap(LONG Width, LONG Height);
CBitmap* CreateTruecolorBitmapObject(LONG Width, LONG Height);
void DrawLocationIndicator(Graphics& g, INT x, INT y, INT Size=16);
void DrawLocationIndicator(CDC& dc, INT x, INT y, INT Size=16);
void DrawCategory(CDC& dc, CRect rect, LPCWSTR Caption, LPCWSTR Hint, BOOL Themed);
void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected);
void DrawSubitemBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Enabled, BOOL Selected, BOOL Hover, BOOL ClipHorizontal=FALSE);
void DrawMilledRectangle(Graphics& g, CRect rect, BOOL Backstage=TRUE, INT Radius=4);
void DrawBackstageSelection(CDC& dc, Graphics& g, const CRect& rect, BOOL Selected, BOOL Enabled, BOOL Themed);
void DrawBackstageButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Hover, BOOL Pressed, BOOL Enabled, BOOL Themed, BOOL Red=FALSE);
void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover);
void DrawWhiteButtonBorder(Graphics& g, LPCRECT lpcRect, BOOL IncludeBottom=TRUE);
void DrawWhiteButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled=FALSE, BOOL DrawBorder=FALSE);
void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL ShowKeyboardCues=FALSE);
void DrawColor(CDC& dc, CRect rect, BOOL Themed, COLORREF Color, BOOL Enabled=TRUE, BOOL Focused=FALSE, BOOL Hover=FALSE);


// IATA

UINT FMIATAGetCountryCount();
UINT FMIATAGetAirportCount();
LPCCOUNTRY FMIATAGetCountry(UINT ID);
INT FMIATAGetNextAirport(INT Last, LPCAIRPORT& lpcAirport);
INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, LPCAIRPORT& lpcAirport);
BOOL FMIATAGetAirportByCode(LPCSTR Code, LPCAIRPORT& lpcAirport);
HBITMAP FMIATACreateAirportMap(LPCAIRPORT lpcAirport, LONG Width, LONG Height);
void FMGeoCoordinateToString(const DOUBLE Coordinate, LPSTR pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates& Coordinates, LPSTR pStr, SIZE_T cCount, BOOL FillZero);
CString FMGeoCoordinateToString(const DOUBLE Coordinate, BOOL IsLatitude, BOOL FillZero);
CString FMGeoCoordinatesToString(const FMGeoCoordinates& Coordinates, BOOL FillZero=FALSE);


// Date and time

CString FMTimeToString(const FILETIME& FileTime);


// License

BOOL FMIsLicensed(FMLicense* pLicense=NULL, BOOL Reload=FALSE);
BOOL FMIsSharewareExpired();


// MessageBox

INT FMMessageBox(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type);
void FMErrorBox(CWnd* pParentWnd, UINT nID);


// Progress

inline LRESULT FMSetTaskbarProgress(CWnd* pWnd, ULONGLONG ullCompleted, ULONGLONG ullTotal, TBPFLAG tbpFlags=TBPF_NORMAL)
{
	ASSERT(pWnd);

	PROGRESSDATA pd = { ullCompleted, ullTotal, tbpFlags };

	return pWnd->GetTopLevelParent()->SendMessage(FMGetApp()->m_SetProgressMsg, (WPARAM)pWnd->GetSafeHwnd(), (LPARAM)&pd);
}

inline LRESULT FMHideTaskbarProgress(CWnd* pWnd)
{
	ASSERT(pWnd);

	return FMSetTaskbarProgress(pWnd, 0, 0, TBPF_NOPROGRESS);
}
