
// CItinerary.cpp: Implementierung der Klasse CItinerary
//

#include "stdafx.h"
#include "CItinerary.h"
#include "Flightmap.h"
#include <math.h>
#include <winioctl.h>


void ResetFlight(AIRX_Flight& Flight)
{
	ZeroMemory(&Flight, sizeof(AIRX_Flight));
	Flight.Waypoint.Latitude = Flight.Waypoint.Longitude = Flight.DistanceNM = 0.0;
	Flight.Color = (COLORREF)-1;
}

void CalcDistance(AIRX_Flight& Flight, BOOL Force)
{
	if ((Flight.Flags & AIRX_DistanceCalculated) && !Force)
		return;

	Flight.Flags |= AIRX_DistanceCalculated;
	Flight.Flags &= ~(AIRX_DistanceValid | AIRX_UnknownFrom | AIRX_UnknownTo);

	FMAirport* pFrom;
	if (!FMIATAGetAirportByCode(Flight.From.Code, &pFrom))
		Flight.Flags |= AIRX_UnknownFrom;

	FMAirport* pTo;
	if (!FMIATAGetAirportByCode(Flight.To.Code, &pTo))
		Flight.Flags |= AIRX_UnknownTo;

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
}

void CalcFuture(AIRX_Flight& Flight, SYSTEMTIME* pTime)
{
	Flight.Flags &= ~AIRX_FutureFlight;
	if ((Flight.From.Time.dwLowDateTime==0) && (Flight.From.Time.dwHighDateTime==0))
		return;

	SYSTEMTIME stNow;
	if (pTime)
	{
		stNow = *pTime;
	}
	else
	{
		GetLocalTime(&stNow);
	}

	SYSTEMTIME stFlight;
	FileTimeToSystemTime(&Flight.From.Time, &stFlight);

	if ((stFlight.wYear>stNow.wYear) || ((stFlight.wYear==stNow.wYear) && (stFlight.wMonth>stNow.wMonth)) || ((stFlight.wYear==stNow.wYear) && (stFlight.wMonth==stNow.wMonth) && (stFlight.wDay>stNow.wDay)))
		Flight.Flags |= AIRX_FutureFlight;
}

void PrepareEditCtrl(CMFCMaskedEdit* pEdit, UINT Attr, AIRX_Flight* pFlight)
{
	ASSERT(pEdit);

	CString tmpStr;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeAnsiString:
		switch (Attr)
		{
		case 0:
		case 3:
			pEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
			break;

		case 2:
		case 5:
		case 8:
		case 16:
			pEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
			break;

		case 9:
			pEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "));
			break;

		case 11:
			pEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789- "));
			break;

		case 14:
			pEdit->SetValidChars(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/"));
			break;
		}

	case FMTypeUnicodeString:
		pEdit->SetLimitText(FMAttributes[Attr].DataParameter);

		break;

	case FMTypeUINT:
		pEdit->SetLimitText(6);
		pEdit->SetValidChars(_T("0123456789"));

		break;

	case FMTypeClass:
		pEdit->SetLimitText(2);
		pEdit->SetValidChars(_T("CFHJYcfhjy+"));

		break;

	case FMTypeDateTime:
		ENSURE(tmpStr.LoadString(IDS_CUEBANNER_DATETIME));

		pEdit->SetCueBanner(tmpStr);
		pEdit->SetLimitText(16);
		pEdit->SetValidChars(_T("0123456789:-. "));

		break;

	case FMTypeTime:
		ENSURE(tmpStr.LoadString(IDS_CUEBANNER_TIME));

		pEdit->SetCueBanner(tmpStr);
		pEdit->SetLimitText(5);
		pEdit->SetValidChars(_T("0123456789:"));

		break;
	}

	if (pFlight)
	{
		WCHAR tmpBuf[256];
		AttributeToString(*pFlight, Attr, tmpBuf, 256);

		pEdit->SetWindowText(tmpBuf);
	}
}

void PrepareCarrierCtrl(CComboBox* pComboBox, CItinerary* pItinerary, BOOL IncludeDatabase)
{
	if (IncludeDatabase)
		for (UINT a=0; a<CARRIERCOUNT; a++)
			pComboBox->AddString(Carriers[a]);

	if (pItinerary)
		for (UINT a=0; a<pItinerary->m_Flights.m_ItemCount; a++)
			if (pItinerary->m_Flights[a].Carrier[0])
				if (pComboBox->FindStringExact(-1, pItinerary->m_Flights[a].Carrier)==CB_ERR)
					pComboBox->AddString(pItinerary->m_Flights[a].Carrier);
}

void PrepareEquipmentCtrl(CComboBox* pComboBox, CItinerary* pItinerary, BOOL IncludeDatabase)
{
	if (IncludeDatabase)
		for (UINT a=0; a<EQUIPMENTCOUNT; a++)
			pComboBox->AddString(Equipment[a]);

	if (pItinerary)
		for (UINT a=0; a<pItinerary->m_Flights.m_ItemCount; a++)
			if (pItinerary->m_Flights[a].Equipment[0])
				if (pComboBox->FindStringExact(-1, pItinerary->m_Flights[a].Equipment)==CB_ERR)
					pComboBox->AddString(pItinerary->m_Flights[a].Equipment);
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

			StringToAttribute(tmpStr.GetBuffer(), *pFlight, Attr);
		}
	}
	else
	{
		PrepareEditCtrl(&rControl, Attr, pFlight);
	}
}

