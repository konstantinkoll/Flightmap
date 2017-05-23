
#pragma once
#include "CBackstageBar.h"
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
#include "CFrontstagePane.h"
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
#include "FMDynArray.h"
#include "FMApplication.h"
#include "FMColorDlg.h"
#include "FMDialog.h"
#include "FMFont.h"
#include "FMLicenseDlg.h"
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

#define REQUEST_TEXTCOLOR                1
#define REQUEST_TOOLTIP_DATA             2
#define REQUEST_DRAWBUTTONFOREGROUND     3

#define COLORREF2RGB(clr)             (0xFF000000 | ((clr & 0xFF)<<16) | (clr & 0xFF00) | (clr>>16))
#define COLORREF2ARGB(clr, alpha)     ((alpha<<24) | ((clr & 0xFF)<<16) | (clr & 0xFF00) | (clr>>16))

struct NM_TEXTCOLOR
{
	NMHDR hdr;
	INT Item;
	COLORREF Color;
};

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

void CreateRoundRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateRoundTop(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
void CreateReflectionRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path);
BOOL IsCtrlThemed();
HBITMAP CreateTransparentBitmap(LONG Width, LONG Height);
HBITMAP CreateTruecolorBitmap(LONG Width, LONG Height);
CBitmap* CreateTruecolorBitmapObject(LONG Width, LONG Height);
void DrawLocationIndicator(Graphics& g, INT x, INT y, INT Size=16);
void DrawControlBorder(CWnd* pWnd);
void DrawCategory(CDC& dc, CRect rect, LPCWSTR Caption, LPCWSTR Hint, BOOL Themed);
void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected);
void DrawSubitemBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal=FALSE);
void DrawBackstageBorder(Graphics& g, CRect rect);
void DrawBackstageSelection(CDC& dc, Graphics& g, const CRect& rect, BOOL Selected, BOOL Enabled, BOOL Themed);
void DrawBackstageButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Hover, BOOL Pressed, BOOL Enabled, BOOL Themed, BOOL Red=FALSE);
void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover);
void DrawWhiteButtonBorder(Graphics& g, LPCRECT lpRect, BOOL IncludeBottom=TRUE);
void DrawWhiteButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled=FALSE, BOOL DrawBorder=FALSE);
void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL ShowKeyboardCues=FALSE);
void DrawColor(CDC& dc, CRect rect, BOOL Themed, COLORREF Color, BOOL Enabled=TRUE, BOOL Focused=FALSE, BOOL Hover=FALSE);


// IATA

UINT FMIATAGetCountryCount();
UINT FMIATAGetAirportCount();
const FMCountry* FMIATAGetCountry(UINT ID);
INT FMIATAGetNextAirport(INT Last, FMAirport** ppAirport);
INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** ppAirport);
BOOL FMIATAGetAirportByCode(LPCSTR Code, FMAirport** ppAirport);
HBITMAP FMIATACreateAirportMap(FMAirport* pAirport, UINT Width, UINT Height);
void FMGeoCoordinateToString(const DOUBLE c, LPSTR tmpStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinateToString(const DOUBLE c, CString& tmpStr, BOOL IsLatitude, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates& c, LPSTR tmpStr, SIZE_T cCount, BOOL FillZero);
void FMGeoCoordinatesToString(const FMGeoCoordinates& c, CString& tmpStr, BOOL FillZero=FALSE);


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
