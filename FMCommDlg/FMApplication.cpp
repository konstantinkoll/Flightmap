
// FMApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "FMCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>


// FMApplication
//

#define ResetNagCounter     m_NagCounter = 0;

BEGIN_MESSAGE_MAP(FMApplication, CWinAppEx)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
	ON_COMMAND(ID_APP_PURCHASE, OnAppPurchase)
	ON_COMMAND(ID_APP_ENTERLICENSEKEY, OnAppEnterLicenseKey)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_SUPPORT, ID_APP_ENTERLICENSEKEY, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, OnUpdateAppCommands)
END_MESSAGE_MAP()

void PlayRegSound(CString Identifier)
{
	CString strFile;
	CString strKey = _T("AppEvents\\Schemes\\");
	strKey += Identifier;
	strKey += _T("\\.current");

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
		if (reg.Read(_T(""), strFile))
			if (!strFile.IsEmpty())
				PlaySound(strFile, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
}


// FMApplication-Erstellung

FMApplication::FMApplication()
{
	// Version
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : OS_Seven;

	// Clipboard
	CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Messages
	msgUseBgImagesChanged = RegisterWindowMessageA("Flightmap.UseBgImagesChanged");
	msgDistanceSettingChanged = RegisterWindowMessageA("Flightmap.DistanceSettingChanged");

	// Custom colors
	ZeroMemory(&m_CustomColors, sizeof(m_CustomColors));

	// Themes
	hModThemes = LoadLibrary(_T("UXTHEME.DLL"));
	if (hModThemes)
	{
		zSetWindowTheme = (PFNSETWINDOWTHEME)GetProcAddress(hModThemes, "SetWindowTheme");
		zOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hModThemes, "OpenThemeData");
		zCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(hModThemes, "CloseThemeData");
		zDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(hModThemes, "DrawThemeBackground");
		zDrawThemeText = (PFNDRAWTHEMETEXT)GetProcAddress(hModThemes, "DrawThemeText");
		zDrawThemeTextEx = (PFNDRAWTHEMETEXTEX)GetProcAddress(hModThemes, "DrawThemeTextEx");
		zGetThemeSysFont = (PFNGETTHEMESYSFONT)GetProcAddress(hModThemes, "GetThemeSysFont");
		zGetThemeSysColor = (PFNGETTHEMESYSCOLOR)GetProcAddress(hModThemes, "GetThemeSysColor");
		zGetThemePartSize = (PFNGETTHEMEPARTSIZE)GetProcAddress(hModThemes, "GetThemePartSize");
		zSetWindowThemeAttribute = (PFNSETWINDOWTHEMEATTRIBUTE)GetProcAddress(hModThemes, "SetWindowThemeAttribute");
		zIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hModThemes, "IsAppThemed");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zDrawThemeText && zGetThemeSysFont && zGetThemeSysColor && zGetThemePartSize && zIsAppThemed);
		if (m_ThemeLibLoaded)
		{
			FreeLibrary(hModThemes);
			hModThemes = NULL;
		}
	}
	else
	{
		zSetWindowTheme = NULL;
		zOpenThemeData = NULL;
		zCloseThemeData = NULL;
		zDrawThemeBackground = NULL;
		zDrawThemeText = NULL;
		zDrawThemeTextEx = NULL;
		zGetThemeSysFont = NULL;
		zGetThemeSysColor = NULL;
		zGetThemePartSize = NULL;
		zSetWindowThemeAttribute = NULL;
		zIsAppThemed = NULL;

		m_ThemeLibLoaded = FALSE;
	}

	// Aero
	hModAero = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModAero)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModAero, "DwmIsCompositionEnabled");
		zDwmExtendFrameIntoClientArea = (PFNDWMEXTENDFRAMEINTOCLIENTAREA)GetProcAddress(hModAero, "DwmExtendFrameIntoClientArea");
		zDwmDefWindowProc = (PFNDWMDEFWINDOWPROC)GetProcAddress(hModAero, "DwmDefWindowProc");

		m_AeroLibLoaded = (zDwmIsCompositionEnabled && zDwmExtendFrameIntoClientArea && zDwmDefWindowProc);
		if (!m_AeroLibLoaded)
		{
			FreeLibrary(hModAero);
			hModAero = NULL;
		}
	}
	else
	{
		zDwmIsCompositionEnabled = NULL;
		zDwmExtendFrameIntoClientArea = NULL;
		zDwmDefWindowProc = NULL;

		m_AeroLibLoaded = FALSE;
	}

	// Fonts
	CString face = GetDefaultFontFace();

	INT sz = 8;
	LOGFONT lf;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
		sz = abs(lf.lfHeight);

	m_DefaultFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_BoldFont.CreateFont(-sz, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_ItalicFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 1, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_SmallFont.CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		_T("MSShellDlg"));
	m_LargeFont.CreateFont(-(sz+2), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_CaptionFont.CreateFont(-(sz+5), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

	// System image lists
	IImageList* il;
	if (SUCCEEDED(SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&il)))
		m_SystemImageListSmall.Attach((HIMAGELIST)il);
	if (SUCCEEDED(SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListLarge.Attach((HIMAGELIST)il);
	if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListExtraLarge.Attach((HIMAGELIST)il);
}

FMApplication::~FMApplication()
{
	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);
}


// FMApplication-Initialisierung

BOOL FMApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (!CWinAppEx::InitInstance())
		return FALSE;

	// OLE Initialisieren
	ENSURE(AfxOleInit());

	SetRegistryKey(_T(""));

	// Zähler zurücksetzen
	ResetNagCounter;

	return TRUE;
}