CString ExportAttribute(AIRX_Flight& Flight, UINT Attr, BOOL Label=TRUE, BOOL Colon=TRUE, BOOL NewLine=TRUE)
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

CString ExportLocation(AIRX_Flight& Flight, UINT AttrBase)
{
	CString DateTime = ExportAttribute(Flight, AttrBase+1, FALSE, FALSE, FALSE);
	CString Gate = ExportAttribute(Flight, AttrBase+2, TRUE, FALSE, FALSE);

	CString tmpStr =ExportAttribute(Flight, AttrBase, TRUE, TRUE, FALSE);

	if ((!DateTime.IsEmpty()) || (!Gate.IsEmpty()))
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

void CompressFile(HANDLE hFile, WCHAR cDrive, BOOL bCompress)
{
	BY_HANDLE_FILE_INFORMATION FileInformation;
	if (GetFileInformationByHandle(hFile, &FileInformation))
		if ((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)==0)
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


// ToString
//

void MilesToString(CString &tmpStr, LONG AwardMiles, LONG StatusMiles)
{
	tmpStr.Format(IDS_MILES, AwardMiles, StatusMiles);
}

void DistanceToString(LPWSTR pStr, SIZE_T cCount, DOUBLE DistanceNM)
{
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
	swprintf_s(pStr, cCount, L"%02d:%02d", Time/60, Time%60);
}

void DateTimeToString(LPWSTR pStr, SIZE_T cCount, FILETIME ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);

	if ((st.wHour!=0) || (st.wMinute!=0))
	{
		swprintf_s(pStr, cCount, L"%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	}
	else
	{
		swprintf_s(pStr, cCount, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
	}
}

void RouteToString(LPWSTR pStr, SIZE_T cCount, AIRX_Route& Route)
{
	if (Route.DistanceNM==-1)
	{
		wcscpy_s(pStr, cCount, _T("—"));
		return;
	}

	CString From(Route.From);
	CString To(Route.To);

	WCHAR tmpBuf[256];
	DistanceToString(tmpBuf, 256, Route.DistanceNM);

	swprintf_s(pStr, cCount, L"%s–%s, %s", From.GetBuffer(), To.GetBuffer(), tmpBuf);
}

void AttributeToString(AIRX_Flight& Flight, UINT Attr, LPWSTR pStr, SIZE_T cCount)
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
		if ((((FILETIME*)pData)->dwHighDateTime!=0) || (((FILETIME*)pData)->dwLowDateTime!=0))
			DateTimeToString(pStr, cCount, *((FILETIME*)pData));

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

__forceinline void ScanUINT(LPCWSTR Str, UINT& num)
{
	swscanf_s(Str, L"%u", &num);
}

void ScanDateTime(LPCWSTR pStr, FILETIME& ft)
{
	ASSERT(pStr);

	UINT Year;
	UINT Month;
	UINT Day;
	UINT Hour;
	UINT Minute;
	SYSTEMTIME st;

	INT c = swscanf_s(pStr, L"%u-%u-%u %u:%u", &Year, &Month, &Day, &Hour, &Minute);
	if (c>=3)
	{
		ZeroMemory(&st, sizeof(st));

		st.wYear = (WORD)Year;
		st.wMonth = (WORD)Month;
		st.wDay = (WORD)Day;

		if (c==5)
		{
			st.wHour = (WORD)Hour;
			st.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&st, &ft);
		return;
	}

	c = swscanf_s(pStr, L"%u/%u/%u %u:%u", &Month, &Day, &Year, &Hour, &Minute);
	if (c>=3)
	{
		ZeroMemory(&st, sizeof(st));

		st.wYear = (WORD)Year;
		st.wMonth = (WORD)Month;
		st.wDay = (WORD)Day;

		if (c==5)
		{
			st.wHour = (WORD)Hour;
			st.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&st, &ft);
		return;
	}

	c = swscanf_s(pStr, L"%u.%u.%u %u:%u", &Day, &Month, &Year, &Hour, &Minute);
	if (c>=3)
	{
		ZeroMemory(&st, sizeof(st));

		st.wYear = (WORD)Year;
		st.wMonth = (WORD)Month;
		st.wDay = (WORD)Day;

		if (c==5)
		{
			st.wHour = (WORD)Hour;
			st.wMinute = (WORD)Minute;
		}

		SystemTimeToFileTime(&st, &ft);
		return;
	}

	ft.dwHighDateTime = ft.dwLowDateTime = 0;
}

void ScanTime(LPCWSTR pStr, UINT& Time)
{
	ASSERT(pStr);

	UINT Hour;
	UINT Minute;

	INT c = swscanf_s(pStr, L"%u:%u", &Hour, &Minute);
	if (c>=1)
	{
		Time = Hour*60;

		if (c==2)
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

void StringToAttribute(LPWSTR pStr, AIRX_Flight& Flight, UINT Attr)
{
	ASSERT(Attr<FMAttributeCount);
	ASSERT(pStr);

	while ((*pStr!=L'\0') && (*pStr<=L' '))
		pStr++;

	WCHAR* Ptr = pStr;
	WCHAR* pEnd = pStr;

	while (*Ptr!=L'\0')
	{
		if (*Ptr!=L' ')
			pEnd = Ptr+1;

		Ptr++;
	}

	if (*pEnd)
		*pEnd = L'\0';

	const LPVOID pData = (((LPBYTE)&Flight)+FMAttributes[Attr].Offset);
	WCHAR* pWChar;
	CHAR* pChar;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeUnicodeString:
		wcsncpy_s((WCHAR*)pData, FMAttributes[Attr].DataParameter, (LPCWSTR)pStr, _TRUNCATE);

		if ((Attr==2) || (Attr==5) || (Attr==9))
		{
			pWChar = (LPWSTR)pData;

			while (*pWChar)
			{
				*pWChar = (WCHAR)towupper(*pWChar);
				pWChar++;
			}
		}

		break;

	case FMTypeAnsiString:
		pChar = (CHAR*)pData;
		WideCharToMultiByte(CP_ACP, 0, pStr, -1, pChar, FMAttributes[Attr].DataParameter, NULL, NULL);

		while (*pChar)
			*(pChar++) = (CHAR)toupper(*pChar);

		break;

	case FMTypeUINT:
		ScanUINT(pStr, *((UINT*)pData));
		break;

	case FMTypeFlags:
		Flight.Flags &= ~(AIRX_AwardFlight | AIRX_GroundTransportation | AIRX_BusinessTrip | AIRX_LeisureTrip);

		if (wcschr(pStr, L'A'))
			Flight.Flags |= AIRX_AwardFlight;

		if (wcschr(pStr, L'G'))
			Flight.Flags |= AIRX_GroundTransportation;

		if (wcschr(pStr, L'B'))
			Flight.Flags |= AIRX_BusinessTrip;

		if (wcschr(pStr, L'L'))
			Flight.Flags |= AIRX_LeisureTrip;

		if (wcschr(pStr, L'U'))
			Flight.Flags |= AIRX_Upgrade;

		if (wcschr(pStr, L'C'))
			Flight.Flags |= AIRX_Cancelled;

		break;

	case FMTypeDateTime:
		ScanDateTime(pStr, *((FILETIME*)pData));
		break;

	case FMTypeTime:
		ScanTime(pStr, *((UINT*)pData));
		break;

	case FMTypeClass:
		*((CHAR*)pData) = (_wcsicmp(L"Y", pStr)==0) ? AIRX_Economy : (_wcsicmp(L"Y+", pStr)==0) ? AIRX_PremiumEconomy : (_wcsicmp(L"J", pStr)==0) ? AIRX_Business : (_wcsicmp(L"F", pStr)==0) ? AIRX_First : ((_wcsicmp(L"C", pStr)==0) || (_wcsicmp(L"CREW", pStr)==0) || (_wcsicmp(L"CREW/DCM", pStr)==0)) ? AIRX_Crew : ((_wcsicmp(L"H", pStr)==0) || (_wcsicmp(L"CHARTER", pStr)==0)) ? AIRX_Charter : AIRX_Unknown;
		break;

	case FMTypeColor:
		ScanColor(pStr, *((COLORREF*)pData));
		break;
	}
}


// Other
//

BOOL Tokenize(const CString& strSrc, CString& strDst, INT& Pos, const CString& Delimiter, LPWSTR pDelimiterFound)
{
	if (Pos>=strSrc.GetLength())
		return FALSE;

	strDst = _T("");

	while (Pos<strSrc.GetLength())
	{
		if (Delimiter.Find(strSrc[Pos])!=-1)
		{
			if (pDelimiterFound)
				*pDelimiterFound = strSrc[Pos];

			Pos++;

			return TRUE;
		}

		strDst.AppendChar(strSrc[Pos++]);
	}

	if (pDelimiterFound)
		*pDelimiterFound = L'\0';

	return TRUE;
}

__forceinline UINT ReadUTF7Length(CFile& f)
{
	UINT Result = 0;
	UINT Shift = 0;
	BYTE b;

	do
	{
		if (f.Read(&b, 1)!=1)
			return 0;

		Result |= (b & 0x7F) << Shift;
		Shift += 7;
	}
	while (b & 0x80);

	return Result;
}

CString ReadUTF7String(CFile& f)
{
	UINT nCount = ReadUTF7Length(f);
	if (!nCount)
		return _T("");

	LPBYTE Buffer = new BYTE[nCount];
	CString Result;

	try
	{
		if (f.Read(Buffer, nCount)!=nCount)
			goto Finish;
	}
	catch(CFileException ex)
	{
		goto Finish;
	}

	WCHAR Ch = L'\0';
	UINT Shift = 0;
	UINT Ptr = 0;

	while (nCount)
	{
		Ch |= (Buffer[Ptr] & 0x7F) << Shift;

		if ((Buffer[Ptr++] & 0x80)==0)
		{
			Result.AppendChar(Ch);
			Ch = L'\0';
			Shift = 0;
		}
		else
		{
			Shift += 7;
		}

		nCount --;
	}

Finish:
	free(Buffer);
	return Result;
}

__forceinline void ReadUTF7WCHAR(CFile& f, LPWSTR pStr, SIZE_T cCount)
{
	wcscpy_s(pStr, cCount, ReadUTF7String(f));
}

__forceinline void ReadUTF7CHAR(CFile& f, LPSTR pStr, SIZE_T cCount)
{
	WideCharToMultiByte(CP_ACP, 0, ReadUTF7String(f), -1, pStr, (INT)cCount, NULL, NULL);
}

void ReadUTF7FILETIME(CFile& f, FILETIME& Time)
{
	ScanDateTime(ReadUTF7String(f), Time);
}

__forceinline void ReadUTF7COLORREF(CFile& f, COLORREF& Color)
{
	ScanColor(ReadUTF7String(f), Color);
}

void ReadUTF7UINT(CFile& f, UINT& Number)
{
	ScanUINT(ReadUTF7String(f), Number);
}


BOOL ReadRecord(CFile& f, LPVOID buf, UINT BufferSize, UINT OnDiscSize)
{
	UINT Read = BufferSize ? f.Read(buf, min(BufferSize, OnDiscSize)) : 0;

	if (OnDiscSize>BufferSize)
		f.Seek(OnDiscSize-BufferSize, CFile::current);

	return Read==min(BufferSize, OnDiscSize);
}


// CItinerary
//

#define AttachmentEndBuffer     1

CItinerary::CItinerary(const CString& Filename)
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
	if (!Filename.IsEmpty())
	{
		CString Ext = Filename;
		Ext.Delete(0, Ext.ReverseFind(L'\\')+1);
		Ext.Delete(0, Ext.ReverseFind(L'.')+1);
		Ext.MakeLower();

		if (Ext==_T("airx"))
			OpenAIRX(Filename);

		if (Ext==_T("air"))
			OpenAIR(Filename);

		if (Ext==_T("csv"))
			OpenCSV(Filename);
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

void CItinerary::OpenAIRX(const CString& Filename)
{
	ASSERT(!m_IsOpen);

	LPVOID pData = NULL;

	CFile f;
	if (f.Open(Filename, CFile::modeRead | CFile::osSequentialScan))
	{
		m_FileName = Filename;
		SetDisplayName(Filename);
		theApp.AddToRecentFiles(Filename);
		SHAddToRecentDocs(SHARD_PATHW, Filename);

		SYSTEMTIME st;
		GetLocalTime(&st);

		try
		{
			AIRX_Header Header;
			ZeroMemory(&Header, sizeof(Header));

			if (f.Read(&Header, sizeof(Header))==sizeof(Header))
				if (Header.Magic==0x58524941)
				{
					ReadRecord(f, &m_Metadata, sizeof(m_Metadata), Header.MetadataRecordSize);

					for (UINT a=0; a<Header.FlightCount; a++)
					{
						AIRX_Flight Flight;
						ResetFlight(Flight);

						if (ReadRecord(f, &Flight, sizeof(Flight), Header.FlightRecordSize))
						{
							CalcDistance(Flight, TRUE);
							CalcFuture(Flight, &st);

							if (Flight.AttachmentCount>AIRX_MaxAttachmentCount)
								Flight.AttachmentCount = AIRX_MaxAttachmentCount;

							m_Flights.AddItem(Flight);
						}
					}

					if (m_Metadata.CurrentRow>m_Flights.m_ItemCount)
						m_Metadata.CurrentRow = m_Flights.m_ItemCount;

					for (UINT a=0; a<Header.AttachmentCount; a++)
					{
						AIRX_Attachment Attachment;
						ZeroMemory(&Attachment, sizeof(Attachment));

						if (ReadRecord(f, &Attachment, sizeof(Attachment), Header.AttachmentRecordSize))
						{
							Attachment.IconID = -1;

							if (pData)
								free(pData);

							pData = malloc(Attachment.Size+AttachmentEndBuffer);
							if (pData)
								if (f.Read(pData, Attachment.Size)==Attachment.Size)
								{
									Attachment.pData = pData;
									pData = NULL;

									ValidateAttachment(Attachment, TRUE);

									m_Attachments.AddItem(Attachment);
								}
						}
					}
				}
				else
				{
					FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
				}

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}

	if (pData)
		free(pData);
}

void CItinerary::OpenAIR(const CString& Filename)
{
	ASSERT(!m_IsOpen);

	CFile f;
	if (f.Open(Filename, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(Filename);
		theApp.AddToRecentFiles(Filename);
		SHAddToRecentDocs(SHARD_PATHW, Filename);

		SYSTEMTIME st;
		GetLocalTime(&st);

		try
		{
			UINT Count = 0;
			f.Read(&Count, sizeof(Count));

			if ((ULONGLONG)Count*30>f.GetLength())
			{
				FMErrorBox(CWnd::GetActiveWindow(), IDS_ILLEGALFORMAT);
			}
			else
			{
				for (UINT a=0; a<Count; a++)
				{
					AIRX_Flight Flight;
					ResetFlight(Flight);

					ReadUTF7CHAR(f, Flight.From.Code, 4);
					ReadUTF7CHAR(f, Flight.To.Code, 4);
					ReadUTF7WCHAR(f, Flight.Carrier, 64);
					ReadUTF7WCHAR(f, Flight.Equipment, 64);
					ReadUTF7CHAR(f, Flight.FlightNo, 8);

					CString Class = ReadUTF7String(f);
					Flight.Class = (Class==_T("Y")) ? AIRX_Economy : (Class==_T("Y+")) ? AIRX_PremiumEconomy : (Class==_T("J")) ? AIRX_Business : (Class==_T("F")) ? AIRX_First : (Class==_T("C")) ? AIRX_Crew : AIRX_Unknown;

					ReadUTF7FILETIME(f, Flight.From.Time);
					ReadUTF7COLORREF(f, Flight.Color);
					ReadUTF7CHAR(f, Flight.Seat, 4);
					ReadUTF7CHAR(f, Flight.Registration, 16);
					ReadUTF7WCHAR(f, Flight.Name, 64);

					if (ReadUTF7String(f).MakeUpper()==_T("A"))
						Flight.Flags = AIRX_AwardFlight;

					ReadUTF7UINT(f, Flight.MilesAward);
					ReadUTF7UINT(f, Flight.MilesStatus);
					ReadUTF7FILETIME(f, Flight.To.Time);
					ReadUTF7CHAR(f, Flight.EtixCode, 7);
					ReadUTF7UINT(f, Flight.Fare);

					for (UINT b=0; b<15; b++)
						ReadUTF7String(f);

					CalcDistance(Flight, TRUE);
					CalcFuture(Flight, &st);

					m_Flights.AddItem(Flight);
				}
			}

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}

void CItinerary::OpenCSV(const CString& Filename)
{
	ASSERT(!m_IsOpen);

	CStdioFile f;
	if (f.Open(Filename, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(Filename);
		theApp.AddToRecentFiles(Filename);
		SHAddToRecentDocs(SHARD_PATHW, Filename);

		SYSTEMTIME st;
		GetLocalTime(&st);

		WCHAR Separator[4];
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, Separator, 4);

		try
		{
			CString Caption;
			if (f.ReadString(Caption))
			{
				CMap<CString, LPCTSTR, INT, INT&> map;
				map[L"FROM"] = 0;
				map[L"FRM IATA"] = 0;
				map[L"FROM IATA"] = 0;
				map[L"ORT VON"] = 0;
				map[L"START"] = 0;
				map[L"DATE & TIME"] = 1;
				map[L"DATE"] = 1;
				map[L"DATUM"] = 1;
				map[L"DEPARTURE"] = 1;
				map[L"DEPT. TIME"] = 1;
				map[L"ZEIT VON"] = 1;
				map[L"DEPT. GATE"] = 2;
				map[L"TO"] = 3;
				map[L"TO IATA"] = 3;
				map[L"ORT BIS"] = 3;
				map[L"ZIEL"] = 3;
				map[L"ARRIVAL"] = 4;
				map[L"ARR. TIME"] = 4;
				map[L"ZEIT BIS"] = 4;
				map[L"ARR. GATE"] = 5;
				map[L"AIRLINE"] = 7;
				map[L"CARRIER"] = 7;
				map[L"OPERATOR"] = 7;
				map[L"FLIGHT"] = 8;
				map[L"FLIGHT #"] = 8;
				map[L"FLIGHT_NUMBER"] = 8;
				map[L"FLUGNR"] = 8;
				map[L"CODESHARE"] = 9;
				map[L"CODESHARES"] = 9;
				map[L"EQUIPMENT"] = 10;
				map[L"MUSTER"] = 10;
				map[L"PLANE"] = 10;
				map[L"TYPE"] = 10;
				map[L"AIRCRAFT REGISTRATION"] = 11;
				map[L"REGISTRATION"] = 11;
				map[L"TAIL"] = 11;
				map[L"AIRCRAFT NAME"] = 12;
				map[L"CLASS"] = 13;
				map[L"KLASSE"] = 13;
				map[L"SEAT"] = 14;
				map[L"SITZ"] = 14;
				map[L"COLOR"] = 15;
				map[L"ETIX"] = 16;
				map[L"BOOKING"] = 16;
				map[L"ETIX CODE"] = 16;
				map[L"ETIX BOOKING CODE"] = 16;
				map[L"FARE"] = 17;
				map[L"PRICE"] = 17;
				map[L"AWARD MILES"] = 18;
				map[L"STATUS MILES"] = 19;
				map[L"FLAGS"] = 20;
				map[L"RATING"] = 21;
				map[L"COMMENTS"] = 22;
				map[L"FLIGHT TIME"] = 23;
				map[L"VOUCHER"] = 24;
				map[L"EVOUCHER"] = 24;
				map[L"UPGRADE VOUCHER"] = 24;

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
						map.Lookup(resToken, Mapping[Column]);
					}

					Column++;
				}

				// Flights
				CString Line;
				CString Route;
				while (f.ReadString(Line))
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
								StringToAttribute(resToken.GetBuffer(), Flight, Mapping[Column]);
							}

						Column++;
					}

					if (Route.IsEmpty())
					{
						CalcDistance(Flight, TRUE);
						CalcFuture(Flight);

						m_Flights.AddItem(Flight);
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

							if ((!From.IsEmpty()) && (!To.IsEmpty()))
							{
								StringToAttribute(From.GetBuffer(), Flight, 0);
								StringToAttribute(To.GetBuffer(), Flight, 3);

								CalcDistance(Flight, TRUE);
								CalcFuture(Flight, &st);

								m_Flights.AddItem(Flight);
							}

							if (DelimiterFound==L',')
								To.Empty();
						}
					}
				}
			}

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}

void CItinerary::SaveAIRX(CString Filename)
{
	CFile f;
	if (f.Open(Filename, CFile::modeCreate | CFile::modeReadWrite | CFile::osSequentialScan))
	{
		// Compression
		BOOL bCompress = TRUE;
		for (UINT a=0; a<m_Attachments.m_ItemCount; a++)
		{
			CString Ext(m_Attachments[a].Name);
			INT Pos = Ext.ReverseFind(L'.');
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

		CompressFile(f.m_hFile, Filename[0], bCompress);

		// Write file
		m_FileName = Filename;
		SetDisplayName(Filename);
		theApp.AddToRecentFiles(Filename);
		SHAddToRecentDocs(SHARD_PATHW, Filename);

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

			f.Write(&Header, sizeof(Header));
			f.Write(&m_Metadata, sizeof(m_Metadata));
			f.Write(&m_Flights[0], Header.FlightCount*Header.FlightRecordSize);

			for (UINT a=0; a<Header.AttachmentCount; a++)
			{
				AIRX_Attachment Attachment = m_Attachments[a];

				if (!Attachment.pData)
					Attachment.Size = 0;
				Attachment.pData = NULL;
				Attachment.IconID = -1;

				f.Write(&Attachment, sizeof(Attachment));
				if ((m_Attachments[a].pData) && (Attachment.Size!=0))
					f.Write(m_Attachments[a].pData, Attachment.Size);
			}

			f.Close();

			m_IsModified = FALSE;
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}
}

CString CItinerary::Flight2Text(AIRX_Flight& Flight)
{
	CString tmpStr;

	tmpStr += ExportLocation(Flight, 0);
	tmpStr += ExportLocation(Flight, 3);

	const UINT Attrs[15] = { 6, 23, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 19, 22 };
	for (UINT a=0; a<15; a++)
		tmpStr += ExportAttribute(Flight, Attrs[a]);

	if (!tmpStr.IsEmpty())
		tmpStr += _T("\n");

	return tmpStr;
}

CString CItinerary::Flight2Text(UINT Index)
{
	ASSERT(Index<m_Flights.m_ItemCount);

	return Flight2Text(m_Flights[Index]);
}

FILETIME CItinerary::MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute)
{
	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(st));

	st.wYear = wYear;
	st.wMonth = wMonth;
	st.wDay = wDay;
	st.wHour = wHour;
	st.wMinute = wMinute;

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	return ft;
}

INT CItinerary::Compare(AIRX_Flight* Eins, AIRX_Flight* Zwei, const UINT Attr, const BOOL Descending)
{
	ASSERT(Attr<FMAttributeCount);
	ASSERT(FMAttributes[Attr].Sortable);

	const void* pValue1 = (LPBYTE)Eins+FMAttributes[Attr].Offset;
	const void* pValue2 = (LPBYTE)Zwei+FMAttributes[Attr].Offset;

	// Gewünschtes Attribut vergleichen
	INT Compare = 0;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeUnicodeString:
		Compare = _wcsicmp((LPCWSTR)pValue1, (LPCWSTR)pValue2);
		break;

	case FMTypeAnsiString:
		Compare = _stricmp((LPCSTR)pValue1, (LPCSTR)pValue2);
		break;

	case FMTypeRating:
		Compare = (INT)((*(UINT*)pValue1)>>FMAttributes[Attr].DataParameter)-(INT)((*(UINT*)pValue2)>>FMAttributes[Attr].DataParameter);

	case FMTypeUINT:
	case FMTypeTime:
		Compare = *(UINT*)pValue1==*(UINT*)pValue2 ? 0 : *(UINT*)pValue1<*(UINT*)pValue2 ? -1 : 1;
		break;

	case FMTypeDistance:
		Compare = *(DOUBLE*)pValue1==*(DOUBLE*)pValue2 ? 0 : *(DOUBLE*)pValue1<*(DOUBLE*)pValue2 ? -1 : 1;
		break;

	case FMTypeDateTime:
		Compare = CompareFileTime((FILETIME*)pValue1, (FILETIME*)pValue2);
		break;

	case FMTypeClass:
		Compare = (INT)(*(UCHAR*)pValue1)-(INT)(*(UCHAR*)pValue2);
		break;

	default:
		ASSERT(FALSE);
	}

	// Ggf. Reihenfolge umkehren
	return Descending ? -Compare : Compare;
}

void CItinerary::Heap(UINT Wurzel, const UINT Anzahl, const UINT Attr, const BOOL Descending)
{
	AIRX_Flight Flight = m_Flights[Wurzel];
	UINT Parent = Wurzel;
	UINT Child;

	while ((Child=(Parent+1)*2)<Anzahl)
	{
		if (Compare(&m_Flights[Child-1], &m_Flights[Child], Attr, Descending)>0)
			Child--;

		m_Flights[Parent] = m_Flights[Child];
		Parent = Child;
	}

	if (Child==Anzahl)
	{
		if (Compare(&m_Flights[--Child], &Flight, Attr, Descending)>=0)
		{
			m_Flights[Parent] = m_Flights[Child];
			m_Flights[Child] = Flight;

			return;
		}

		Child = Parent;
	}
	else
	{
		if (Parent==Wurzel)
			return;

		if (Compare(&m_Flights[Parent], &Flight, Attr, Descending)>=0)
		{
			m_Flights[Parent] = Flight;

			return;
		}

		Child = (Parent-1)/2;
	}

	while (Child!=Wurzel)
	{
		Parent = (Child-1)/2;

		if (Compare(&m_Flights[Parent], &Flight, Attr, Descending)>=0)
			break;

		m_Flights[Child] = m_Flights[Parent];
		Child = Parent;
	}

	m_Flights[Child] = Flight;
}

void CItinerary::Sort(UINT Attr, BOOL Descending)
{
	if (m_Flights.m_ItemCount>1)
	{
		for (INT a=m_Flights.m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_Flights.m_ItemCount, Attr, Descending);

		for (INT a=m_Flights.m_ItemCount-1; a>0; a--)
		{
			Swap(m_Flights[0], m_Flights[a]);
			Heap(0, a, Attr, Descending);
		}
	}
}

AIRX_Attachment* CItinerary::GetGPSPath(AIRX_Flight& Flight)
{
	for (UINT a=0; a<Flight.AttachmentCount; a++)
	{
		AIRX_Attachment* pAttachment = &m_Attachments[Flight.Attachments[a]];

		LPCWSTR pExtension = wcsrchr(pAttachment->Name, L'.');
		if (pExtension)
			if (_wcsicmp(pExtension, L".gpx")==0)
			{
				ValidateAttachment(*pAttachment);

				if (pAttachment->Flags & AIRX_Valid)
					return pAttachment;
			}
	}

	return NULL;
}

AIRX_Attachment* CItinerary::GetGPSPath(UINT Row)
{
	ASSERT(Row<m_Flights.m_ItemCount);

	return GetGPSPath(m_Flights[Row]);
}


void CItinerary::AddFlight()
{
	AIRX_Flight Flight;
	ResetFlight(Flight);

	m_Flights.AddItem(Flight);
}

void CItinerary::AddFlight(CItinerary* pItinerary, UINT Row)
{
	ASSERT(pItinerary);
	ASSERT(Row<pItinerary->m_Flights.m_ItemCount);

	AIRX_Flight Flight = pItinerary->m_Flights[Row];
	Flight.AttachmentCount = 0;

	for (UINT a=0; a<pItinerary->m_Flights[Row].AttachmentCount; a++)
	{
		const UINT Index = AddAttachment(pItinerary, pItinerary->m_Flights[Row].Attachments[a]);
		if (Index!=(UINT)-1)
			Flight.Attachments[Flight.AttachmentCount++] = Index;
	}

	m_Flights.AddItem(Flight);
}

void CItinerary::AddFlight(LPCSTR From, LPCSTR To, LPCWSTR Carrier, LPCWSTR Equipment, LPCSTR FlightNo, CHAR Class, LPCSTR Seat, LPCSTR Registration, LPCWSTR Name, UINT Miles, COLORREF Color, FILETIME Departure)
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

	CalcDistance(Flight);
	CalcFuture(Flight);

	m_Flights.AddItem(Flight);
}

void CItinerary::InsertFlights(UINT Row, UINT Count, AIRX_Flight* pFlights)
{
	if (m_Flights.InsertEmpty(Row, Count, false))
		if (pFlights)
		{
			const SIZE_T sz = Count*sizeof(AIRX_Flight);
			memcpy_s(&m_Flights[Row], sz, pFlights, sz);

			SYSTEMTIME st;
			GetLocalTime(&st);

			for (UINT a=Row; a<Row+Count; a++)
			{
				CalcDistance(m_Flights[a], TRUE);
				CalcFuture(m_Flights[a], &st);

				m_Flights[a].AttachmentCount = 0;
			}
		}
		else
		{
			for (UINT a=Row; a<Row+Count; a++)
				ResetFlight(m_Flights[a]);
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
			while (m_Flights[Row].AttachmentCount)
				DeleteAttachment(m_Flights[Row].Attachments[0]);

			m_Flights.DeleteItems(Row);
			m_IsModified = TRUE;
		}
		else
		{
			Row++;
		}
	}
}

void CItinerary::SetDisplayName(const CString& Filename)
{
	const LPCWSTR pChar = wcsrchr(Filename, L'\\');

	m_DisplayName = pChar ? pChar+1 : Filename;
}

BOOL CItinerary::AddAttachment(AIRX_Flight& Flight, CString Filename)
{
	if (Flight.AttachmentCount>=AIRX_MaxAttachmentCount)
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_ATTACHMENT_TOOMANY);
		return FALSE;
	}

	// FindFirst
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(Filename, &ffd);

	if (hFind==INVALID_HANDLE_VALUE)
		return FALSE;

	AIRX_Attachment Attachment;
	ZeroMemory(&Attachment, sizeof(Attachment));

	wcscpy_s(Attachment.Name, MAX_PATH, Filename.Mid(Filename.ReverseFind(L'\\')+1));
	Attachment.Size = (((INT64)ffd.nFileSizeHigh) << 32)+ffd.nFileSizeLow;
	Attachment.Created = ffd.ftCreationTime;
	Attachment.Modified = ffd.ftLastWriteTime;
	Attachment.IconID = -1;

	FindClose(hFind);

	if (Attachment.Size>1024*1024)
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_ATTACHMENT_TOOLARGE);
		return FALSE;
	}

	Attachment.pData = malloc(Attachment.Size+AttachmentEndBuffer);
	if (!Attachment.pData)
		return FALSE;

	BOOL Result = FALSE;

	// Read file
	CFile f;
	if (f.Open(Filename, CFile::modeRead | CFile::osSequentialScan))
	{
		try
		{
			f.Read(Attachment.pData, Attachment.Size);
			f.Close();

			ValidateAttachment(Attachment, TRUE);

			if (m_Attachments.AddItem(Attachment))
			{
				Flight.Attachments[Flight.AttachmentCount++] = m_Attachments.m_ItemCount-1;
				Result = m_IsModified = TRUE;
			}
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(CWnd::GetActiveWindow(), IDS_DRIVENOTREADY);
	}

	if (!Result && (Attachment.pData))
		free(Attachment.pData);

	return Result;
}

