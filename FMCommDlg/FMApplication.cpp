
// FMApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>


void AppendAttribute(CString& dst, UINT ResID, const CString& Value)
{
	if (!Value.IsEmpty())
	{
		CString Name((LPCSTR)ResID);

		dst.Append(Name);
		dst.Append(_T(": "));
		dst.Append(Value);
		dst.Append(_T("\n"));
	}
}

void AppendAttribute(CString& dst, UINT ResID, LPCSTR pValue)
{
	AppendAttribute(dst, ResID, CString(pValue));
}

void PlayRegSound(CString Identifier)
{
	CString strKey;
	strKey.Format(_T("AppEvents\\Schemes\\%s\\.Current"), Identifier);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
	{
		CString strFile;

		if (reg.Read(_T(""), strFile))
			if (!strFile.IsEmpty())
				PlaySound(strFile, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
	}
}


// FMApplication
//

FMApplication::FMApplication(GUID& AppID)
{
	// ID
	m_AppID = AppID;

	// Update notification
	m_pUpdateNotification = NULL;

	// Version
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==1)) ? OS_Seven : OS_Eight;

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Messages
	m_LicenseActivatedMsg = RegisterWindowMessage(_T("Flightmap.LicenseActivated"));
	m_WakeupMsg = RegisterWindowMessage(_T("Flightmap.NewWindow"));
	m_UseBgImagesChangedMsg = RegisterWindowMessageA("Flightmap.UseBgImagesChanged");
	m_DistanceSettingChangedMsg = RegisterWindowMessageA("Flightmap.DistanceSettingChanged");
	m_TaskbarButtonCreated = RegisterWindowMessageA("TaskbarButtonCreated");

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
	hModDwm = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModDwm)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModDwm, "DwmIsCompositionEnabled");
		zDwmExtendFrameIntoClientArea = (PFNDWMEXTENDFRAMEINTOCLIENTAREA)GetProcAddress(hModDwm, "DwmExtendFrameIntoClientArea");
		zDwmDefWindowProc = (PFNDWMDEFWINDOWPROC)GetProcAddress(hModDwm, "DwmDefWindowProc");

		m_DwmLibLoaded = (zDwmIsCompositionEnabled && zDwmExtendFrameIntoClientArea && zDwmDefWindowProc);
		if (!m_DwmLibLoaded)
		{
			FreeLibrary(hModDwm);
			hModDwm = NULL;
		}
	}
	else
	{
		zDwmIsCompositionEnabled = NULL;
		zDwmExtendFrameIntoClientArea = NULL;
		zDwmDefWindowProc = NULL;

		m_DwmLibLoaded = FALSE;
	}

	// Shell
	hModShell = LoadLibrary(_T("SHELL32.DLL"));
	if (hModShell)
	{
		zSetCurrentProcessExplicitAppUserModelID = (PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)GetProcAddress(hModShell, "SetCurrentProcessExplicitAppUserModelID");

		m_ShellLibLoaded = (zSetCurrentProcessExplicitAppUserModelID!=NULL);
		if (!m_ShellLibLoaded)
		{
			FreeLibrary(hModShell);
			hModShell = NULL;
		}
	}
	else
	{
		zSetCurrentProcessExplicitAppUserModelID = NULL;

		m_ShellLibLoaded = FALSE;
	}

	// Kernel
	hModKernel = LoadLibrary(_T("KERNEL32.DLL"));
	if (hModKernel)
	{
		zRegisterApplicationRestart = (PFNREGISTERAPPLICATIONRESTART)GetProcAddress(hModKernel, "RegisterApplicationRestart");

		m_KernelLibLoaded = (zRegisterApplicationRestart!=NULL);
		if (!m_KernelLibLoaded)
		{
			FreeLibrary(hModKernel);
			hModKernel = NULL;
		}
	}
	else
	{
		zRegisterApplicationRestart = NULL;

		m_KernelLibLoaded = FALSE;
	}

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

	if (hModDwm)
		FreeLibrary(hModDwm);

	if (hModShell)
		FreeLibrary(hModShell);

	if (hModKernel)
		FreeLibrary(hModKernel);

	if (hFontLetterGothic)
		RemoveFontMemResourceEx(hFontLetterGothic);
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

	// Dialog classes
	WNDCLASS wc;
	GetClassInfo(AfxGetInstanceHandle(), _T("#32770"), &wc);
	wc.lpszClassName = _T("UpdateDlg");
	AfxRegisterClass(&wc);

	// Rating bitmaps
	for (UINT a=0; a<=MaxRating; a++)
		m_RatingBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_RATING0+a));

	// Eingebettete Schrift
	hFontLetterGothic = LoadFontFromResource(IDF_LETTERGOTHIC);

	// Fonts
	INT Size = 11;
	LOGFONT LogFont;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &LogFont, 0))
		Size = max(abs(LogFont.lfHeight), 11);

	afxGlobalData.fontTooltip.GetLogFont(&LogFont);

	m_DefaultFont.CreateFont(-Size);
	m_ItalicFont.CreateFont(-Size, CLEARTYPE_QUALITY, FW_NORMAL, 1);
	m_SmallFont.CreateFont(-(Size*5/6+1), CLEARTYPE_QUALITY, FW_NORMAL, 0, _T("Segoe UI"));
	m_SmallBoldFont.CreateFont(-(Size*5/6+1), CLEARTYPE_QUALITY, FW_BOLD, 0, _T("Segoe UI"));
	m_LargeFont.CreateFont(-Size*7/6);
	m_CaptionFont.CreateFont(-Size*2, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("Letter Gothic"));

	CFont* pDialogFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pDialogFont);

	pDialogFont->GetLogFont(&LogFont);
	LogFont.lfItalic = 0;
	LogFont.lfWeight = FW_NORMAL;

	m_DialogFont.CreateFontIndirect(&LogFont);

	// Registry
	SetRegistryKey(_T(""));

	// Zähler zurücksetzen
	m_NagCounter = 3;

	// Tooltip
	m_wndTooltip.Create();

	return TRUE;
}

