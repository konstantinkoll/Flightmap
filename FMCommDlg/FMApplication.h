
// FMApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "CGdiPlusBitmap.h"
#include <uxtheme.h>

#define RatingBitmapWidth        88
#define RatingBitmapHeight       15
#define MaxRating                10

#define OS_XP                     0
#define OS_Vista                  1
#define OS_Seven                  2
#define OS_Eight                  3

#define NAG_NOTLICENSED           0
#define NAG_EXPIRED               1
#define NAG_COUNTER               0
#define NAG_FORCE                 2

typedef HRESULT(__stdcall* PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
typedef HRESULT(__stdcall* PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HTHEME(__stdcall* PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, LPCWSTR pszText, INT iCharCount, DWORD dwTextFlags,
							DWORD dwTextFlags2, const RECT* pRect);
typedef HRESULT(__stdcall* PFNDRAWTHEMETEXTEX)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, LPCWSTR pszText, INT iCharCount, DWORD dwTextFlags,
							const RECT* pRect, const DTTOPTS* pOptions);
typedef HRESULT(__stdcall* PFNGETTHEMESYSFONT)(HTHEME hTheme, INT iFontID, LOGFONT* plf);
typedef HRESULT(__stdcall* PFNGETTHEMESYSCOLOR)(HTHEME hTheme, INT iColorID);
typedef HRESULT (__stdcall* PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId,
							LPRECT prc, THEMESIZE eSize, SIZE *psz);
typedef HRESULT (__stdcall* PFNSETWINDOWTHEMEATTRIBUTE)(HWND hWnd, WINDOWTHEMEATTRIBUTETYPE eAttribute,
							void* pAttribute, DWORD cdAttribute);
typedef BOOL (__stdcall* PFNISTHEMEACTIVE)();
typedef BOOL (__stdcall* PFNISAPPTHEMED)();

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const MARGINS* pMarInset);
typedef BOOL(__stdcall* PFNDWMDEFWINDOWPROC)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

struct CDS_Wakeup
{
	GUID AppID;
	WCHAR FileName[MAX_PATH];
};


// FMApplication:
// Siehe FMApplication.cpp für die Implementierung dieser Klasse
//

#define TIMESTAMP CString Timestamp = _T(__DATE__); Timestamp.Append(_T(", ")); Timestamp.Append(_T(__TIME__));

class FMApplication : public CWinAppEx
{
public:
	FMApplication(GUID& AppID);
	virtual ~FMApplication();

	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListLarge;
	CImageList m_SystemImageListExtraLarge;
	HBITMAP m_RatingBitmaps[MaxRating+1];
	CFont m_DefaultFont;
	CFont m_BoldFont;
	CFont m_ItalicFont;
	CFont m_SmallFont;
	CFont m_LargeFont;
	CFont m_CaptionFont;
	UINT OSVersion;
	BOOL m_UseBgImages;
	UINT m_WakeupMsg;
	UINT m_UseBgImagesChangedMsg;
	UINT m_DistanceSettingChangedMsg;
	GUID m_AppID;
	COLORREF m_CustomColors[16];

	PFNSETWINDOWTHEME zSetWindowTheme;
	PFNOPENTHEMEDATA zOpenThemeData;
	PFNCLOSETHEMEDATA zCloseThemeData;
	PFNDRAWTHEMEBACKGROUND zDrawThemeBackground;
	PFNDRAWTHEMETEXT zDrawThemeText;
	PFNDRAWTHEMETEXTEX zDrawThemeTextEx;
	PFNGETTHEMESYSFONT zGetThemeSysFont;
	PFNGETTHEMESYSCOLOR zGetThemeSysColor;
	PFNGETTHEMEPARTSIZE zGetThemePartSize;
	PFNSETWINDOWTHEMEATTRIBUTE zSetWindowThemeAttribute;
	PFNISAPPTHEMED zIsAppThemed;
	BOOL m_ThemeLibLoaded;

	PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled;
	PFNDWMEXTENDFRAMEINTOCLIENTAREA zDwmExtendFrameIntoClientArea;
	PFNDWMDEFWINDOWPROC zDwmDefWindowProc;
	BOOL m_AeroLibLoaded;

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	BOOL ShowNagScreen(UINT Level, CWnd* pWndParent=NULL);
	BOOL ChooseColor(COLORREF* pColor, CWnd* pParentWnd=NULL, CString Caption=_T(""));
	CString GetDefaultFontFace();
	void SendMail(CString Subject=_T(""));
	static void PlayStandardSound();
	static void PlayNavigateSound();
	static void PlayWarningSound();
	static void PlayTrashSound();
	static HRESULT SaveBitmap(CBitmap* pBitmap, CString Filename, const GUID& guidFileType, BOOL DeleteBitmap=TRUE);
	static void AddFileExtension(CString& Extensions, UINT nID, CString Extension, BOOL Last=FALSE);
	void GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval);
	void SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval);
	BOOL IsUpdateCheckDue();

protected:
	UINT m_NagCounter;

	afx_msg void OnAppPurchase();
	afx_msg void OnAppEnterLicenseKey();
	afx_msg void OnAppSupport();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;
	HMODULE hModThemes;
	HMODULE hModAero;
};
