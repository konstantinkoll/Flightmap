
// CItinerary.cpp: Implementierung der Klasse CItinerary
//

#include "stdafx.h"
#include "CItinerary.h"
#include "Flightmap.h"
#include <math.h>
#include <winioctl.h>


// Helpers
//

void PrepareEditCtrl(CMFCMaskedEdit* pMaskedEdit, UINT Attr, AIRX_Flight* pFlight)
{
	ASSERT(pMaskedEdit);

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeAnsiString:
		switch (Attr)
		{
		case 0:
		case 3:
			pMaskedEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
			break;

		case 2:
		case 5:
		case 8:
		case 16:
			pMaskedEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
			break;

		case 9:
			pMaskedEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "));
			break;

		case 11:
			pMaskedEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789- "));
			break;

		case 14:
			pMaskedEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/"));
			break;
		}

	case FMTypeUnicodeString:
		pMaskedEdit->SetLimitText(FMAttributes[Attr].DataParameter);

		break;

	case FMTypeUINT:
		pMaskedEdit->SetLimitText(6);
		pMaskedEdit->SetValidChars(_T("0123456789"));

		break;

	case FMTypeClass:
		pMaskedEdit->SetLimitText(2);
		pMaskedEdit->SetValidChars(_T("CFHJYcfhjy+"));

		break;

	case FMTypeDateTime:
		pMaskedEdit->SetCueBanner(CString((LPCSTR)IDS_CUEBANNER_DATETIME));
		pMaskedEdit->SetLimitText(16);
		pMaskedEdit->SetValidChars(_T("0123456789:-. "));

		break;

	case FMTypeTime:
		pMaskedEdit->SetCueBanner(CString((LPCSTR)IDS_CUEBANNER_TIME));
		pMaskedEdit->SetLimitText(5);
		pMaskedEdit->SetValidChars(_T("0123456789:"));

		break;
	}

	if (pFlight)
	{
		WCHAR tmpBuf[256];
		AttributeToString(*pFlight, Attr, tmpBuf, 256);

		pMaskedEdit->SetWindowText(tmpBuf);
	}
}

void DDX_MaskedText(CDataExchange* pDX, INT nIDC, CMFCMaskedEdit& rControl, UINT Attr, AIRX_Flight* pFlight)
{
	DDX_Control(pDX, nIDC, rControl);

	if (pDX->m_bSaveAndValidate)
	{
		if (pFlight)
		{
			CString tmpStr;
			rControl.GetWindowText(tmpStr);

			StringToAttribute(tmpStr, *pFlight, Attr);
		}
	}
	else
	{
		PrepareEditCtrl(&rControl, Attr, pFlight);
	}
}

BOOL Tokenize(const CString& strSrc, CString& strDst, INT& Pos, const CString& Delimiter, WCHAR* DelimiterFound)
{
	if (Pos>=strSrc.GetLength())
		return FALSE;

	strDst.Empty();

	while (Pos<strSrc.GetLength())
	{
		if (Delimiter.Find(strSrc[Pos])!=-1)
		{
			if (DelimiterFound)
				*DelimiterFound = strSrc[Pos];

			Pos++;

			return TRUE;
		}

		strDst.AppendChar(strSrc[Pos++]);
	}

	if (DelimiterFound)
		*DelimiterFound = L'\0';

	return TRUE;
}


// ToString
//

void DistanceToString(LPWSTR pStr, SIZE_T cCount, DOUBLE DistanceNM)
{
	ASSERT(pStr);

	if (theApp.m_UseStatuteMiles)
	{
		swprintf_s(pStr, cCount, L"%u mi (%u km)", (UINT)(DistanceNM*1.15077945), (UINT)(DistanceNM*1.852));
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u nm (%u km)", (UINT)DistanceNM, (UINT)(DistanceNM*1.852));
	}
}

void TimeToString(LPWSTR pStr, SIZE_T cCount, UINT Time)
{
	ASSERT(pStr);

	swprintf_s(pStr, cCount, L"%02d:%02d", Time/60, Time%60);
}

void DateTimeToString(LPWSTR pStr, SIZE_T cCount, const FILETIME& FileTime)
{
	ASSERT(pStr);

	SYSTEMTIME SystemTime;
	FileTimeToSystemTime(&FileTime, &SystemTime);

	if (SystemTime.wHour || SystemTime.wMinute)
	{
		swprintf_s(pStr, cCount, L"%04d-%02d-%02d %02d:%02d", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute);
	}
	else
	{
		swprintf_s(pStr, cCount, L"%04d-%02d-%02d", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);
	}
}

void RouteToString(LPWSTR pStr, SIZE_T cCount, AIRX_Route& Route)
{
	ASSERT(pStr);

	if (Route.DistanceNM==-1)
	{
		ASSERT(cCount>=2);

		pStr[0] = L'—';
		pStr[1] = L'\0';

		return;
	}

	CString From(Route.From);
	CString To(Route.To);

	WCHAR tmpBuf[256];
	DistanceToString(tmpBuf, 256, Route.DistanceNM);

	swprintf_s(pStr, cCount, L"%s–%s, %s", From.GetBuffer(), To.GetBuffer(), tmpBuf);
}

void MilesToString(LPWSTR pStr, SIZE_T cCount, LONG AwardMiles, LONG StatusMiles)
{
	ASSERT(pStr);

	swprintf_s(pStr, cCount, CString((LPCSTR)IDS_MILES), AwardMiles, StatusMiles);
}