CWnd* FMApplication::OpenCommandLine(WCHAR* /*CmdLine*/)
{
	return NULL;
}

INT FMApplication::ExitInstance()
{
	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		delete m_ResourceCache.m_Items[a].pImage;

	for (UINT a=0; a<=MaxRating; a++)
		DeleteObject(m_RatingBitmaps[a]);

	GdiplusShutdown(m_gdiplusToken);

	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModDwm)
		FreeLibrary(hModDwm);

	return CWinAppEx::ExitInstance();
}

void FMApplication::AddFrame(CWnd* pFrame)
{
	m_pMainFrames.AddTail(pFrame);
	m_pMainWnd = pFrame;
	m_pActiveWnd = NULL;
}

void FMApplication::KillFrame(CWnd* pVictim)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CWnd* pFrame = m_pMainFrames.GetNext(p);
		if (pFrame==pVictim)
		{
			m_pMainFrames.RemoveAt(pl);
		}
		else
		{
			m_pMainWnd = pFrame;
		}
	}
}

BOOL FMApplication::ShowNagScreen(UINT Level, CWnd* pWndParent)
{
	if ((Level & NAG_EXPIRED) ? FMIsSharewareExpired() : !FMIsLicensed())
		if ((Level & NAG_FORCE) || (++m_NagCounter)>=5)
		{
			FMRegisterDlg dlg(pWndParent ? pWndParent : CWnd::GetForegroundWindow());
			dlg.DoModal();

			m_NagCounter = 0;

			return TRUE;
		}

	return FALSE;
}

BOOL FMApplication::ChooseColor(COLORREF* pColor, CWnd* pParentWnd, const CString& Caption) const
{
	ASSERT(pColor);

	FMColorDlg dlg(pParentWnd, *pColor, CC_RGBINIT, Caption);
	if (dlg.DoModal()==IDOK)
	{
		*pColor = dlg.m_cc.rgbResult;
		return TRUE;
	}

	return FALSE;
}

void FMApplication::SendMail(const CString& Subject) const
{
	CString URL = _T("mailto:support@flightmap.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
}

Bitmap* FMApplication::GetResourceImage(UINT nID) const
{
	Bitmap* pBitmap = NULL;

	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPVOID pResourceData = LockResource(hMemory);
			if (pResourceData)
			{
				DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
				if (Size)
				{
					IStream* pStream = SHCreateMemStream((BYTE*)pResourceData, Size);

					pBitmap = Gdiplus::Bitmap::FromStream(pStream);

					pStream->Release();
				}
			}

			UnlockResource(hMemory);
		}
	}

	return pBitmap;
}

Bitmap* FMApplication::GetCachedResourceImage(UINT nID)
{
	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		if (m_ResourceCache.m_Items[a].nID==nID)
			return m_ResourceCache.m_Items[a].pImage;

	Bitmap* pBitmap = GetResourceImage(nID);;
	if (pBitmap)
	{
		ResourceCacheItem Item;
		Item.pImage = pBitmap;
		Item.nID = nID;

		m_ResourceCache.AddItem(Item);
	}

	return pBitmap;
}

HICON FMApplication::LoadDialogIcon(UINT nID)
{
	return (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
}

HANDLE FMApplication::LoadFontFromResource(UINT nID)
{
	HANDLE hFont = NULL;

	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPVOID pResourceData = LockResource(hMemory);
			if (pResourceData)
			{
				DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
				if (Size)
				{
					DWORD nFonts;
					hFont = AddFontMemResourceEx(pResourceData, Size, NULL, &nFonts);
				}
			}

			UnlockResource(hMemory);
		}
	}

	return hFont;
}