INT FMApplication::ExitInstance()
{
	GdiplusShutdown(m_gdiplusToken);

	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);

	return CWinAppEx::ExitInstance();
}

BOOL FMApplication::ShowNagScreen(UINT Level, CWnd* pWndParent)
{
	if ((Level & NAG_EXPIRED) ? FMIsSharewareExpired() : !FMIsLicensed())
		if ((Level & NAG_FORCE) || (++m_NagCounter)>5)
		{
			FMRegisterDlg dlg(pWndParent ? pWndParent : CWnd::GetForegroundWindow());
			dlg.DoModal();

			ResetNagCounter;

			return TRUE;
		}

	return FALSE;
}

BOOL FMApplication::ChooseColor(COLORREF* pColor, CWnd* pParentWnd, CString Caption)
{
	ASSERT(pColor);

	FMColorDlg dlg(*pColor, CC_RGBINIT, pParentWnd, Caption);
	if (dlg.DoModal()==IDOK)
	{
		*pColor = dlg.m_cc.rgbResult;
		return TRUE;
	}

	return FALSE;
}

CString FMApplication::GetDefaultFontFace()
{
	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);

	return lf.lfFaceName;
}

void FMApplication::SendMail(CString Subject)
{
	CString URL = _T("mailto:support@flightmap.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOW);
}


void FMApplication::OnAppSupport()
{
	SendMail();
}

void FMApplication::OnAppPurchase()
{
	CString url;
	ENSURE(url.LoadString(IDS_PURCHASEURL));

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
}

void FMApplication::OnAppEnterLicenseKey()
{
	FMLicenseDlg dlg(m_pActiveWnd);
	dlg.DoModal();
}

void FMApplication::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_APP_PURCHASE:
	case ID_APP_ENTERLICENSEKEY:
		pCmdUI->Enable(!FMIsLicensed());
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}


void FMApplication::PlayStandardSound()
{
	PlayRegSound(L"Apps\\.Default\\.Default");
}

void FMApplication::PlayNavigateSound()
{
	PlayRegSound(L"Apps\\Explorer\\Navigating");
}

void FMApplication::PlayWarningSound()
{
	PlayRegSound(L"Apps\\Explorer\\SecurityBand");
}

void FMApplication::PlayTrashSound()
{
	PlayRegSound(L"Apps\\Explorer\\EmptyRecycleBin");
}


HRESULT FMApplication::SaveBitmap(CBitmap* pBitmap, CString Filename, const GUID& guidFileType, BOOL DeleteBitmap)
{
	ASSERT(pBitmap);

	CImage img;
	img.Attach(*pBitmap);
	HRESULT res = img.Save(Filename, guidFileType);

	if (!DeleteBitmap)
		img.Detach();

	return res;
}

void FMApplication::AddFileExtension(CString& Extensions, UINT nID, CString Extension, BOOL Last)
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(nID));

	Extensions += tmpStr;
	Extensions += _T(" (*.");
	Extensions += Extension;
	Extensions += _T(")|*.");
	Extensions += Extension;
	Extensions += _T("|");
	if (Last)
		Extensions += _T("|");
}

void FMApplication::GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval)
{
	if (EnableAutoUpdate)
		*EnableAutoUpdate = GetInt(_T("EnableAutoUpdate"), 1)!=0;
	if (Interval)
		*Interval = GetInt(_T("UpdateInterval"), 0);
}

void FMApplication::SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteInt(_T("UpdateInterval"), Interval);
}

BOOL FMApplication::IsUpdateCheckDue()
{
	BOOL EnableAutoUpdate;
	INT Interval;
	GetUpdateSettings(&EnableAutoUpdate, &Interval);

	if ((EnableAutoUpdate) && (Interval>=0) && (Interval<=2))
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER LastUpdate;
		LastUpdate.HighPart = GetInt(_T("LastUpdateHigh"), 0);
		LastUpdate.LowPart = GetInt(_T("LastUpdateLow"), 0);

		ULARGE_INTEGER Now;
		Now.HighPart = ft.dwHighDateTime;
		Now.LowPart = ft.dwLowDateTime;

#define SECOND ((ULONGLONG)10000000)
#define MINUTE (60*SECOND)
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)

		switch (Interval)
		{
		case 0:
			LastUpdate.QuadPart += DAY;
			break;
		case 1:
			LastUpdate.QuadPart += 7*DAY;
			break;
		case 2:
			LastUpdate.QuadPart += 30*DAY;
			break;
		}
		LastUpdate.QuadPart += 10*SECOND;

		if (Now.QuadPart>=LastUpdate.QuadPart)
		{
			WriteInt(_T("LastUpdateHigh"), Now.HighPart);
			WriteInt(_T("LastUpdateLow"), Now.LowPart);

			return TRUE;
		}
	}

	return FALSE;
}