void AttributeToString(const AIRX_Flight& Flight, UINT Attr, LPWSTR pStr, SIZE_T cCount)
{
	ASSERT(Attr<FMAttributeCount);
	ASSERT(pStr);
	ASSERT(cCount>=1);

	const LPVOID pData = (((LPBYTE)&Flight)+FMAttributes[Attr].Offset);
	*pStr = L'\0';

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeUnicodeString:
		wcscpy_s(pStr, cCount, (LPCWSTR)pData);
		break;

	case FMTypeAnsiString:
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pData, -1, pStr, (INT)cCount);
		break;

	case FMTypeUINT:
		if (*((UINT*)pData))
			swprintf_s(pStr, cCount, L"%u", *((UINT*)pData));

		break;

	case FMTypeDistance:
		if (Flight.Flags & AIRX_DistanceValid)
			DistanceToString(pStr, cCount, *((DOUBLE*)pData));

		break;

	case FMTypeFlags:
		if (Flight.Flags & AIRX_AwardFlight)
			wcscat_s(pStr, cCount, L"A");

		if (Flight.Flags & AIRX_GroundTransportation)
			wcscat_s(pStr, cCount, L"G");

		if (Flight.Flags & AIRX_BusinessTrip)
			wcscat_s(pStr, cCount, L"B");

		if (Flight.Flags & AIRX_LeisureTrip)
			wcscat_s(pStr, cCount, L"L");

		if (Flight.Flags & AIRX_Upgrade)
			wcscat_s(pStr, cCount, L"U");

		if (Flight.Flags & AIRX_Cancelled)
			wcscat_s(pStr, cCount, L"C");

		break;

	case FMTypeDateTime:
		if ((((LPFILETIME)pData)->dwHighDateTime!=0) || (((LPFILETIME)pData)->dwLowDateTime!=0))
			DateTimeToString(pStr, cCount, *((LPFILETIME)pData));

		break;

	case FMTypeTime:
		if (*((UINT*)pData))
			TimeToString(pStr, cCount, *((UINT*)pData));

		break;

	case FMTypeClass:
		switch (*((LPCSTR)pData))
		{
		case AIRX_Economy:
			wcscpy_s(pStr, cCount, L"Y");
			break;

		case AIRX_PremiumEconomy:
			wcscpy_s(pStr, cCount, L"Y+");
			break;

		case AIRX_Business:
			wcscpy_s(pStr, cCount, L"J");
			break;

		case AIRX_First:
			wcscpy_s(pStr, cCount, L"F");
			break;

		case AIRX_Crew:
			wcscpy_s(pStr, cCount, L"Crew/DCM");
			break;

		case AIRX_Charter:
			wcscpy_s(pStr, cCount, L"Charter");
			break;
		}

		break;

	case FMTypeColor:
		if (*((COLORREF*)pData)!=(COLORREF)-1)
			swprintf_s(pStr, cCount, L"#%06X", _byteswap_ulong(*((COLORREF*)pData)) >> 8);

		break;
	}
}


// FromString
//

inline void ScanUINT(LPCWSTR pStr, UINT& num)
{
	ASSERT(pStr);

	swscanf_s(pStr, L"%u", &num);
}

void ScanDateTime(LPCWSTR pStr, FILETIME& FileTime)
{
	ASSERT(pStr);

	INT cToken;

	UINT Year;
	UINT Month;
	UINT Day;
	UINT Hour;
	UINT Minute;

	SYSTEMTIME SystemTime;
	ZeroMemory(&SystemTime, sizeof(SystemTime));

	if ((cToken=swscanf_s(pStr, L"%u-%u-%u %u:%u", &Year, &Month, &Day, &Hour, &Minute))>=3)
	{
		SystemTime.wYear = (WORD)Year;
		SystemTime.wMonth = (WORD)Month;
		SystemTime.wDay = (WORD)Day;

		if (cToken==5)
		{
			SystemTime.wHour = (WORD)Hour;
			SystemTime.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&SystemTime, &FileTime);

		return;
	}

	if ((cToken=swscanf_s(pStr, L"%u/%u/%u %u:%u", &Month, &Day, &Year, &Hour, &Minute))>=3)
	{
		SystemTime.wYear = (WORD)Year;
		SystemTime.wMonth = (WORD)Month;
		SystemTime.wDay = (WORD)Day;

		if (cToken==5)
		{
			SystemTime.wHour = (WORD)Hour;
			SystemTime.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&SystemTime, &FileTime);

		return;
	}

	if ((cToken=swscanf_s(pStr, L"%u.%u.%u %u:%u", &Day, &Month, &Year, &Hour, &Minute))>=3)
	{
		SystemTime.wYear = (WORD)Year;
		SystemTime.wMonth = (WORD)Month;
		SystemTime.wDay = (WORD)Day;

		if (cToken==5)
		{
			SystemTime.wHour = (WORD)Hour;
			SystemTime.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&SystemTime, &FileTime);

		return;
	}

	FileTime.dwHighDateTime = FileTime.dwLowDateTime = 0;
}

void ScanTime(LPCWSTR pStr, UINT& Time)
{
	ASSERT(pStr);

	INT cToken;

	UINT Hour;
	UINT Minute;

	if ((cToken=swscanf_s(pStr, L"%u:%u", &Hour, &Minute))>=1)
	{
		Time = Hour*60;

		if (cToken==2)
			Time += min(Minute, 59);

		return;
	}

	Time = 0;
}

void ScanColor(LPCWSTR pStr, COLORREF& col)
{
	ASSERT(pStr);

	if (swscanf_s((*pStr==L'#') ? pStr+1 : pStr, L"%06X", &col)==1)
		if (col!=(COLORREF)-1)
		{
			col = (((UINT)col & 0xFF0000)>>16) | ((UINT)col & 0xFF00) | (((UINT)col & 0xFF)<<16);

			return;
		}

	col = (COLORREF)-1;
}

void StringToAttribute(CString tmpStr, AIRX_Flight& Flight, UINT Attr)
{
	ASSERT(Attr<FMAttributeCount);

	tmpStr.Trim();

	const LPVOID pData = (((LPBYTE)&Flight)+FMAttributes[Attr].Offset);

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeUnicodeString:
		if ((Attr==2) || (Attr==5) || (Attr==9))
			tmpStr.MakeUpper();

		wcsncpy_s((WCHAR*)pData, FMAttributes[Attr].DataParameter, tmpStr, _TRUNCATE);
		break;

	case FMTypeAnsiString:
		tmpStr.MakeUpper();

		WideCharToMultiByte(CP_ACP, 0, tmpStr, -1, (LPSTR)pData, FMAttributes[Attr].DataParameter, NULL, NULL);
		break;

	case FMTypeUINT:
		ScanUINT(tmpStr, *((UINT*)pData));
		break;

	case FMTypeFlags:
		Flight.Flags &= ~(AIRX_AwardFlight | AIRX_GroundTransportation | AIRX_BusinessTrip | AIRX_LeisureTrip);

		if (tmpStr.Find(L'A')!=-1)
			Flight.Flags |= AIRX_AwardFlight;

		if (tmpStr.Find(L'G')!=-1)
			Flight.Flags |= AIRX_GroundTransportation;

		if (tmpStr.Find(L'B')!=-1)
			Flight.Flags |= AIRX_BusinessTrip;

		if (tmpStr.Find(L'L')!=-1)
			Flight.Flags |= AIRX_LeisureTrip;

		if (tmpStr.Find(L'U')!=-1)
			Flight.Flags |= AIRX_Upgrade;

		if (tmpStr.Find(L'C')!=-1)
			Flight.Flags |= AIRX_Cancelled;

		break;

	case FMTypeDateTime:
		ScanDateTime(tmpStr, *((LPFILETIME)pData));
		break;

	case FMTypeTime:
		ScanTime(tmpStr, *((UINT*)pData));
		break;

	case FMTypeClass:
		tmpStr.MakeUpper();

		*((CHAR*)pData) =
			(tmpStr==_T("Y")) ? AIRX_Economy :
			(tmpStr==_T("Y+")) ? AIRX_PremiumEconomy :
			(tmpStr==_T("J")) ? AIRX_Business :
			(tmpStr==_T("F")) ? AIRX_First :
			((tmpStr==_T("C")) || (tmpStr==_T("CREW")) || (tmpStr==_T("CREW/DCM"))) ? AIRX_Crew :
			((tmpStr==_T("H")) || (tmpStr==_T("CHARTER"))) ? AIRX_Charter : AIRX_Unknown;
		break;

	case FMTypeColor:
		ScanColor(tmpStr, *((COLORREF*)pData));
		break;
	}
}


