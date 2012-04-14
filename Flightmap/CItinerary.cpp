
// CItinerary.cpp: Implementierung der Klasse CItinerary
//

#pragma once
#include "stdafx.h"
#include "CItinerary.h"


__forceinline UINT ReadUTF7UINT(CFile& f)
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
	UINT nCount = ReadUTF7UINT(f);
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
	ReadUTF7String(f);
}

__forceinline void ReadUTF7COLORREF(CFile& f, COLORREF& Color)
{
	ReadUTF7String(f);
}

void ReadUTF7Number(CFile& f, UINT& Number)
{
	ReadUTF7String(f);
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
			delete m_Attachments.m_Items[a].pData;
}

void CItinerary::NewSampleAtlantic()
{
	ASSERT(!m_IsOpen);

	m_DisplayName.LoadString(IDS_SAMPLEITINERARY);
	m_IsOpen = TRUE;

	AddFlight("DUS", "FRA", L"Lufthansa", L"Boeing 737", "LH 803", AIRX_Economy, "9F", "", L"", 500, (COLORREF)-1, MakeTime(2007, 1, 25, 6, 15));
	AddFlight("FRA", "JFK", L"Lufthansa", L"Airbus A340", "LH 400", AIRX_Crew, "FD", "D-AIHD", L"Stuttgart", 2565, (COLORREF)-1, MakeTime(2007, 1, 25, 9, 35));
	AddFlight("EWR", "SFO", L"Continental Airlines", L"Boeing 737", "CO 572", AIRX_Economy, "15F", "", L"", 0, 0xFFC0A0, MakeTime(2007, 1, 28, 11, 45));
	AddFlight("SFO", "MUC", L"Lufthansa", L"Airbus A340", "LH 459", AIRX_Economy, "38H", "D-AIHB", L"Bremerhaven", 5864, (COLORREF)-1, MakeTime(2007, 2, 5, 21, 50));
	AddFlight("MUC", "DUS", L"Lufthansa", L"Airbus A320", "LH 848", AIRX_Economy, "21A", "", L"", 500, (COLORREF)-1, MakeTime(2007, 2, 6, 19, 30));
}

void CItinerary::NewSamplePacific()
{
	ASSERT(!m_IsOpen);

	m_DisplayName.LoadString(IDS_SAMPLEITINERARY);
	m_IsOpen = TRUE;
}

void CItinerary::AppendAIRX(CString FileName)
{
}

void CItinerary::AppendAIR(CString FileName)
{
	CFile f;
	if (f.Open(FileName, CFile::modeRead))
	{
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

				if (ReadUTF7String(f)==_T("A"))
					Flight.Flags = AIRX_AwardFlight;

				ReadUTF7Number(f, Flight.MilesAward);
				ReadUTF7Number(f, Flight.MilesStatus);
				ReadUTF7FILETIME(f, Flight.To.Time);
				ReadUTF7CHAR(f, Flight.EtixCode, 7);
				ReadUTF7WCHAR(f, Flight.Fare, 16);

				for (UINT b=0; b<15; b++)
					ReadUTF7String(f);

				m_Flights.AddItem(Flight);
			}

			f.Close();
		}
		catch(CFileException ex)
		{
			FMErrorBox(IDS_DRIVENOTREADY);
			f.Close();
		}
	}
}

void CItinerary::AppendCSV(CString FileName)
{
}

void CItinerary::ResetFlight(AIRX_Flight& Flight)
{
	ZeroMemory(&Flight, sizeof(AIRX_Flight));
	Flight.Waypoint.Latitude = Flight.Waypoint.Longitude = 0.0;
	Flight.DistanceNM = -1.0;
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

	m_Flights.AddItem(Flight);
}