UINT CItinerary::AddAttachment(CItinerary* pItinerary, UINT Index)
{
	ASSERT(pItinerary);
	ASSERT(Index<pItinerary->m_Attachments.m_ItemCount);

	AIRX_Attachment Attachment = pItinerary->m_Attachments[Index];

	Attachment.pData = malloc(Attachment.Size+AttachmentEndBuffer);
	if (!Attachment.pData)
		return (UINT)-1;

	memcpy_s(Attachment.pData, Attachment.Size, pItinerary->m_Attachments[Index].pData, Attachment.Size);

	return m_Attachments.AddItem(Attachment) ? m_Attachments.m_ItemCount-1 : (UINT)-1;
}

Bitmap* CItinerary::DecodePictureAttachment(UINT Index) const
{
	ASSERT(Index<m_Attachments.m_ItemCount);

	return DecodePictureAttachment(m_Attachments[Index]);
}

Bitmap* CItinerary::DecodePictureAttachment(const AIRX_Attachment& Attachment)
{
	IStream* pStream = SHCreateMemStream((LPBYTE)Attachment.pData, Attachment.Size);

	Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);

	pStream->Release();

	return pBitmap;
}

CGPXFile* CItinerary::DecodeGPXAttachment(const AIRX_Attachment& Attachment)
{
	if (Attachment.Size<16)
		return NULL;

	ASSERT(_msize(Attachment.pData)>=Attachment.Size+AttachmentEndBuffer);

	// Terminating NUL
	*((CHAR*)Attachment.pData+Attachment.Size) = '\0';

	CGPXFile* pGPXFile = new CGPXFile();

	try
	{
		pGPXFile->parse<parse_non_destructive | parse_no_data_nodes>((CHAR*)Attachment.pData);
	}
	catch(parse_error e)
	{
		delete pGPXFile;
		pGPXFile = NULL;
	}

	return pGPXFile;
}

