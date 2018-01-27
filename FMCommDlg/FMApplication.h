
// FMApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "FMDynArray.h"
#include "FMFont.h"
#include "FMTooltip.h"
#include "GLRenderer.h"
#include "IATA.h"
#include "License.h"
#include <uxtheme.h>

#define RATINGBITMAPWIDTH      88
#define RATINGBITMAPHEIGHT     15
#define MAXRATING              10

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2
#define OS_Eight                3
#define OS_Ten                  4

#define NAG_EXPIRED             1
#define NAG_COUNTER             0
#define NAG_FORCE               2

typedef HRESULT(__stdcall* PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
typedef HRESULT(__stdcall* PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HTHEME(__stdcall* PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(__stdcall* PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, INT iPartId,
							INT iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT (__stdcall* PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId,
							LPCRECT prc, THEMESIZE eSize, SIZE *psz);
typedef BOOL (__stdcall* PFNISTHEMEACTIVE)();
typedef BOOL (__stdcall* PFNISAPPTHEMED)();

typedef HRESULT(__stdcall* PFNDWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT(__stdcall* PFNDWMSETWINDOWATTRIBUTE)(HWND hWnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);

typedef HRESULT(__stdcall* PFNREGISTERAPPLICATIONRESTART)(PCWSTR CommandLine, DWORD Flags);

typedef HRESULT(__stdcall* PFNGETPROPERTYSTOREFORWINDOW)(HWND hwnd, REFIID riid, void** ppv);
typedef HRESULT(__stdcall* PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)(PCWSTR AppID);

typedef HRESULT(__stdcall* PFNCHANGEWINDOWMESSAGEFILTER)(HWND hWnd, UINT message, DWORD action);

struct CDSWAKEUP
{
	GUID AppID;
	WCHAR Command[MAX_PATH];
};

struct ResourceCacheItem
{
	Bitmap* pImage;
	UINT nID;
};


// FMApplication
//

class FMUpdateDlg;

class FMApplication : public CWinAppEx
{
public:
	FMApplication(const GUID& AppID);
	virtual ~FMApplication();

	virtual BOOL InitInstance();
	virtual BOOL OpenCommandLine(LPWSTR pCmdLine=NULL);
	virtual INT ExitInstance();

	void AddFrame(CWnd* pFrame);
	void KillFrame(CWnd* pVictim);

	BOOL ShowNagScreen(UINT Level, CWnd* pWndParent=NULL);
	BOOL ChooseColor(COLORREF* pColor, CWnd* pParentWnd=NULL, BOOL AllowReset=TRUE);
	void SendMail(const CString& Subject=_T("")) const;
	static HRESULT SaveBitmap(CBitmap* pBitmap, const CString& FileName, const GUID& guidFileType, BOOL DeleteBitmap=TRUE);
	static void AddFileExtension(CString& Extensions, UINT nID, const CString& Extension, BOOL Last=FALSE);

	void GetBinary(LPCTSTR lpszEntry, LPVOID pData, UINT Size);
	INT GetGlobalInt(LPCTSTR lpszEntry, INT nDefault=0);
	CString GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=_T(""));
	BOOL WriteGlobalInt(LPCTSTR lpszEntry, INT nValue);
	BOOL WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue);

	Bitmap* GetResourceImage(UINT nID) const;
	Bitmap* GetCachedResourceImage(UINT nID);
	static HICON LoadDialogIcon(UINT nID);
	static HANDLE LoadFontFromResource(UINT nID);

	void ShowTooltip(CWnd* pWndOwner, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void ShowTooltip(CWnd* pWndOwner, CPoint point, LPCAIRPORT lpcAirport, const CString& Hint);
	void ShowTooltip(CWnd* pWndOwner, CPoint point, LPCSTR Code, const CString& Hint);
	void HideTooltip(const CWnd* pWndOwner=NULL);

	void OpenFolderAndSelectItem(LPCWSTR Path);

	static void PlayAsteriskSound();
	static void PlayDefaultSound();
	static void PlayErrorSound();
	static void PlayNavigateSound();
	static void PlayNotificationSound();
	static void PlayQuestionSound();
	static void PlayTrashSound();
	static void PlayWarningSound();

	void GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval);
	void WriteUpdateSettings(BOOL EnableAutoUpdate, INT Interval);
	void CheckForUpdate(BOOL Force=FALSE, CWnd* pParentWnd=NULL);

	CImageList m_CoreImageListJumbo;
	CImageList m_SystemImageListSmall;
	CImageList m_SystemImageListExtraLarge;
	HBITMAP hRatingBitmaps[MAXRATING+1];
	FMFont m_DefaultFont;
	FMFont m_ItalicFont;
	FMFont m_SmallFont;
	FMFont m_SmallBoldFont;
	FMFont m_LargeFont;
	FMFont m_CaptionFont;
	FMFont m_UACFont;
	FMFont m_DialogFont;
	FMFont m_DialogBoldFont;
	UINT OSVersion;
	SmoothingMode m_SmoothingModeAntiAlias8x8;
	UINT m_DistanceSettingChangedMsg;
	UINT m_TaskbarButtonCreated;
	UINT m_LicenseActivatedMsg;
	UINT m_SetProgressMsg;
	UINT m_WakeupMsg;
	GUID m_AppID;
	COLORREF m_ColorHistory[16];
	CList<CWnd*> m_pMainFrames;
	FMUpdateDlg* m_pUpdateNotification;
	GLModelQuality m_ModelQuality;
	GLTextureQuality m_TextureQuality;
	BOOL m_TextureCompress;

	PFNSETWINDOWTHEME zSetWindowTheme;
	PFNOPENTHEMEDATA zOpenThemeData;
	PFNCLOSETHEMEDATA zCloseThemeData;
	PFNDRAWTHEMEBACKGROUND zDrawThemeBackground;
	PFNGETTHEMEPARTSIZE zGetThemePartSize;
	PFNISAPPTHEMED zIsAppThemed;
	BOOL m_ThemeLibLoaded;

	PFNDWMISCOMPOSITIONENABLED zDwmIsCompositionEnabled;
	PFNDWMSETWINDOWATTRIBUTE zDwmSetWindowAttribute;
	BOOL m_DwmLibLoaded;

	PFNREGISTERAPPLICATIONRESTART zRegisterApplicationRestart;
	BOOL m_KernelLibLoaded;

	PFNGETPROPERTYSTOREFORWINDOW zGetPropertyStoreForWindow;
	PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID zSetCurrentProcessExplicitAppUserModelID;
	BOOL m_ShellLibLoaded;

	PFNCHANGEWINDOWMESSAGEFILTER zChangeWindowMessageFilter;
	BOOL m_UserLibLoaded;

	afx_msg void OnBackstagePurchase();
protected:
	afx_msg void OnBackstageEnterLicenseKey();
	afx_msg void OnBackstageSupport();
	afx_msg void OnUpdateBackstageCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	FMTooltip m_wndTooltip;
	const CWnd* p_WndTooltipOwner;
	FMDynArray<ResourceCacheItem, 16, 4> m_ResourceCache;
	UINT m_NagCounter;

private:
	static void PlayRegSound(const CString& Identifier);

	BOOL IsUpdateCheckDue();
	void WriteUpdateCheckTime();
	static CString GetLatestVersion(CString CurrentVersion);
	static CString GetIniValue(CString Ini, const CString& Name);
	static void ParseVersion(const CString& tmpStr, FMVersion* pVersion);
	static BOOL IsVersionLater(const FMVersion& LatestVersion, const FMVersion& CurrentVersion);
	static BOOL IsUpdateFeatureLater(const CString& VersionIni, const CString& Name, FMVersion& CurrentVersion);
	static DWORD GetUpdateFeatures(const CString& VersionIni, FMVersion& CurrentVersion);

	ULONG_PTR m_GdiPlusToken;
	HMODULE hModThemes;
	HMODULE hModDwm;
	HMODULE hModKernel;
	HMODULE hModShell;
	HMODULE hModUser;
	HANDLE hFontDinMittelschrift;
};

inline INT FMApplication::GetGlobalInt(LPCTSTR lpszEntry, INT nDefault)
{
	return GetInt(lpszEntry, nDefault);
}

inline CString FMApplication::GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	return GetString(lpszEntry, lpszDefault);
}

inline BOOL FMApplication::WriteGlobalInt(LPCTSTR lpszEntry, INT nValue)
{
	return WriteInt(lpszEntry, nValue);
}

inline BOOL FMApplication::WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	return WriteString(lpszEntry, lpszValue);
}