// CItinerary
//

#define ATTACHMENT_ENDBUFFER     1

CItinerary::CItinerary(const CString& Path)
{
	ZeroMemory(&m_Metadata, sizeof(m_Metadata));

	m_IsModified = m_IsOpen = FALSE;
	ENSURE(m_DisplayName.LoadString(IDS_EMPTYITINERARY));

	// Author
	FMLicense License;
	if (FMIsLicensed(&License))
	{
		MultiByteToWideChar(CP_ACP, 0, License.RegName, -1, m_Metadata.Author, 256);
	}
	else
	{
		HKEY hKey;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ | KEY_WOW64_64KEY, &hKey)==ERROR_SUCCESS)
		{
			DWORD dwSize = 256;
			RegQueryValueEx(hKey, L"RegisteredOwner", NULL, NULL, (LPBYTE)&m_Metadata.Author, &dwSize);

			RegCloseKey(hKey);
		}
	}

	// File
	if (!Path.IsEmpty())
	{
		CString Ext = Path;
		Ext.Delete(0, Ext.ReverseFind(L'\\')+1);
		Ext.Delete(0, Ext.ReverseFind(L'.')+1);
		Ext.MakeLower();

		if (Ext==_T("airx"))
			OpenAIRX(Path);

		if (Ext==_T("csv"))
			OpenCSV(Path);
	}
}

CItinerary::CItinerary(CItinerary* pItinerary)
{
	ASSERT(pItinerary);

	m_IsModified = FALSE;
	m_IsOpen = TRUE;
	m_Metadata = pItinerary->m_Metadata;
	m_Metadata.CurrentRow = 0;

	if (pItinerary->m_FileName[0])
	{
		SetDisplayName(pItinerary->m_FileName);

		m_DisplayName += CString((LPCSTR)IDS_FILTEREDITINERARY);
	}
	else
	{
		m_DisplayName = pItinerary->m_DisplayName;
	}
}

CItinerary::~CItinerary()
{
	for (UINT a=0; a<m_Attachments.m_ItemCount; a++)
		FreeAttachment(m_Attachments[a]);
}


// Sample itineraries

FILETIME CItinerary::MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute)
{
	SYSTEMTIME SystemTime;
	SystemTime.wYear = wYear;
	SystemTime.wMonth = wMonth;
	SystemTime.wDay = wDay;
	SystemTime.wHour = wHour;
	SystemTime.wMinute = wMinute;
	SystemTime.wDayOfWeek = SystemTime.wMilliseconds = 0;

	FILETIME FileTime;
	SystemTimeToFileTime(&SystemTime, &FileTime);

	return FileTime;
}

void CItinerary::NewSampleAtlantic()
{
	ASSERT(!m_IsOpen);

	ENSURE(m_DisplayName.LoadString(IDS_SAMPLEITINERARY));
	m_IsOpen = TRUE;

	wcscpy_s(m_Metadata.Author, 256, L"liquidFOLDERS");
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_COMMENTS, m_Metadata.Comments, 256));
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_TITLE_ATLANTIC, m_Metadata.Title, 256));
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_KEYWORDS_ATLANTIC, m_Metadata.Keywords, 256));

	AddFlight("DUS", "FRA", L"Lufthansa", L"Boeing 737", "LH 803", AIRX_Economy, "9F", "", L"", 500, (COLORREF)-1, MakeTime(2007, 1, 25, 6, 15));
	AddFlight("FRA", "JFK", L"Lufthansa", L"Airbus A340", "LH 400", AIRX_Crew, "F/D", "D-AIHD", L"Stuttgart", 2565, (COLORREF)-1, MakeTime(2007, 1, 25, 9, 35));
	AddFlight("EWR", "SFO", L"Continental Airlines", L"Boeing 737", "CO 572", AIRX_Economy, "15F", "", L"", 0, 0xFFC0A0, MakeTime(2007, 1, 28, 11, 45));
	AddFlight("SFO", "MUC", L"Lufthansa", L"Airbus A340", "LH 459", AIRX_Economy, "38H", "D-AIHB", L"Bremerhaven", 5864, (COLORREF)-1, MakeTime(2007, 2, 5, 21, 50));
	AddFlight("MUC", "DUS", L"Lufthansa", L"Airbus A320", "LH 848", AIRX_Economy, "21A", "", L"", 500, (COLORREF)-1, MakeTime(2007, 2, 6, 19, 30));
}

void CItinerary::NewSamplePacific()
{
	ASSERT(!m_IsOpen);

	ENSURE(m_DisplayName.LoadString(IDS_SAMPLEITINERARY));
	m_IsOpen = TRUE;

	wcscpy_s(m_Metadata.Author, 256, L"liquidFOLDERS");
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_COMMENTS, m_Metadata.Comments, 256));
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_TITLE_PACIFIC, m_Metadata.Title, 256));
	ENSURE(LoadString(AfxGetResourceHandle(), IDS_METADATA_KEYWORDS_PACIFIC, m_Metadata.Keywords, 256));

	AddFlight("YVR", "DFW", L"American Airlines", L"Boeing 737", "AA 260", AIRX_Economy, "16A", "", L"", 1522, (COLORREF)-1, MakeTime(2012, 7, 9, 12, 15));
	AddFlight("DFW", "LAX", L"American Airlines", L"Boeing 737", "AA 2489", AIRX_Economy, "18D", "", L"", 1070, (COLORREF)-1, MakeTime(2012, 7, 9, 20, 35));
	AddFlight("LAX", "MEL", L"Quantas", L"Airbus A380", "QF 94", AIRX_Economy, "75A", "VH-OQF", L"Charles Kingsford Smith", 6886, 0x0000FF, MakeTime(2012, 7, 9, 23, 30));
	AddFlight("MEL", "LAX", L"Quantas", L"Airbus A380", "QF 93", AIRX_Economy, "88E", "VH-OQH", L"Reginald Ansett", 6886, 0x0000FF, MakeTime(2012, 7, 23, 9, 35));
	AddFlight("LAX", "DFW", L"American Airlines", L"Boeing 737", "AA 2436", AIRX_Economy, "23F", "", L"", 1070, (COLORREF)-1, MakeTime(2012, 7, 23, 10, 45));
	AddFlight("DFW", "YVR", L"American Airlines", L"Boeing 737", "AA 887", AIRX_Economy, "22B", "", L"", 1522, (COLORREF)-1, MakeTime(2012, 7, 23, 18, 25));
}