void CItinerary::ValidateAttachment(AIRX_Attachment& Attachment, BOOL Force)
{
	if (!Force)
		if (Attachment.Flags & (AIRX_Valid | AIRX_Invalid))
		{
			if (Attachment.Flags & AIRX_Invalid)
				Attachment.Flags &= ~AIRX_Valid;

			return;
		}

	Attachment.Flags &= ~(AIRX_Valid | AIRX_Invalid);

	LPCWSTR pExtension = wcsrchr(Attachment.Name, L'.');
	if (pExtension)
		if (_wcsicmp(pExtension, L".gpx")==0)
		{
			UINT Mask = AIRX_Invalid;

			CGPXFile* pGPXFile = DecodeGPXAttachment(Attachment);
			if (pGPXFile)
			{
				Mask = AIRX_Valid;
				delete pGPXFile;
			}

			Attachment.Flags |= Mask;
		}
}

void CItinerary::DeleteAttachment(UINT Index, AIRX_Flight* pFlight)
{
	ASSERT(Index<m_Attachments.m_ItemCount);

	FreeAttachment(m_Attachments[Index]);

	m_Attachments.m_ItemCount--;
	for (UINT a=Index; a<m_Attachments.m_ItemCount; a++)
		m_Attachments[a] = m_Attachments[a+1];

	for (UINT a=0; a<m_Flights.m_ItemCount; a++)
		RemoveAttachment(Index, &m_Flights[a]);

	if (pFlight)
		RemoveAttachment(Index, pFlight);

	m_IsModified = TRUE;
}

