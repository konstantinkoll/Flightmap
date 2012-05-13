
// CItinerary.cpp: Implementierung der Klasse CItinerary
//

#pragma once
#include "stdafx.h"
#include "CItinerary.h"
#include "Flightmap.h"


void CalcDistance(AIRX_Flight& Flight, BOOL Force)
{
	if ((Flight.Flags & AIRX_DistanceCalculated) && !Force)
		return;

	Flight.Flags |= AIRX_DistanceCalculated;

	FMAirport* pFrom = NULL;
	FMAirport* pTo = NULL;

	if (FMIATAGetAirportByCode(Flight.From.Code, &pFrom) && FMIATAGetAirportByCode(Flight.To.Code, &pTo))
	{
		const DOUBLE Lat1 = PI*pFrom->Location.Latitude/180;
		const DOUBLE Lon1 = PI*pFrom->Location.Longitude/180;
		const DOUBLE Lat2 = PI*pTo->Location.Latitude/180;
		const DOUBLE Lon2 = PI*pTo->Location.Longitude/180;

		const DOUBLE DeltaLon = abs(Lon1-Lon2);
		const DOUBLE tmp1 = cos(Lat1)*sin(DeltaLon);
		const DOUBLE tmp2 = cos(Lat2)*sin(Lat1)-sin(Lat2)*cos(Lat1)*cos(DeltaLon);
		const DOUBLE T = sqrt(tmp1*tmp1+tmp2*tmp2);
		const DOUBLE B = sin(Lat2)*sin(Lat1)+cos(Lat2)*cos(Lat1)*cos(DeltaLon);
		const DOUBLE GreatCircle = atan2(T, B);

		Flight.DistanceNM = 3438.461*GreatCircle;
		Flight.Flags |= AIRX_DistanceValid;
	}
	else
	{
		Flight.DistanceNM = 0.0;
		Flight.Flags &= ~AIRX_DistanceValid;
	}
}


// ToString
//

void DistanceToString(WCHAR* pBuffer, SIZE_T cCount, DOUBLE DistanceNM)
{
	if (theApp.m_UseStatuteMiles)
	{
		swprintf(pBuffer, cCount, L"%d mi (%d km)", (UINT)(DistanceNM*1.15077945), (UINT)(DistanceNM*1.852));
	}
	else
	{
		swprintf(pBuffer, cCount, L"%d nm (%d km)", (UINT)DistanceNM, (UINT)(DistanceNM*1.852));
	}
}