// Open and save

void CItinerary::SetDisplayName(const CString& Path)
{
	const LPCWSTR pChar = wcsrchr(Path, L'\\');

	m_DisplayName = pChar ? pChar+1 : Path;
}

void CItinerary::CompressFile(HANDLE hFile, WCHAR cDrive, BOOL bCompress)
{
	BY_HANDLE_FILE_INFORMATION FileInformation;
	if (GetFileInformationByHandle(hFile, &FileInformation) && ((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)==0))
	{
		WCHAR Root[4] = L" :\\";
		Root[0] = cDrive;

		DWORD Flags;
		if (GetVolumeInformation(Root, NULL, 0, NULL, NULL, &Flags, NULL, 0))
			if (Flags & FS_FILE_COMPRESSION)
			{
				USHORT Mode = bCompress ? COMPRESSION_FORMAT_LZNT1 : COMPRESSION_FORMAT_NONE;
				DWORD Returned;

				DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &Mode, sizeof(Mode), NULL, 0, &Returned, NULL);
			}
	}
}

BOOL CItinerary::ReadRecord(CFile& File, LPVOID pBuffer, UINT BufferSize, UINT OnDiscSize)
{
	ASSERT(pBuffer);

	UINT Read = BufferSize ? File.Read(pBuffer, min(BufferSize, OnDiscSize)) : 0;

	if (OnDiscSize>BufferSize)
		File.Seek(OnDiscSize-BufferSize, CFile::current);

	return Read==min(BufferSize, OnDiscSize);
}