void FMApplication::ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_wndTooltip));
	ASSERT(pCallerWnd);

	pCallerWnd->ClientToScreen(&point);
	m_wndTooltip.ShowTooltip(point, Caption, Hint, hIcon, hBitmap);
}

void FMApplication::ShowTooltip(CWnd* pCallerWnd, CPoint point, FMAirport* pAirport, const CString& Hint)
{
	CString Caption(pAirport->Code);
	CString Text(_T(""));
	CString tmpStr;

	AppendAttribute(Text, IDS_AIRPORT_NAME, pAirport->Name);
	AppendAttribute(Text, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
	FMGeoCoordinatesToString(pAirport->Location, tmpStr);
	AppendAttribute(Text, IDS_AIRPORT_LOCATION, tmpStr);

	if (!Hint.IsEmpty())
		Text.Append(Hint);

	ShowTooltip(pCallerWnd, point, Caption, Text, NULL, FMIATACreateAirportMap(pAirport, 192, 192));
}

void FMApplication::ShowTooltip(CWnd* pCallerWnd, CPoint point, const CHAR* Code, const CString& Hint)
{
	FMAirport* pAirport = NULL;
	if (FMIATAGetAirportByCode(Code, &pAirport))
		ShowTooltip(pCallerWnd, point, pAirport, Hint);
}

BOOL FMApplication::IsTooltipVisible() const
{
	ASSERT(IsWindow(m_wndTooltip));

	return m_wndTooltip.IsWindowVisible();
}

void FMApplication::HideTooltip()
{
	ASSERT(IsWindow(m_wndTooltip));

	m_wndTooltip.HideTooltip();
}


void FMApplication::PlayAsteriskSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemAsterisk"));
}

void FMApplication::PlayDefaultSound()
{
	PlayRegSound(_T("Apps\\.Default\\.Default"));
}

void FMApplication::PlayErrorSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemHand"));
}

void FMApplication::PlayNavigateSound()
{
	PlayRegSound(_T("Apps\\Explorer\\Navigating"));
}

void FMApplication::PlayNotificationSound()
{
	PlayRegSound(_T("Apps\\Explorer\\SecurityBand"));
}

void FMApplication::PlayQuestionSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemQuestion"));
}

void FMApplication::PlayTrashSound()
{
	PlayRegSound(_T("Apps\\Explorer\\EmptyRecycleBin"));
}

void FMApplication::PlayWarningSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemExclamation"));
}


HRESULT FMApplication::SaveBitmap(CBitmap* pBitmap, const CString& FileName, const GUID& guidFileType, BOOL DeleteBitmap)
{
	ASSERT(pBitmap);

	CImage img;
	img.Attach(*pBitmap);
	HRESULT Result = img.Save(FileName, guidFileType);

	if (!DeleteBitmap)
		img.Detach();

	return Result;
}

void FMApplication::AddFileExtension(CString& Extensions, UINT nID, const CString& Extension, BOOL Last)
{
	CString tmpStr((LPCSTR)nID);

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
		*Interval = GetInt(_T("UpdateCheckInterval"), 0);
}

void FMApplication::SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteInt(_T("UpdateCheckInterval"), Interval);
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

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetInt(_T("LastUpdateCheckHigh"), 0);
		LastUpdateCheck.LowPart = GetInt(_T("LastUpdateCheckLow"), 0);

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
			LastUpdateCheck.QuadPart += DAY;
			break;

		case 1:
			LastUpdateCheck.QuadPart += 7*DAY;
			break;

		case 2:
			LastUpdateCheck.QuadPart += 30*DAY;
			break;
		}
		LastUpdateCheck.QuadPart += 10*SECOND;

		if (Now.QuadPart>=LastUpdateCheck.QuadPart)
		{
			WriteInt(_T("LastUpdateCheckHigh"), Now.HighPart);
			WriteInt(_T("LastUpdateCheckLow"), Now.LowPart);

			return TRUE;
		}
	}

	return FALSE;
}

void FMApplication::GetBinary(LPCTSTR lpszEntry, void* pData, UINT Size)
{
	UINT Bytes;
	LPBYTE pBuffer = NULL;
	CWinAppEx::GetBinary(lpszEntry, &pBuffer, &Bytes);

	if (pBuffer)
	{
		memcpy_s(pData, Size, pBuffer, min(Size, Bytes));
		free(pBuffer);
	}
}


BEGIN_MESSAGE_MAP(FMApplication, CWinAppEx)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
	ON_COMMAND(ID_APP_PURCHASE, OnAppPurchase)
	ON_COMMAND(ID_APP_ENTERLICENSEKEY, OnAppEnterLicenseKey)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_SUPPORT, ID_APP_ENTERLICENSEKEY, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, OnUpdateAppCommands)
END_MESSAGE_MAP()

void FMApplication::OnAppSupport()
{
	SendMail();
}

void FMApplication::OnAppPurchase()
{
	CString URL((LPCSTR)IDS_PURCHASEURL);

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
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