void AttributeToString(AIRX_Flight& Flight, UINT Attr, WCHAR* pBuffer, SIZE_T cCount)
{
	ASSERT(Attr<FMAttributeCount);
	ASSERT(pBuffer);
	ASSERT(cCount>=1);

	const LPVOID pData = (((BYTE*)&Flight)+FMAttributes[Attr].Offset);
	*pBuffer = L'\0';

	SYSTEMTIME st;

	switch (FMAttributes[Attr].Type)
	{
	case FMTypeUnicodeString:
		wcscpy_s(pBuffer, cCount, (WCHAR*)pData);
		break;
	case FMTypeAnsiString:
		MultiByteToWideChar(CP_ACP, 0, (CHAR*)pData, -1, pBuffer, (INT)cCount);
		break;
	case FMTypeUINT:
		if (*((UINT*)pData))
			swprintf(pBuffer, cCount, L"%d", *((UINT*)pData));
		break;
	case FMTypeDistance:
		if (Flight.Flags & AIRX_DistanceValid)
			DistanceToString(pBuffer, cCount, *((DOUBLE*)pData));
		break;
	case FMTypeFlags:
		if (Flight.Flags & AIRX_AwardFlight)
			wcscat_s(pBuffer, cCount, L"A");
		if (Flight.Flags & AIRX_BusinessTrip)
			wcscat_s(pBuffer, cCount, L"B");
		if (Flight.Flags & AIRX_LeisureTrip)
			wcscat_s(pBuffer, cCount, L"L");
		break;
	case FMTypeDateTime:
		if ((((FILETIME*)pData)->dwHighDateTime!=0) || (((FILETIME*)pData)->dwLowDateTime!=0))
		{
			FileTimeToSystemTime((FILETIME*)pData, &st);
			if ((st.wHour!=0) || (st.wMinute!=0))
			{
				swprintf(pBuffer, cCount, L"%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
			}
			else
			{
				swprintf(pBuffer, cCount, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
			}
		}
		break;
	case FMTypeTime:
		if (*((UINT*)pData))
			swprintf(pBuffer, cCount, L"%02d:%02d", *((UINT*)pData)/60, *((UINT*)pData)%60);
		break;
	case FMTypeClass:
		switch (*((CHAR*)pData))
		{
		case AIRX_Economy:
			wcscpy_s(pBuffer, cCount, L"Y");
			break;
		case AIRX_EconomyPlus:
			wcscpy_s(pBuffer, cCount, L"Y+");
			break;
		case AIRX_Business:
			wcscpy_s(pBuffer, cCount, L"J");
			break;
		case AIRX_First:
			wcscpy_s(pBuffer, cCount, L"F");
			break;
		case AIRX_Crew:
			wcscpy_s(pBuffer, cCount, L"Crew/DCM");
			break;
		}
		break;
	case FMTypeColor:
		if (*((COLORREF*)pData)!=(COLORREF)-1)
			swprintf(pBuffer, cCount, L"%06X", *((COLORREF*)pData));
		break;
	}
}


// FromString
//


// Other
//

void ScanDate(LPCWSTR str, FILETIME& ft)
{
	UINT Year;
	UINT Month;
	UINT Day;
	UINT Hour;
	UINT Minute;
	SYSTEMTIME st;

	INT c = swscanf_s(str, L"%u-%u-%u %u:%u", &Year, &Month, &Day, &Hour, &Minute);
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

	c = swscanf_s(str, L"%u.%u.%u %u:%u", &Day, &Month, &Year, &Hour, &Minute);
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
	}
}

void ScanColor(LPCWSTR str, COLORREF& col)
{
	if (swscanf_s(str, L"%06X", &col)==1)
		if (col!=(COLORREF)-1)
			col = (((UINT)col & 0xFF0000)>>16) | ((UINT)col & 0xFF00) | (((UINT)col & 0xFF)<<16);
}

__forceinline void ScanNumber(LPCWSTR str, UINT& num)
{
	swscanf_s(str, L"%d", &num);
}


__forceinline UINT ReadUTF7Length(CFile& f)
{
	UINT Res = 0;
	UINT Shift = 0;
	BYTE b;

	do
	{
		if (f.Read(&b, 1)!=1)
			return 0;

		Res |= (b & 0x7F) << Shift;
		Shift += 7;
	}
	while (b & 0x80);

	return Res;
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

__forceinline void ReadUTF7WCHAR(CFile& f, WCHAR* pChar, UINT cCount)
{
	wcscpy_s(pChar, cCount, ReadUTF7String(f));
}

__forceinline void ReadUTF7CHAR(CFile& f, CHAR* pChar, UINT cCount)
{
	WideCharToMultiByte(CP_ACP, 0, ReadUTF7String(f), -1, pChar, cCount, NULL, NULL);
}

void ReadUTF7FILETIME(CFile& f, FILETIME& Time)
{
	ScanDate(ReadUTF7String(f), Time);
}

__forceinline void ReadUTF7COLORREF(CFile& f, COLORREF& Color)
{
	ScanColor(ReadUTF7String(f), Color);
}

void ReadUTF7UINT(CFile& f, UINT& Number)
{
	ScanNumber(ReadUTF7String(f), Number);
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

CItinerary::CItinerary()
{
	m_IsModified = m_IsOpen = FALSE;
	m_DisplayName.LoadString(IDS_EMPTYITINERARY);
	ZeroMemory(&m_Metadata, sizeof(m_Metadata));
}

CItinerary::~CItinerary()
{
	for (UINT a=0; a<m_Attachments.m_ItemCount; a++)
		if (m_Attachments.m_Items[a].pData)
			free(m_Attachments.m_Items[a].pData);
}

void CItinerary::NewSampleAtlantic()
{
	ASSERT(!m_IsOpen);

	m_DisplayName.LoadString(IDS_SAMPLEITINERARY);
	m_IsOpen = TRUE;

	AddFlight("DUS", "FRA", L"Lufthansa", L"Boeing 737", "LH 803", AIRX_Economy, "9F", "", L"", 500, (COLORREF)-1, MakeTime(2007, 1, 25, 6, 15));
	AddFlight("FRA", "JFK", L"Lufthansa", L"Airbus A340", "LH 400", AIRX_Crew, "F/D", "D-AIHD", L"Stuttgart", 2565, (COLORREF)-1, MakeTime(2007, 1, 25, 9, 35));
	AddFlight("EWR", "SFO", L"Continental Airlines", L"Boeing 737", "CO 572", AIRX_Economy, "15F", "", L"", 0, 0xFFC0A0, MakeTime(2007, 1, 28, 11, 45));
	AddFlight("SFO", "MUC", L"Lufthansa", L"Airbus A340", "LH 459", AIRX_Economy, "38H", "D-AIHB", L"Bremerhaven", 5864, (COLORREF)-1, MakeTime(2007, 2, 5, 21, 50));
	AddFlight("MUC", "DUS", L"Lufthansa", L"Airbus A320", "LH 848", AIRX_Economy, "21A", "", L"", 500, (COLORREF)-1, MakeTime(2007, 2, 6, 19, 30));
}

void CItinerary::NewSamplePacific()
{
	ASSERT(!m_IsOpen);

	m_DisplayName.LoadString(IDS_SAMPLEITINERARY);
	m_IsOpen = TRUE;

	AddFlight("YVR", "DFW", L"American Airlines", L"Boeing 737", "AA 260", AIRX_Economy, "16A", "", L"", 1522, (COLORREF)-1, MakeTime(2012, 7, 9, 12, 15));
	AddFlight("DFW", "LAX", L"American Airlines", L"Boeing 737", "AA 2489", AIRX_Economy, "18D", "", L"", 1070, (COLORREF)-1, MakeTime(2012, 7, 9, 20, 35));
	AddFlight("LAX", "MEL", L"Quantas", L"Airbus A380", "QF 94", AIRX_Economy, "75A", "VH-OQF", L"Charles Kingsford Smith", 6886, 0x0000FF, MakeTime(2012, 7, 9, 23, 30));
	AddFlight("MEL", "LAX", L"Quantas", L"Airbus A380", "QF 93", AIRX_Economy, "88E", "VH-OQH", L"Reginald Ansett", 6886, 0x0000FF, MakeTime(2012, 7, 23, 9, 35));
	AddFlight("LAX", "DFW", L"American Airlines", L"Boeing 737", "AA 2436", AIRX_Economy, "23F", "", L"", 1070, (COLORREF)-1, MakeTime(2012, 7, 23, 10, 45));
	AddFlight("DFW", "YVR", L"American Airlines", L"Boeing 737", "AA 887", AIRX_Economy, "22B", "", L"", 1522, (COLORREF)-1, MakeTime(2012, 7, 23, 18, 25));
}

void CItinerary::OpenAIRX(CString FileName)
{
	ASSERT(!m_IsOpen);

	LPVOID pData = NULL;

	CFile f;
	if (f.Open(FileName, CFile::modeRead | CFile::osSequentialScan))
	{
		m_FileName = FileName;
		SetDisplayName(FileName);
		theApp.AddToRecentList(FileName);

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
							m_Flights.AddItem(Flight);
						}
					}

					for (UINT a=0; a<Header.AttachmentCount; a++)
					{
						AIRX_Attachment Attachment;
						ZeroMemory(&Attachment, sizeof(Attachment));

						if (ReadRecord(f, &Attachment, sizeof(Attachment), Header.AttachmentRecordSize))
						{
							if (pData)
								free(pData);

							pData = malloc(Attachment.Size);
							if (pData)
								if (f.Read(pData, Attachment.Size)==Attachment.Size)
								{
									Attachment.pData = pData;
									pData = NULL;

									m_Attachments.AddItem(Attachment);
								}
						}
					}
				}

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}

	if (pData)
		free(pData);
}

