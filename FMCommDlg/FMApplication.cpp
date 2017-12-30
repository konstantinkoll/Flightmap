
// FMApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>
#include <winhttp.h>


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

	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : (osInfo.dwMajorVersion==6) ? (osInfo.dwMinorVersion==0) ? OS_Vista : (osInfo.dwMinorVersion==1) ? OS_Seven : OS_Eight : OS_Ten;

	// GdiPlus
	m_SmoothingModeAntiAlias8x8 = (OSVersion>=OS_Vista) ? (SmoothingMode)(SmoothingModeAntiAlias+1) : SmoothingModeAntiAlias;

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

	if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListExtraLarge.Attach((HIMAGELIST)il);

	// Tooltip
	p_WndTooltipOwner = NULL;
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

	if (hFontDinMittelschrift)
		RemoveFontMemResourceEx(hFontDinMittelschrift);

	for (UINT a=0; a<=MAXRATING; a++)
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
	for (UINT a=0; a<=MAXRATING; a++)
		hRatingBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_RATING0+a));

	// Eingebettete Schrift
	hFontDinMittelschrift = LoadFontFromResource(IDF_DINMITTELSCHRIFT);

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
	m_CaptionFont.CreateFont(-Size*2+1, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("DIN Mittelschrift"));
	m_UACFont.CreateFont(-Size*3/2);

	CFont* pDialogFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pDialogFont);

	pDialogFont->GetLogFont(&LogFont);
	LogFont.lfItalic = 0;
	LogFont.lfWeight = FW_NORMAL;

	m_DialogFont.CreateFontIndirect(&LogFont);

	LogFont.lfWeight = FW_BOLD;
	m_DialogBoldFont.CreateFontIndirect(&LogFont);

	// Registry
	SetRegistryKey(_T(""));

	// Zähler zurücksetzen
	m_NagCounter = 3;

	// Tooltip
	m_wndTooltip.Create();

	return TRUE;
}

CWnd* FMApplication::OpenCommandLine(LPWSTR /*pCmdLine*/)
{
	return NULL;
}

INT FMApplication::ExitInstance()
{
	m_wndTooltip.DestroyWindow();

	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		delete m_ResourceCache[a].pImage;

	GdiplusShutdown(m_GdiPlusToken);

	for (UINT a=0; a<=MAXRATING; a++)
		DeleteObject(hRatingBitmaps[a]);

	return CWinAppEx::ExitInstance();
}


// Frame handling

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


// Dialog wrapper

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
	Extensions += CString((LPCSTR)nID);
	Extensions += _T(" (*.");
	Extensions += Extension;
	Extensions += _T(")|*.");
	Extensions += Extension;
	Extensions += _T("|");

	if (Last)
		Extensions += _T("|");
}


// Registry access

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


// Resource access

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
					IStream* pStream = SHCreateMemStream((LPBYTE)pResourceData, Size);

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


// Tooltips

void FMApplication::ShowTooltip(CWnd* pWndOwner, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_wndTooltip));
	ASSERT(pWndOwner);

	(p_WndTooltipOwner=pWndOwner)->ClientToScreen(&point);
	m_wndTooltip.ShowTooltip(point, Caption, Hint, hIcon, hBitmap);
}

void FMApplication::ShowTooltip(CWnd* pWndOwner, CPoint point, FMAirport* pAirport, const CString& Hint)
{
	CString Caption(pAirport->Code);
	CString Text;
	CString tmpStr;

	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_NAME, pAirport->Name);
	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_COUNTRY, FMIATAGetCountry(pAirport->CountryID)->Name);
	FMGeoCoordinatesToString(pAirport->Location, tmpStr);
	FMTooltip::AppendAttribute(Text, IDS_AIRPORT_LOCATION, tmpStr);

	if (!Hint.IsEmpty())
		Text.Append(_T("\n")+Hint);

	ShowTooltip(pWndOwner, point, Caption, Text, NULL, FMIATACreateAirportMap(pAirport, 192, 192));
}