void CItinerary::DeleteAttachments(AIRX_Flight* pFlight)
{
	if (pFlight)
	{
		// Alle Attachments von einem externen Flug (der nicht zu diesem Itinerary gehört) löschen
		for (UINT a=0; a<pFlight->AttachmentCount; a++)
			DeleteAttachment(pFlight->Attachments[a]);

		pFlight->AttachmentCount = 0;
	}
	else
	{
		// Alle Attachments von diesem Itinerary löschen
		for (UINT a=0; a<m_Flights.m_ItemCount; a++)
			m_Flights[a].AttachmentCount = 0;

		for (UINT a=0; a<m_Attachments.m_ItemCount; a++)
			FreeAttachment(m_Attachments[a]);

		m_Attachments.m_ItemCount = 0;
	}

	m_IsModified = TRUE;
}

void CItinerary::RemoveAttachment(UINT Index, AIRX_Flight* pFlight)
{
	ASSERT(pFlight);

	UINT No = 0;
	while (No<pFlight->AttachmentCount)
		if (pFlight->Attachments[No]==Index)
		{
			pFlight->AttachmentCount--;

			if (No<pFlight->AttachmentCount)
				for (UINT b=No; b<pFlight->AttachmentCount; b++)
					pFlight->Attachments[b] = pFlight->Attachments[b+1];
		}
		else
		{
			if (pFlight->Attachments[No]>Index)
				pFlight->Attachments[No]--;

			No++;
		}
}