void CItinerary::OpenAIR(CString FileName)
{
	ASSERT(!m_IsOpen);

	CFile f;
	if (f.Open(FileName, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(FileName);
		theApp.AddToRecentList(FileName);

		try
		{
			UINT Count = 0;
			f.Read(&Count, sizeof(Count));

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
				Flight.Class = (Class==_T("Y")) ? AIRX_Economy : (Class==_T("Y+")) ? AIRX_EconomyPlus : (Class==_T("J")) ? AIRX_Business : (Class==_T("F")) ? AIRX_First : (Class==_T("C")) ? AIRX_Crew : AIRX_Unknown;

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
				ReadUTF7WCHAR(f, Flight.Fare, 16);

				for (UINT b=0; b<15; b++)
					ReadUTF7String(f);

				CalcDistance(Flight, TRUE);
				m_Flights.AddItem(Flight);
			}

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
}

void CItinerary::OpenCSV(CString FileName)
{
	ASSERT(!m_IsOpen);

	CFile f;
	if (f.Open(FileName, CFile::modeRead | CFile::osSequentialScan))
	{
		SetDisplayName(FileName);
		theApp.AddToRecentList(FileName);

		try
		{
			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
}

void CItinerary::SaveAIRX(CString FileName)
{
	CFile f;
	if (f.Open(FileName, CFile::modeCreate | CFile::modeWrite | CFile::osSequentialScan))
	{
		m_FileName = FileName;
		SetDisplayName(FileName);
		theApp.AddToRecentList(FileName);

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
			f.Write(&m_Flights.m_Items[0], Header.FlightCount*Header.FlightRecordSize);

			for (UINT a=0; a<Header.AttachmentCount; a++)
			{
				AIRX_Attachment Attachment = m_Attachments.m_Items[a];

				if (Attachment.pData==NULL)
					Attachment.Size = 0;
				Attachment.pData = NULL;

				f.Write(&Attachment, sizeof(Attachment));
				if ((m_Attachments.m_Items[a].pData) && (Attachment.Size!=0))
					f.Write(m_Attachments.m_Items[a].pData, Attachment.Size);
			}

			f.Close();

			m_IsModified = FALSE;
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(IDS_DRIVENOTREADY);
		}
	}
	else
	{
		FMErrorBox(IDS_DRIVENOTREADY);
	}
}

CString CItinerary::Flight2Text(AIRX_Flight& Flight)
{
	CString tmpStr;

	return tmpStr;
}

CString CItinerary::Flight2Text(UINT Idx)
{
	ASSERT(Idx<m_Flights.m_ItemCount);

	return Flight2Text(m_Flights.m_Items[Idx]);
}

void CItinerary::ResetFlight(AIRX_Flight& Flight)
{
	ZeroMemory(&Flight, sizeof(AIRX_Flight));
	Flight.Waypoint.Latitude = Flight.Waypoint.Longitude = Flight.DistanceNM = 0.0;
	Flight.Color = (COLORREF)-1;
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

void CItinerary::AddFlight()
{
	AIRX_Flight Flight;
	ResetFlight(Flight);

	m_Flights.AddItem(Flight);
}

void CItinerary::AddFlight(CHAR* From, CHAR* To, WCHAR* Carrier, WCHAR* Equipment, CHAR* FlightNo, CHAR Class, CHAR* Seat, CHAR* Registration, WCHAR* Name, UINT Miles, COLORREF Color, FILETIME Departure)
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

	m_Flights.AddItem(Flight);
}

void CItinerary::SetDisplayName(CString FileName)
{
	const WCHAR* pChar = wcsrchr(FileName, L'\\');
	m_DisplayName = pChar ? pChar+1 : FileName;
}
