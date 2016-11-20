
// FMApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>


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
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : OS_Seven;

	// GdiPlus
	m_SmoothingModeAntiAlias8x8 = OSVersion>=OS_Vista ? (SmoothingMode)(SmoothingModeAntiAlias+1) : SmoothingModeAntiAlias;

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Messages
	m_DistanceSettingChangedMsg = RegisterWindowMessageA("Flightmap.DistanceSettingChanged");
	m_TaskbarButtonCreated = RegisterWindowMessage(_T("TaskbarButtonCreated"));
	m_LicenseActivatedMsg = RegisterWindowMessage(_T("Flightmap.LicenseActivated"));
	m_SetProgressMsg = RegisterWindowMessage(_T("Flightmap.SetProgress"));
	m_WakeupMsg = RegisterWindowMessage(_T("Flightmap.NewWindow"));

	// Color history
	for (UINT a=0; a<16; a++)
		m_ColorHistory[a] = (COLORREF)-1;

	// Themes
	hModThemes = LoadLibrary(_T("UXTHEME.DLL"));
	if (hModThemes)
	{
		zSetWindowTheme = (PFNSETWINDOWTHEME)GetProcAddress(hModThemes, "SetWindowTheme");
		zOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hModThemes, "OpenThemeData");
		zCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(hModThemes, "CloseThemeData");
		zDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(hModThemes, "DrawThemeBackground");
		zGetThemePartSize = (PFNGETTHEMEPARTSIZE)GetProcAddress(hModThemes, "GetThemePartSize");
		zIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hModThemes, "IsAppThemed");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zGetThemePartSize && zIsAppThemed);
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
		zGetThemePartSize = NULL;
		zIsAppThemed = NULL;

		m_ThemeLibLoaded = FALSE;
	}

	// DWM
	hModDwm = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModDwm)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModDwm, "DwmIsCompositionEnabled");
		zDwmSetWindowAttribute = (PFNDWMSETWINDOWATTRIBUTE)GetProcAddress(hModDwm, "DwmSetWindowAttribute");

		m_DwmLibLoaded = (zDwmIsCompositionEnabled && zDwmSetWindowAttribute);
		if (!m_DwmLibLoaded)
		{
			FreeLibrary(hModDwm);
			hModDwm = NULL;
		}
	}
	else
	{
		zDwmIsCompositionEnabled = NULL;
		zDwmSetWindowAttribute = NULL;

		m_DwmLibLoaded = FALSE;
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

	// User
	hModUser = LoadLibrary(_T("USER32.DLL"));
	if (hModUser)
	{
		zChangeWindowMessageFilter = (PFNCHANGEWINDOWMESSAGEFILTER)GetProcAddress(hModKernel, "ChangeWindowMessageFilter");

		m_UserLibLoaded = (zChangeWindowMessageFilter!=NULL);
		if (!m_UserLibLoaded)
		{
			FreeLibrary(hModUser);
			hModUser = NULL;
		}
	}
	else
	{
		zChangeWindowMessageFilter = NULL;

		m_UserLibLoaded = FALSE;
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

	if (hModUser)
		FreeLibrary(hModUser);

	if (hFontLetterGothic)
		RemoveFontMemResourceEx(hFontLetterGothic);

	for (UINT a=0; a<=MaxRating; a++)
		DeleteObject(hRatingBitmaps[a]);
}


// FMApplication-Initialisierung

BOOL FMApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_GdiPlusToken, &gdiplusStartupInput, NULL);

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

	// OLE initialisieren
	ENSURE(AfxOleInit());

	// Rating bitmaps
	for (UINT a=0; a<=MaxRating; a++)
		hRatingBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_RATING0+a));

	// Eingebettete Schrift
	hFontLetterGothic = LoadFontFromResource(IDF_LETTERGOTHIC);

	// Fonts
	INT Size = 11;
	LOGFONT LogFont;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &LogFont, 0))
		Size = max(abs(LogFont.lfHeight), 11);

	m_DefaultFont.CreateFont(-Size);
	m_ItalicFont.CreateFont(-Size, CLEARTYPE_QUALITY, FW_NORMAL, 1);
	m_SmallFont.CreateFont(-(Size*2/3+3), CLEARTYPE_QUALITY, FW_NORMAL, 0, _T("Segoe UI"));
	m_SmallBoldFont.CreateFont(-(Size*2/3+3), CLEARTYPE_QUALITY, FW_BOLD, 0, _T("Segoe UI"));
	m_LargeFont.CreateFont(-Size*7/6);
	m_CaptionFont.CreateFont(-Size*2, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("Letter Gothic"));
	m_UACFont.CreateFont(-Size*3/2);

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
	m_wndTooltip.DestroyWindow();

	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		delete m_ResourceCache[a].pImage;

	GdiplusShutdown(m_GdiPlusToken);

	for (UINT a=0; a<=MaxRating; a++)
		DeleteObject(hRatingBitmaps[a]);

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

BOOL FMApplication::ChooseColor(COLORREF* pColor, CWnd* pParentWnd, BOOL AllowReset)
{
	FMColorDlg dlg(pColor, pParentWnd, AllowReset);

	if (dlg.DoModal()==IDOK)
	{
		if (*pColor!=(COLORREF)-1)
		{
			COLORREF Colors[16];
			memcpy_s(Colors, sizeof(Colors), m_ColorHistory, sizeof(m_ColorHistory));

			m_ColorHistory[0] = *pColor;

			UINT PtrSrc = 0;
			UINT PtrDst = 1;

			while ((PtrSrc<16) && (PtrDst<16))
			{
				if (Colors[PtrSrc]!=*pColor)
					m_ColorHistory[PtrDst++] = Colors[PtrSrc];

				PtrSrc++;
			}

			while (PtrDst<16)
				m_ColorHistory[PtrDst++] = (COLORREF)-1;
		}

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
		}
	}

	return pBitmap;
}

Bitmap* FMApplication::GetCachedResourceImage(UINT nID)
{
	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		if (m_ResourceCache[a].nID==nID)
			return m_ResourceCache[a].pImage;

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

	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_NAME, pAirport->Name);
	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
	FMGeoCoordinatesToString(pAirport->Location, tmpStr);
	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_LOCATION, tmpStr);

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


void FMApplication::PlayRegSound(const CString& Identifier)
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
	ON_COMMAND(IDM_BACKSTAGE_PURCHASE, OnBackstagePurchase)
	ON_COMMAND(IDM_BACKSTAGE_ENTERLICENSEKEY, OnBackstageEnterLicenseKey)
	ON_COMMAND(IDM_BACKSTAGE_SUPPORT, OnBackstageSupport)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_BACKSTAGE_PURCHASE, IDM_BACKSTAGE_ABOUT, OnUpdateBackstageCommands)
END_MESSAGE_MAP()

void FMApplication::OnBackstagePurchase()
{
	CString URL((LPCSTR)IDS_PURCHASEURL);

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
}

void FMApplication::OnBackstageEnterLicenseKey()
{
	FMLicenseDlg dlg(m_pActiveWnd);
	dlg.DoModal();
}

void FMApplication::OnBackstageSupport()
{
	SendMail();
}

void FMApplication::OnUpdateBackstageCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_BACKSTAGE_PURCHASE:
	case IDM_BACKSTAGE_ENTERLICENSEKEY:
		pCmdUI->Enable(!FMIsLicensed());
		break;

	default:
		pCmdUI->Enable(TRUE);
	}
}