void CItinerary::OpenAIRX(const CString& Path)
{
	ASSERT(!m_IsOpen);

	CFile File;
	if (File.Open(Path, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(m_FileName=Path);
		theApp.AddToRecentFiles(Path);
		SHAddToRecentDocs(SHARD_PATHW, Path);

		SYSTEMTIME stLocal;
		GetLocalTime(&stLocal);

		try
		{
			// No need to zero out the header
			AIRX_Header Header;

			if ((File.Read(&Header, sizeof(Header))==sizeof(Header)) && (Header.Magic==0x58524941))
			{
				// m_Metadata is already zeroed out in constructor
				ReadRecord(File, &m_Metadata, sizeof(m_Metadata), Header.MetadataRecordSize);

				// Reset flight once to accomodate future expansion
				AIRX_Flight Flight;
				ResetFlight(Flight);

				for (UINT a=0; a<Header.FlightCount; a++)
				{
					if (ReadRecord(File, &Flight, sizeof(Flight), Header.FlightRecordSize))
					{
						// Mask out volatile flags
						Flight.Flags &= (AIRX_AwardFlight | AIRX_BusinessTrip | AIRX_LeisureTrip | AIRX_GroundTransportation | AIRX_Selected | AIRX_Cancelled | AIRX_Upgrade);

						// Precaution for future increase of max. attachment count
						if (Flight.AttachmentCount>AIRX_MaxAttachmentCount)
							Flight.AttachmentCount = AIRX_MaxAttachmentCount;

						AddFlight(Flight, &stLocal);
					}
				}

				if (m_Metadata.CurrentRow>m_Flights.m_ItemCount)
					m_Metadata.CurrentRow = m_Flights.m_ItemCount;

				AIRX_Attachment Attachment;
				ZeroMemory(&Attachment, sizeof(Attachment));

				for (UINT a=0; a<Header.AttachmentCount; a++)
					if (ReadRecord(File, &Attachment, sizeof(Attachment), Header.AttachmentRecordSize))
					{
						Attachment.IconID = -1;

						LPVOID pData = malloc(Attachment.FileSize+ATTACHMENT_ENDBUFFER);
						if (pData)
							if (File.Read(pData, Attachment.FileSize)==Attachment.FileSize)
							{
								Attachment.pData = pData;
								ValidateAttachment(Attachment, TRUE);

								m_Attachments.AddItem(Attachment);
							}
							else
							{
								free (pData);
							}
					}
			}
			else
			{
				FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
			}
		}
		catch(CFileException ex)
		{
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}

		File.Close();
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}

void CItinerary::OpenCSV(const CString& Path)
{
	ASSERT(!m_IsOpen);

	CStdioFile File;
	if (File.Open(Path, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(Path);
		theApp.AddToRecentFiles(Path);
		SHAddToRecentDocs(SHARD_PATHW, Path);

		SYSTEMTIME stLocal;
		GetLocalTime(&stLocal);

		WCHAR Separator[4];
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, Separator, 4);

		try
		{
			CString Caption;
			if (File.ReadString(Caption))
			{
				CMap<CString, LPCTSTR, INT, INT&> Map;
				Map[L"FROM"] = 0;
				Map[L"FRM IATA"] = 0;
				Map[L"FROM IATA"] = 0;
				Map[L"ORT VON"] = 0;
				Map[L"START"] = 0;
				Map[L"DATE & TIME"] = 1;
				Map[L"DATE"] = 1;
				Map[L"DATUM"] = 1;
				Map[L"DEPARTURE"] = 1;
				Map[L"DEPT. TIME"] = 1;
				Map[L"ZEIT VON"] = 1;
				Map[L"DEPT. GATE"] = 2;
				Map[L"TO"] = 3;
				Map[L"TO IATA"] = 3;
				Map[L"ORT BIS"] = 3;
				Map[L"ZIEL"] = 3;
				Map[L"ARRIVAL"] = 4;
				Map[L"ARR. TIME"] = 4;
				Map[L"ZEIT BIS"] = 4;
				Map[L"ARR. GATE"] = 5;
				Map[L"AIRLINE"] = 7;
				Map[L"CARRIER"] = 7;
				Map[L"OPERATOR"] = 7;
				Map[L"FLIGHT"] = 8;
				Map[L"FLIGHT #"] = 8;
				Map[L"FLIGHT_NUMBER"] = 8;
				Map[L"FLUGNR"] = 8;
				Map[L"CODESHARE"] = 9;
				Map[L"CODESHARES"] = 9;
				Map[L"EQUIPMENT"] = 10;
				Map[L"MUSTER"] = 10;
				Map[L"PLANE"] = 10;
				Map[L"TYPE"] = 10;
				Map[L"AIRCRAFT REGISTRATION"] = 11;
				Map[L"REGISTRATION"] = 11;
				Map[L"TAIL"] = 11;
				Map[L"AIRCRAFT NAME"] = 12;
				Map[L"CLASS"] = 13;
				Map[L"KLASSE"] = 13;
				Map[L"SEAT"] = 14;
				Map[L"SITZ"] = 14;
				Map[L"COLOR"] = 15;
				Map[L"ETIX"] = 16;
				Map[L"BOOKING"] = 16;
				Map[L"ETIX CODE"] = 16;
				Map[L"ETIX BOOKING CODE"] = 16;
				Map[L"FARE"] = 17;
				Map[L"PRICE"] = 17;
				Map[L"AWARD MILES"] = 18;
				Map[L"STATUS MILES"] = 19;
				Map[L"FLAGS"] = 20;
				Map[L"RATING"] = 21;
				Map[L"COMMENTS"] = 22;
				Map[L"FLIGHT TIME"] = 23;
				Map[L"VOUCHER"] = 24;
				Map[L"EVOUCHER"] = 24;
				Map[L"UPGRADE VOUCHER"] = 24;

				INT RouteMapping = -1;
				INT Mapping[100];
				for (UINT a=0; a<100; a++)
					Mapping[a] = -1;

				// Header
				INT Pos = 0;
				UINT Column = 0;
				CString resToken;
				while (Tokenize(Caption, resToken, Pos, Separator) && (Column<100))
				{
					resToken.MakeUpper();

					if (resToken==_T("ROUTE"))
					{
						RouteMapping = Column;
					}
					else
					{
						Map.Lookup(resToken, Mapping[Column]);
					}

					Column++;
				}

				// Flights
				CString Line;
				CString Route;
				while (File.ReadString(Line))
				{
					AIRX_Flight Flight;
					ResetFlight(Flight);

					Route.Empty();
					Pos = 0;
					Column = 0;

					while (Tokenize(Line, resToken, Pos, Separator) && (Column<100))
					{
						if ((INT)Column==RouteMapping)
						{
							Route = resToken;
						}
						else
							if (Mapping[Column]!=-1)
							{
								StringToAttribute(resToken, Flight, Mapping[Column]);
							}

						Column++;
					}

					if (Route.IsEmpty())
					{
						AddFlight(Flight, &stLocal);
					}
					else
					{
						CString From;
						CString To;
						Pos = 0;
						WCHAR DelimiterFound;

						while (Tokenize(Route, resToken, Pos, _T("-/,"), &DelimiterFound))
						{
							From = To;
							To = resToken;

							if (!From.IsEmpty() && !To.IsEmpty())
							{
								StringToAttribute(From, Flight, 0);
								StringToAttribute(To, Flight, 3);

								AddFlight(Flight, &stLocal);
							}

							if (DelimiterFound==L',')
								To.Empty();
						}
					}
				}
			}
		}
		catch(CFileException ex)
		{
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}

		File.Close();
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}

void CItinerary::SaveAIRX(const CString& Path, UINT CurrentRow)
{
	ASSERT(CurrentRow<=m_Flights.m_ItemCount);

	// Current row
	m_Metadata.CurrentRow = CurrentRow;

	// Save file
	CFile File;
	if (File.Open(Path, CFile::modeCreate | CFile::modeReadWrite | CFile::osSequentialScan))
	{
		// Compression
		BOOL bCompress = TRUE;
		for (UINT a=0; a<m_Attachments.m_ItemCount; a++)
		{
			CString Ext(m_Attachments[a].Name);

			const INT Pos = Ext.ReverseFind(L'.');
			if (Pos!=-1)
			{
				Ext = Ext.Mid(Pos+1).MakeLower();

				if ((Ext==_T("7z")) || (Ext==_T("avi")) || (Ext==_T("jpeg")) || (Ext==_T("jpg")) || (Ext==_T("mkv")) || (Ext==_T("mov")) || 
					(Ext==_T("mp3")) || (Ext==_T("mp4")) || (Ext==_T("mpg")) || (Ext==_T("png")) || (Ext==_T("zip")))
				{
					bCompress = FALSE;
					break;
				}
			}
		}

		CompressFile(File.m_hFile, Path[0], bCompress);

		// Write file
		SetDisplayName(m_FileName=Path);
		theApp.AddToRecentFiles(Path);
		SHAddToRecentDocs(SHARD_PATHW, Path);

		try
		{
			AIRX_Header Header;
			ZeroMemory(&Header, sizeof(Header));
			Header.Magic = 0x58524941;
			Header.MetadataRecordSize = sizeof(m_Metadata);
			Header.FlightCount = m_Flights.m_ItemCount;
			Header.FlightRecordSize = sizeof(AIRX_Flight);
			Header.AttachmentCount = m_Attachments.m_ItemCount;
			Header.AttachmentRecordSize = sizeof(AIRX_Attachment);

			File.Write(&Header, sizeof(Header));
			File.Write(&m_Metadata, sizeof(m_Metadata));

			if (Header.FlightCount>0)
				File.Write(&m_Flights[0], Header.FlightCount*Header.FlightRecordSize);

			for (UINT a=0; a<Header.AttachmentCount; a++)
			{
				// Create copy of attachment record
				AIRX_Attachment Attachment = m_Attachments[a];
				Attachment.IconID = -1;

				if (!Attachment.pData)
				{
					Attachment.FileSize = 0;
				}
				else
				{
					Attachment.pData = NULL;
				}

				// Attachment record
				File.Write(&Attachment, sizeof(Attachment));

				// File body
				if (m_Attachments[a].pData && Attachment.FileSize)
					File.Write(m_Attachments[a].pData, Attachment.FileSize);
			}

			m_IsModified = FALSE;
		}
		catch(CFileException ex)
		{
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}

		File.Close();
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}


// Editing itinerary

UINT CItinerary::GetFlightCount(BOOL Limit) const
{
	return Limit && !FMIsLicensed() ? min(m_Flights.m_ItemCount, 10) : m_Flights.m_ItemCount;
}

void CItinerary::ResetFlight(AIRX_Flight& Flight)
{
	ZeroMemory(&Flight, sizeof(AIRX_Flight));

	Flight.Waypoint.Latitude = Flight.Waypoint.Longitude = Flight.DistanceNM = 0.0;
	Flight.Color = (COLORREF)-1;
}

void CItinerary::UpdateFlight(AIRX_Flight& Flight, LPSYSTEMTIME pTime)
{
	// Remove flags
	Flight.Flags &= ~(AIRX_FutureFlight | AIRX_DistanceValid | AIRX_UnknownFrom | AIRX_UnknownTo);

	// Find airports
	LPCAIRPORT pFrom;
	if (!FMIATAGetAirportByCode(Flight.From.Code, pFrom))
		Flight.Flags |= AIRX_UnknownFrom;

	LPCAIRPORT pTo;
	if (!FMIATAGetAirportByCode(Flight.To.Code, pTo))
		Flight.Flags |= AIRX_UnknownTo;

	// Great Circle distance
	if ((Flight.Flags & (AIRX_UnknownFrom | AIRX_UnknownTo))==0)
	{
		const BOOL UseWaypoint = (pFrom==pTo) && ((Flight.Waypoint.Latitude!=0) || (Flight.Waypoint.Longitude!=0));
		const DOUBLE Lat1 = PI*pFrom->Location.Latitude/180;
		const DOUBLE Lon1 = PI*pFrom->Location.Longitude/180;
		const DOUBLE Lat2 = PI*(UseWaypoint ? Flight.Waypoint.Latitude : pTo->Location.Latitude)/180;
		const DOUBLE Lon2 = PI*(UseWaypoint ? Flight.Waypoint.Longitude : pTo->Location.Longitude)/180;

		const DOUBLE DeltaLon = abs(Lon1-Lon2);
		const DOUBLE tmp1 = cos(Lat1)*sin(DeltaLon);
		const DOUBLE tmp2 = cos(Lat2)*sin(Lat1)-sin(Lat2)*cos(Lat1)*cos(DeltaLon);
		const DOUBLE T = sqrt(tmp1*tmp1+tmp2*tmp2);
		const DOUBLE B = sin(Lat2)*sin(Lat1)+cos(Lat2)*cos(Lat1)*cos(DeltaLon);
		const DOUBLE GreatCircle = atan2(T, B);

		Flight.DistanceNM = 3438.461*GreatCircle;
		if (UseWaypoint)
			Flight.DistanceNM *= 2.0;

		Flight.Flags |= AIRX_DistanceValid;
	}
	else
	{
		Flight.DistanceNM = 0.0;
	}

	// Future departure time
	if (Flight.From.Time.dwLowDateTime || Flight.From.Time.dwHighDateTime)
	{
		SYSTEMTIME stLocal;
		if (!pTime)
			GetLocalTime(pTime=&stLocal);

		SYSTEMTIME stFlight;
		FileTimeToSystemTime(&Flight.From.Time, &stFlight);

		if ((stFlight.wYear>pTime->wYear) ||
			((stFlight.wYear==pTime->wYear) && ((stFlight.wMonth>pTime->wMonth) || ((stFlight.wMonth==pTime->wMonth) && (stFlight.wDay>pTime->wDay)))))
			Flight.Flags |= AIRX_FutureFlight;
	}
}

void CItinerary::AddFlight(CItinerary* pItinerary, UINT Row)
{
	ASSERT(pItinerary);
	ASSERT(Row<pItinerary->m_Flights.m_ItemCount);

	AIRX_Flight Flight = pItinerary->m_Flights[Row];
	Flight.AttachmentCount = 0;

	for (UINT a=0; a<pItinerary->m_Flights[Row].AttachmentCount; a++)
	{
		const INT Index = AddAttachment(pItinerary, pItinerary->m_Flights[Row].Attachments[a]);
		if (Index>=0)
			Flight.Attachments[Flight.AttachmentCount++] = Index;
	}

	AddFlight(Flight);
}

void CItinerary::AddFlight(LPCSTR From, LPCSTR To, LPCWSTR Carrier, LPCWSTR Equipment, LPCSTR FlightNo, CHAR Class, LPCSTR Seat, LPCSTR Registration, LPCWSTR Name, UINT Miles, COLORREF Color, const FILETIME& Departure)
{
	AIRX_Flight Flight;
	ResetFlight(Flight);

	strcpy_s(Flight.From.Code, 4, From);
	strcpy_s(Flight.To.Code, 4, To);
	wcscpy_s(Flight.Carrier, 64, Carrier);
	wcscpy_s(Flight.Equipment, 64, Equipment);
	strcpy_s(Flight.FlightNo, 8, FlightNo);
	Flight.Class = Class;
	strcpy_s(Flight.Seat, 4, Seat);
	strcpy_s(Flight.Registration, 16, Registration);
	wcscpy_s(Flight.Name, 64, Name);
	Flight.MilesAward = Flight.MilesStatus = Miles;
	Flight.Color = Color;
	Flight.From.Time = Departure;

	AddFlight(Flight);
}

void CItinerary::InsertFlights(UINT Row, UINT Count, AIRX_Flight* pFlights)
{
	if (m_Flights.InsertEmpty(Row, Count, FALSE) && pFlights)
	{
		memcpy(&m_Flights[Row], pFlights, Count*sizeof(AIRX_Flight));

		SYSTEMTIME stLocal;
		GetLocalTime(&stLocal);

		for (UINT a=Row; a<Row+Count; a++)
		{
			UpdateFlight(a, &stLocal);

			m_Flights[a].AttachmentCount = 0;
		}
	}
	else
	{
		for (UINT a=Row; a<Row+Count; a++)
			ResetFlight(a);
	}
}

void CItinerary::DeleteFlight(UINT Row)
{
	while (m_Flights[Row].AttachmentCount)
		DeleteAttachment(m_Flights[Row].Attachments[0]);

	m_Flights.DeleteItems(Row);
	m_IsModified = TRUE;
}

void CItinerary::DeleteSelectedFlights()
{
	UINT Row = 0;
	while (Row<m_Flights.m_ItemCount)
	{
		if (m_Flights[Row].Flags & AIRX_Selected)
		{
			DeleteFlight(Row);
		}
		else
		{
			Row++;
		}
	}
}


// Conversion

CString CItinerary::ExportAttribute(const AIRX_Flight& Flight, UINT Attr, BOOL Label, BOOL Colon, BOOL NewLine)
{
	WCHAR tmpBuf[256];
	AttributeToString(Flight, Attr, tmpBuf, 256);

	if (tmpBuf[0]==L'\0')
		return _T("");

	CString tmpStr;
	if (Label)
	{
		ENSURE(tmpStr.LoadString(IDS_COLUMN0+Attr));
		tmpStr.Append(Colon ? _T(": ") : _T(" "));
	}

	tmpStr.Append(tmpBuf);
	if (NewLine)
		tmpStr.Append(_T("\n"));

	return tmpStr;
}

CString CItinerary::ExportLocation(const AIRX_Flight& Flight, UINT AttrBase)
{
	const CString DateTime = ExportAttribute(Flight, AttrBase+1, FALSE, FALSE, FALSE);
	const CString Gate = ExportAttribute(Flight, AttrBase+2, TRUE, FALSE, FALSE);

	CString tmpStr = ExportAttribute(Flight, AttrBase, TRUE, TRUE, FALSE);

	if (!DateTime.IsEmpty() || !Gate.IsEmpty())
	{
		tmpStr += _T(" (");

		if (!DateTime.IsEmpty())
		{
			tmpStr += DateTime;
			if (!Gate.IsEmpty())
			{
				tmpStr += _T(", ");
				tmpStr += Gate;
			}
		}
		else
		{
			tmpStr += Gate;
		}

		tmpStr += _T(")");
	}

	return tmpStr+_T("\n");
}

CString CItinerary::Flight2Text(UINT Row) const
{
	ASSERT(Row<m_Flights.m_ItemCount);

	CString tmpStr = ExportLocation(m_Flights[Row], 0);
	tmpStr += ExportLocation(m_Flights[Row], 3);

	const UINT Attrs[15] = { 6, 23, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 19, 22 };
	for (UINT a=0; a<15; a++)
		tmpStr += ExportAttribute(m_Flights[Row], Attrs[a]);

	if (!tmpStr.IsEmpty())
		tmpStr += _T("\n");

	return tmpStr;
}

AIRX_Attachment* CItinerary::GetGPSPath(UINT Row)
{
	ASSERT(Row<m_Flights.m_ItemCount);

	for (UINT a=0; a<m_Flights[Row].AttachmentCount; a++)
	{
		AIRX_Attachment* pAttachment = &m_Attachments[m_Flights[Row].Attachments[a]];

		LPCWSTR pExtension = wcsrchr(pAttachment->Name, L'.');
		if (pExtension && (_wcsicmp(pExtension, L".gpx")==0))
		{
			ValidateAttachment(*pAttachment);

			if (pAttachment->Flags & AIRX_Valid)
				return pAttachment;
		}
	}

	return NULL;
}


// Sorting

INT CItinerary::CompareItems(const AIRX_Flight* pFlight1, const AIRX_Flight* pFlight2, const SortParameters& Parameters)
{
	ASSERT(Parameters.Attr<FMAttributeCount);
	ASSERT(FMAttributes[Parameters.Attr].Sortable);

	const LPCVOID pData1 = (LPBYTE)pFlight1+FMAttributes[Parameters.Attr].Offset;
	const LPCVOID pData2 = (LPBYTE)pFlight2+FMAttributes[Parameters.Attr].Offset;

	// Compare primary attributes
	INT Compare = 0;

	switch (FMAttributes[Parameters.Attr].Type)
	{
	case FMTypeUnicodeString:
		Compare = _wcsicmp((LPCWSTR)pData1, (LPCWSTR)pData2);
		break;

	case FMTypeAnsiString:
		Compare = _stricmp((LPCSTR)pData1, (LPCSTR)pData2);
		break;

	case FMTypeRating:
		Compare = (INT)((*(UINT*)pData1)>>FMAttributes[Parameters.Attr].DataParameter)-(INT)((*(UINT*)pData2)>>FMAttributes[Parameters.Attr].DataParameter);
		break;

	case FMTypeUINT:
	case FMTypeTime:
		Compare = *(UINT*)pData1==*(UINT*)pData2 ? 0 : *(UINT*)pData1<*(UINT*)pData2 ? -1 : 1;
		break;

	case FMTypeDistance:
		Compare = *(DOUBLE*)pData1==*(DOUBLE*)pData2 ? 0 : *(DOUBLE*)pData1<*(DOUBLE*)pData2 ? -1 : 1;
		break;

	case FMTypeDateTime:
		Compare = CompareFileTime((LPFILETIME)pData1, (LPFILETIME)pData2);
		break;

	case FMTypeClass:
		Compare = (INT)(*(UCHAR*)pData1)-(INT)(*(UCHAR*)pData2);
		break;

	default:
		ASSERT(FALSE);
	}

	// Secondary
	if (!Compare && (Parameters.Attr!=1))
		Compare = CompareFileTime(&pFlight1->From.Time, &pFlight2->From.Time);

	if (!Compare && (Parameters.Attr!=0))
		Compare = strcmp(pFlight1->From.Code, pFlight2->From.Code);

	if (!Compare && (Parameters.Attr!=3))
		Compare = strcmp(pFlight1->To.Code, pFlight2->To.Code);

	return Parameters.Descending ? -Compare : Compare;
}


// Attachments

BOOL CItinerary::AddAttachment(AIRX_Flight& Flight, const CString& Path)
{
	// Attachment count
	if (Flight.AttachmentCount>=AIRX_MaxAttachmentCount)
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_ATTACHMENT_TOOMANY);

		return FALSE;
	}

	// FindFirst
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(Path, &FindFileData);

	if (hFind==INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);

	// Metadata
	AIRX_Attachment Attachment;
	wcscpy_s(Attachment.Name, MAX_PATH, Path.Mid(Path.ReverseFind(L'\\')+1));
	Attachment.Created = FindFileData.ftCreationTime;
	Attachment.Modified = FindFileData.ftLastWriteTime;
	Attachment.IconID = -1;
	Attachment.Flags = 0;

	// Check for 1 MB size limit
	if ((Attachment.FileSize=(((INT64)FindFileData.nFileSizeHigh) << 32)+FindFileData.nFileSizeLow)>1024*1024)
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_ATTACHMENT_TOOLARGE);

		return FALSE;
	}

	// Allocate memory
	ASSERT(ATTACHMENT_ENDBUFFER>=1);

	if ((Attachment.pData=malloc(Attachment.FileSize+ATTACHMENT_ENDBUFFER))==NULL)
		return FALSE;

	BOOL Result = FALSE;

	// Read file contents
	CFile File;
	if (File.Open(Path, CFile::modeRead | CFile::osSequentialScan))
	{
		try
		{
			File.Read(Attachment.pData, Attachment.FileSize);
			ValidateAttachment(Attachment, TRUE);

			if (m_Attachments.AddItem(Attachment))
			{
				Flight.Attachments[Flight.AttachmentCount++] = m_Attachments.m_ItemCount-1;
				Result = m_IsModified = TRUE;
			}
		}
		catch(CFileException ex)
		{
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}

		File.Close();
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}

	if (!Result && Attachment.pData)
		FreeAttachment(Attachment);

	return Result;
}