void FMApplication::ShowTooltip(CWnd* pWndOwner, CPoint point, LPCSTR Code, const CString& Hint)
{
	FMAirport* pAirport;
	if (FMIATAGetAirportByCode(Code, pAirport))
		ShowTooltip(pWndOwner, point, pAirport, Hint);
}

void FMApplication::HideTooltip(const CWnd* pWndOwner)
{
	if (!pWndOwner || (pWndOwner==p_WndTooltipOwner))
	{
		if (m_wndTooltip.IsWindowVisible())
			m_wndTooltip.HideTooltip();

		p_WndTooltipOwner = NULL;
	}
}

// Sounds

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


// Updates

void FMApplication::GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval)
{
	EnableAutoUpdate = GetGlobalInt(_T("EnableAutoUpdate"), 1)!=0;
	Interval = GetGlobalInt(_T("UpdateCheckInterval"), 0);
}

void FMApplication::WriteUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteGlobalInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteGlobalInt(_T("UpdateCheckInterval"), Interval);
}

BOOL FMApplication::IsUpdateCheckDue()
{
	BOOL EnableAutoUpdate;
	INT Interval;
	GetUpdateSettings(EnableAutoUpdate, Interval);

	if (EnableAutoUpdate && (Interval>=0) && (Interval<=2))
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetGlobalInt(_T("LastUpdateCheckHigh"), 0);
		LastUpdateCheck.LowPart = GetGlobalInt(_T("LastUpdateCheckLow"), 0);

		ULARGE_INTEGER Now;
		Now.HighPart = ft.dwHighDateTime;
		Now.LowPart = ft.dwLowDateTime;

#define SECOND (10000000ull)
#define MINUTE (60ull*SECOND)
#define HOUR   (60ull*MINUTE)
#define DAY    (24ull*HOUR)

		ULARGE_INTEGER NextUpdateCheck = LastUpdateCheck;
		NextUpdateCheck.QuadPart += 10ull*SECOND;

		switch (Interval)
		{
		case 0:
			NextUpdateCheck.QuadPart += DAY;
			break;

		case 1:
			NextUpdateCheck.QuadPart += 7ull*DAY;
			break;

		case 2:
			NextUpdateCheck.QuadPart += 30ull*DAY;
			break;
		}

		if ((Now.QuadPart>=NextUpdateCheck.QuadPart) || (Now.QuadPart<LastUpdateCheck.QuadPart))
			return TRUE;
	}

	return FALSE;
}

void FMApplication::WriteUpdateCheckTime()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER Now;
	Now.HighPart = ft.dwHighDateTime;
	Now.LowPart = ft.dwLowDateTime;

	WriteGlobalInt(_T("LastUpdateCheckHigh"), Now.HighPart);
	WriteGlobalInt(_T("LastUpdateCheckLow"), Now.LowPart);
}

CString FMApplication::GetLatestVersion(CString CurrentVersion)
{
	CString VersionIni;

	// License
	CurrentVersion += FMIsLicensed() ? _T(" (licensed)") : FMIsSharewareExpired() ? _T(" (expired)") : _T(" (evaluation)");

	// Get available version
	HINTERNET hSession = WinHttpOpen(_T("Flightmap/")+CurrentVersion, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, L"update.flightmap.net", INTERNET_DEFAULT_HTTP_PORT, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/version.ini", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (hRequest)
			{
				if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
					if (WinHttpReceiveResponse(hRequest, NULL))
					{
						DWORD dwSize;

						do
						{
							dwSize = 0;
							if (WinHttpQueryDataAvailable(hRequest, &dwSize))
							{
								CHAR* pBuffer = new CHAR[dwSize+1];
								DWORD dwDownloaded;

								if (WinHttpReadData(hRequest, pBuffer, dwSize, &dwDownloaded))
								{
									pBuffer[dwDownloaded] = '\0';

									VersionIni += CString(pBuffer);
								}

								delete[] pBuffer;
							}
						}
						while (dwSize>0);
					}

				WinHttpCloseHandle(hRequest);
			}

			WinHttpCloseHandle(hConnect);
		}

		WinHttpCloseHandle(hSession);
	}

	return VersionIni;
}

