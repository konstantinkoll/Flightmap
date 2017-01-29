
// FMApplication.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "resource.h"
#include "FMDynArray.h"
#include "FMFont.h"
#include "FMTooltip.h"
#include "GLRenderer.h"
#include "IATA.h"
#include <uxtheme.h>

#define RatingBitmapWidth      88
#define RatingBitmapHeight     15
#define MaxRating              10

#define OS_XP                   0
#define OS_Vista                1
#define OS_Seven                2

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

typedef HRESULT(__stdcall* PFNCHANGEWINDOWMESSAGEFILTER)(HWND hWnd, UINT message, DWORD action);

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
	BOOL ChooseColor(COLORREF* pColor, CWnd* pParentWnd=NULL, BOOL AllowReset=TRUE);
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
	HBITMAP hRatingBitmaps[MaxRating+1];
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

	PFNCHANGEWINDOWMESSAGEFILTER zChangeWindowMessageFilter;
	BOOL m_UserLibLoaded;

	afx_msg void OnBackstagePurchase();
protected:
	afx_msg void OnBackstageEnterLicenseKey();
	afx_msg void OnBackstageSupport();
	afx_msg void OnUpdateBackstageCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	FMTooltip m_wndTooltip;
	FMDynArray<ResourceCacheItem, 16, 4> m_ResourceCache;
	UINT m_NagCounter;

private:
	static void PlayRegSound(const CString& Identifier);

	ULONG_PTR m_GdiPlusToken;
	HMODULE hModThemes;
	HMODULE hModDwm;
	HMODULE hModShell;
	HMODULE hModKernel;
	HMODULE hModUser;
	HANDLE hFontLetterGothic;
};

inline BOOL FMApplication::IsTooltipVisible() const
{
	ASSERT(IsWindow(m_wndTooltip));

	return m_wndTooltip.IsWindowVisible();
}

inline void FMApplication::HideTooltip()
{
	ASSERT(IsWindow(m_wndTooltip));

	m_wndTooltip.HideTooltip();
}