INT CItinerary::AddAttachment(CItinerary* pItinerary, UINT Index)
{
	ASSERT(pItinerary);
	ASSERT(Index<pItinerary->m_Attachments.m_ItemCount);

	AIRX_Attachment Attachment = pItinerary->m_Attachments[Index];
	ASSERT(ATTACHMENT_ENDBUFFER>=1);

	if ((Attachment.pData=malloc(Attachment.FileSize+ATTACHMENT_ENDBUFFER))==NULL)
		return -1;

	memcpy(Attachment.pData, pItinerary->m_Attachments[Index].pData, Attachment.FileSize);

	return m_Attachments.AddItem(Attachment) ? m_Attachments.m_ItemCount-1 : -1;
}

CGPXFile* CItinerary::DecodeGPXAttachment(const AIRX_Attachment& Attachment)
{
	if (Attachment.FileSize<16)
		return NULL;

	ASSERT(ATTACHMENT_ENDBUFFER>=1);
	ASSERT(_msize(Attachment.pData)>=Attachment.FileSize+ATTACHMENT_ENDBUFFER);

	// Terminating NUL
	((LPSTR)Attachment.pData)[Attachment.FileSize] = '\0';

	CGPXFile* pGPXFile = new CGPXFile();

	try
	{
		pGPXFile->parse<parse_non_destructive | parse_no_data_nodes>((LPSTR)Attachment.pData);
	}
	catch(parse_error e)
	{
		delete pGPXFile;

		pGPXFile = NULL;
	}

	return pGPXFile;
}

