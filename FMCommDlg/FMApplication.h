
// FMApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "FMDynArray.h"
#include "FMFont.h"
#include "FMTooltip.h"
#include "IATA.h"
#include <uxtheme.h>

#define RatingBitmapWidth      88
#define RatingBitmapHeight     15
#define MaxRating              10

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2
#define OS_Eight                3

#define NAG_EXPIRED             1
#define NAG_COUNTER             0
#define NAG_FORCE               2

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
							LPCRECT prc, THEMESIZE eSize, SIZE *psz);
typedef HRESULT (__stdcall* PFNSETWINDOWTHEMEATTRIBUTE)(HWND hWnd, WINDOWTHEMEATTRIBUTETYPE eAttribute,
							void* pAttribute, DWORD cdAttribute);
typedef BOOL (__stdcall* PFNISTHEMEACTIVE)();
typedef BOOL (__stdcall* PFNISAPPTHEMED)();

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const MARGINS* pMarInset);
typedef BOOL(__stdcall* PFNDWMDEFWINDOWPROC)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

typedef HRESULT(__stdcall* PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)(PCWSTR AppID);

typedef HRESULT(__stdcall* PFNREGISTERAPPLICATIONRESTART)(PCWSTR CommandLine, DWORD Flags);

struct CDS_Wakeup
{
	GUID AppID;
	WCHAR Command[MAX_PATH];
};

struct ResourceCacheItem
{
	Bitmap* pImage;
	UINT nID;
};


// FMApplication:
// Siehe FMApplication.cpp für die Implementierung dieser Klasse
//

class FMUpdateDlg;

class FMApplication : public CWinAppEx
{
public:
	FMApplication(GUID& AppID);
	virtual ~FMApplication();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);
	BOOL ShowNagScreen(UINT Level, CWnd* pWndParent=NULL);
	BOOL ChooseColor(COLORREF* pColor, CWnd* pParentWnd=NULL, const CString& Caption=_T("")) const;
	void SendMail(const CString& Subject=_T("")) const;
	Bitmap* GetResourceImage(UINT nID) const;
	Bitmap* GetCachedResourceImage(UINT nID);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, FMAirport* pAirport, const CString& Hint);
	void ShowTooltip(CWnd* pCallerWnd, CPoint point, const CHAR* Code, const CString& Hint);
	BOOL IsTooltipVisible() const;
	void HideTooltip();
	static void PlayAsteriskSound();
	static void PlayDefaultSound();
	static void PlayErrorSound();
	static void PlayNavigateSound();
	static void PlayNotificationSound();
	static void PlayQuestionSound();
	static void PlayTrashSound();
	static void PlayWarningSound();
	static HRESULT SaveBitmap(CBitmap* pBitmap, const CString& FileName, const GUID& guidFileType, BOOL DeleteBitmap=TRUE);
	static void AddFileExtension(CString& Extensions, UINT nID, const CString& Extension, BOOL Last=FALSE);
	void GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval);
	void SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval);
	BOOL IsUpdateCheckDue();
	void GetBinary(LPCTSTR lpszEntry, void* pData, UINT Size);

	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListLarge;
	CImageList m_SystemImageListExtraLarge;
	HBITMAP m_RatingBitmaps[MaxRating+1];
	FMFont m_DefaultFont;
	FMFont m_ItalicFont;
	FMFont m_SmallFont;
	FMFont m_SmallBoldFont;
	FMFont m_LargeFont;
	FMFont m_CaptionFont;
	FMFont m_DialogFont;
	UINT OSVersion;
	BOOL m_UseBgImages;
	UINT m_LicenseActivatedMsg;
	UINT m_WakeupMsg;
	UINT m_UseBgImagesChangedMsg;
	UINT m_DistanceSettingChangedMsg;
	UINT m_TaskbarButtonCreated;
	GUID m_AppID;
	COLORREF m_CustomColors[16];
	CList<CWnd*> m_pMainFrames;
	FMUpdateDlg* m_pUpdateNotification;

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
	BOOL m_DwmLibLoaded;

	PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID zSetCurrentProcessExplicitAppUserModelID;
	BOOL m_ShellLibLoaded;

	PFNREGISTERAPPLICATIONRESTART zRegisterApplicationRestart;
	BOOL m_KernelLibLoaded;

protected:
	afx_msg void OnAppPurchase();
	afx_msg void OnAppEnterLicenseKey();
	afx_msg void OnAppSupport();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	FMTooltip m_wndTooltip;
	FMDynArray<ResourceCacheItem, 16, 4> m_ResourceCache;
	UINT m_NagCounter;

private:
	ULONG_PTR m_gdiplusToken;
	HMODULE hModThemes;
	HMODULE hModDwm;
	HMODULE hModShell;
	HMODULE hModKernel;
	HANDLE hFontLetterGothic;
};