CString FMApplication::GetIniValue(CString Ini, const CString& Name)
{
	while (!Ini.IsEmpty())
	{
		INT Pos = Ini.Find(L"\n");
		if (Pos==-1)
			Pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, Pos-1);
		Ini.Delete(0, Pos+1);

		Pos = Line.Find(L"=");
		if (Pos!=-1)
			if (Line.Mid(0, Pos)==Name)
				return Line.Mid(Pos+1, Line.GetLength()-Pos);
	}

	return _T("");
}

void FMApplication::ParseVersion(const CString& tmpStr, FMVersion* pVersion)
{
	ASSERT(pVersion);

	swscanf_s(tmpStr, L"%u.%u.%u", &pVersion->Major, &pVersion->Minor, &pVersion->Build);
}

BOOL FMApplication::IsVersionLater(const FMVersion& LatestVersion, const FMVersion& CurrentVersion)
{
	return ((LatestVersion.Major>CurrentVersion.Major) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor>CurrentVersion.Minor)) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor==CurrentVersion.Minor) && (LatestVersion.Build>CurrentVersion.Build)));
}

BOOL FMApplication::IsUpdateFeatureLater(const CString& VersionIni, const CString& Name, FMVersion& CurrentVersion)
{
	FMVersion FeatureVersion = { 0 };
	ParseVersion(GetIniValue(VersionIni, Name), &FeatureVersion);

	return IsVersionLater(FeatureVersion, CurrentVersion);
}

DWORD FMApplication::GetUpdateFeatures(const CString& VersionIni, FMVersion& CurrentVersion)
{
	DWORD UpdateFeatures = 0;

	if (IsUpdateFeatureLater(VersionIni, _T("SecurityPatch"), CurrentVersion))
		UpdateFeatures |= UPDATE_SECUTIRYPATCH;

	if (IsUpdateFeatureLater(VersionIni, _T("ImportantBugfix"), CurrentVersion))
		UpdateFeatures |= UPDATE_IMPORTANTBUGFIX;

	if (IsUpdateFeatureLater(VersionIni, _T("NetworkAPI"), CurrentVersion))
		UpdateFeatures |= UPDATE_NETWORKAPI;

	if (IsUpdateFeatureLater(VersionIni, _T("NewFeature"), CurrentVersion))
		UpdateFeatures |= UPDATE_NEWFEATURE;

	if (IsUpdateFeatureLater(VersionIni, _T("NewVisualization"), CurrentVersion))
		UpdateFeatures |= UPDATE_NEWVISUALIZATION;

	if (IsUpdateFeatureLater(VersionIni, _T("UI"), CurrentVersion))
		UpdateFeatures |= UPDATE_UI;

	if (IsUpdateFeatureLater(VersionIni, _T("SmallBugfix"), CurrentVersion))
		UpdateFeatures |= UPDATE_SMALLBUGFIX;

	if (IsUpdateFeatureLater(VersionIni, _T("IATA"), CurrentVersion))
		UpdateFeatures |= UPDATE_IATA;

	if (IsUpdateFeatureLater(VersionIni, _T("Performance"), CurrentVersion))
		UpdateFeatures |= UPDATE_PERFORMANCE;

	return UpdateFeatures;
}