Bitmap* CItinerary::DecodePictureAttachment(const AIRX_Attachment& Attachment)
{
	IStream* pStream = SHCreateMemStream((LPBYTE)Attachment.pData, Attachment.FileSize);

	Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);

	pStream->Release();

	return pBitmap;
}

void CItinerary::ValidateAttachment(AIRX_Attachment& Attachment, BOOL Force)
{
	// No check neccessary
	if (!Force && (Attachment.Flags & (AIRX_Valid | AIRX_Invalid)))
	{
		if (Attachment.Flags & AIRX_Invalid)
			Attachment.Flags &= ~AIRX_Valid;

		return;
	}

	// Remove flags
	Attachment.Flags &= ~(AIRX_Valid | AIRX_Invalid);

	// Check .gpx file
	LPCWSTR pExtension = wcsrchr(Attachment.Name, L'.');
	if (pExtension && (_wcsicmp(pExtension, L".gpx")==0))
	{
		CGPXFile* pGPXFile;
		Attachment.Flags |= (pGPXFile=DecodeGPXAttachment(Attachment))!=NULL ? AIRX_Valid : AIRX_Invalid;

		delete pGPXFile;
	}
}

void CItinerary::RenameAttachment(AIRX_Attachment& Attachment, const CString& Name)
{
	if (Name.IsEmpty())
		return;

	// Save extension for later
	CString Extension(Attachment.Name);
	
	const INT Pos = Extension.ReverseFind(L'.');
	if (Pos!=-1)
		Extension.Delete(0, Pos);

	// Copy and sanitize name
	wcscpy_s(Attachment.Name, MAX_PATH, Name);

	WCHAR* pChar = Attachment.Name;
	while (*pChar)
	{
		if (wcschr(L"<>:\"/\\|?*", *pChar))
			*pChar = L'_';

		pChar++;
	}

	// Concatenate saved extension
	wcscat_s(Attachment.Name, MAX_PATH, Extension);

	m_IsModified = TRUE;
}

void CItinerary::RemoveAttachmentFromFlight(UINT Index, AIRX_Flight& Flight)
{
	UINT No = 0;
	while (No<Flight.AttachmentCount)
		if (Flight.Attachments[No]==Index)
		{
			Flight.AttachmentCount--;

			if (No<Flight.AttachmentCount)
				for (UINT a=No; a<Flight.AttachmentCount; a++)
					Flight.Attachments[a] = Flight.Attachments[a+1];
		}
		else
		{
			if (Flight.Attachments[No]>Index)
				Flight.Attachments[No]--;

			No++;
		}
}

void CItinerary::DeleteAttachment(UINT Index, AIRX_Flight* pFlight)
{
	ASSERT(Index<m_Attachments.m_ItemCount);

	// Free attachment
	FreeAttachment(m_Attachments[Index]);

	// Compact attachment list
	m_Attachments.m_ItemCount--;
	for (UINT a=Index; a<m_Attachments.m_ItemCount; a++)
		m_Attachments[a] = m_Attachments[a+1];

	// Adjust attachment IDs in all flights
	for (UINT a=0; a<m_Flights.m_ItemCount; a++)
		RemoveAttachmentFromFlight(Index, m_Flights[a]);

	// Adjust attachment IDs in separate flight (that is not part of this itinerary)
	if (pFlight)
		RemoveAttachmentFromFlight(Index, *pFlight);

	m_IsModified = TRUE;
}

void CItinerary::DeleteAttachments(AIRX_Flight& Flight)
{
	// Delete all attachments of a separate flight (that is not party of this itinerary)
	for (UINT a=0; a<Flight.AttachmentCount; a++)
		DeleteAttachment(Flight.Attachments[a]);

	Flight.AttachmentCount = 0;
}


// User interface

void CItinerary::PrepareCarrierCtrl(CComboBox& ComboBox, BOOL IncludeDatabase) const
{
	for (UINT a=0; a<m_Flights.m_ItemCount; a++)
		if (m_Flights[a].Carrier[0] && (ComboBox.FindStringExact(-1, m_Flights[a].Carrier)==CB_ERR))
			ComboBox.AddString(m_Flights[a].Carrier);

	if (IncludeDatabase)
		for (UINT a=0; a<CARRIERCOUNT; a++)
			if (ComboBox.FindStringExact(-1, Carriers[a])==CB_ERR)
				ComboBox.AddString(Carriers[a]);
}

void CItinerary::PrepareEquipmentCtrl(CComboBox& ComboBox, BOOL IncludeDatabase) const
{
	for (UINT a=0; a<m_Flights.m_ItemCount; a++)
		if (m_Flights[a].Equipment[0] && (ComboBox.FindStringExact(-1, m_Flights[a].Equipment)==CB_ERR))
			ComboBox.AddString(m_Flights[a].Equipment);

	if (IncludeDatabase)
		for (UINT a=0; a<EQUIPMENTCOUNT; a++)
			if (ComboBox.FindStringExact(-1, Equipment[a])==CB_ERR)
				ComboBox.AddString(Equipment[a]);
}