void FMApplication::CheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	// Obtain current version from instance version resource
	CString CurrentVersion;
	GetFileVersion(AfxGetResourceHandle(), CurrentVersion);

	FMVersion CV = { 0 };
	ParseVersion(CurrentVersion, &CV);

	// Check due?
	const BOOL Check = Force | IsUpdateCheckDue();

	// Perform check
	CString LatestVersion = GetGlobalString(_T("LatestUpdateVersion"));
	CString LatestMSN = GetGlobalString(_T("LatestUpdateMSN"));
	DWORD LatestUpdateFeatures = GetGlobalInt(_T("LatestUpdateFeatures"));

	if (Check)
	{
		CWaitCursor csr;
		CString VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));
			LatestUpdateFeatures = GetUpdateFeatures(VersionIni, CV);

			WriteGlobalString(_T("LatestUpdateVersion"), LatestVersion);
			WriteGlobalString(_T("LatestUpdateMSN"), LatestMSN);
			WriteGlobalInt(_T("LatestUpdateFeatures"), LatestUpdateFeatures);
			WriteUpdateCheckTime();
		}
	}

	// Update available?
	BOOL UpdateAvailable = FALSE;
	if (!LatestVersion.IsEmpty())
	{
		FMVersion LV = { 0 };
		ParseVersion(LatestVersion, &LV);

		CString IgnoreMSN = GetGlobalString(_T("IgnoreUpdateMSN"));

		UpdateAvailable = ((IgnoreMSN!=LatestMSN) || Force) && IsVersionLater(LV, CV);
	}

	// Result
	if (UpdateAvailable)
	{
		if (pParentWnd)
		{
			if (m_pUpdateNotification)
				m_pUpdateNotification->DestroyWindow();

			FMUpdateDlg dlg(LatestVersion, LatestMSN, LatestUpdateFeatures, pParentWnd);
			dlg.DoModal();
		}
		else
			if (m_pUpdateNotification)
			{
				m_pUpdateNotification->SendMessage(WM_COMMAND, IDM_UPDATE_RESTORE);
			}
			else
			{
				m_pUpdateNotification = new FMUpdateDlg(LatestVersion, LatestMSN, LatestUpdateFeatures);
				m_pUpdateNotification->Create();
				m_pUpdateNotification->ShowWindow(SW_SHOW);
			}
	}
	else
	{
		if (Force)
			FMMessageBox(pParentWnd, CString((LPCSTR)IDS_UPDATENOTAVAILABLE), CString((LPCSTR)IDS_UPDATE), MB_ICONINFORMATION | MB_OK);
	}
}
/*// Updates

void FMApplication::GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval)
{
	EnableAutoUpdate = GetInt(_T("EnableAutoUpdate"), 1)!=0;
	Interval = GetInt(_T("UpdateCheckInterval"), 0);
}

void FMApplication::WriteUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteInt(_T("UpdateCheckInterval"), Interval);
}

BOOL FMApplication::IsUpdateCheckDue()
{
	BOOL EnableAutoUpdate;
	INT Interval;
	GetUpdateSettings(EnableAutoUpdate, Interval);

	if (EnableAutoUpdate && (Interval>=0) && (Interval<=2))
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetInt(_T("LastUpdateCheckHigh"), 0);
		LastUpdateCheck.LowPart = GetInt(_T("LastUpdateCheckLow"), 0);

		ULARGE_INTEGER Now;
		Now.HighPart = ft.dwHighDateTime;
		Now.LowPart = ft.dwLowDateTime;

#define SECOND (10000000ull)
#define MINUTE (60ull*SECOND)
#define HOUR   (60ull*MINUTE)
#define DAY    (24ull*HOUR)

		ULARGE_INTEGER NextUpdateCheck = LastUpdateCheck;
		NextUpdateCheck.QuadPart += 10ull*SECOND;

		switch (Interval)
		{
		case 0:
			NextUpdateCheck.QuadPart += DAY;
			break;

		case 1:
			NextUpdateCheck.QuadPart += 7ull*DAY;
			break;

		case 2:
			NextUpdateCheck.QuadPart += 30ull*DAY;
			break;
		}

		if ((Now.QuadPart>=NextUpdateCheck.QuadPart) || (Now.QuadPart<LastUpdateCheck.QuadPart))
			return TRUE;
	}

	return FALSE;
}

void FMApplication::CheckedForUpdate()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	ULARGE_INTEGER Now;
	Now.HighPart = ft.dwHighDateTime;
	Now.LowPart = ft.dwLowDateTime;

	WriteInt(_T("LastUpdateCheckHigh"), Now.HighPart);
	WriteInt(_T("LastUpdateCheckLow"), Now.LowPart);
}*/


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
