
// CCalendarFile.cpp: Implementierung der Klasse CCalendarFile
//

#pragma once
#include "stdafx.h"
#include "CCalendarFile.h"


/*void CookAttributeString(CString& tmpStr)
{
	tmpStr.Replace(_T(":"), _T("_"));
	tmpStr.Replace(_T(";"), _T("_"));
}*/


// CCalendarFile
//

CCalendarFile::CCalendarFile()
{
	m_IsOpen = FALSE;
}

BOOL CCalendarFile::Open(LPCTSTR lpszFileName, LPCTSTR lpszComment, LPCTSTR lpszDescription)
{
	if (m_IsOpen)
		return FALSE;

	m_IsOpen = CStdioFile::Open(lpszFileName, CFile::modeCreate | CFile::modeWrite);
	if (m_IsOpen)
	{
		CString Version;
		GetFileVersion(AfxGetInstanceHandle(), &Version);

		WriteString(_T("BEGIN:VCALENDAR\n"));
		WriteString(_T("PRODID:-//liquidFOLDERS//Flightmap ")+Version);
		WriteString(_T("//EN\nVERSION:2.0\n"));
		WriteString(_T("METHOD:PUBLISH\n"));

		if (lpszComment)
			if (*lpszComment!=L'\0')
			{
				WriteString(_T("COMMENT:"));
				WriteString(lpszComment);
				WriteString(_T("\n"));
			}

		if (lpszDescription)
			if (*lpszDescription!=L'\0')
			{
				WriteString(_T("DESCRIPTION:"));
				WriteString(lpszDescription);
				WriteString(_T("\n"));
			}
	}

	return m_IsOpen;
}

void CCalendarFile::WriteRoute(AIRX_Flight& Flight)
{
	WriteString(_T("BEGIN:VEVENT\n"));

	FMAirport* pAirport;
	if (FMIATAGetAirportByCode(Flight.From.Code, &pAirport))
	{
		CString Name(pAirport->Name);
		CString Country(FMIATAGetCountry(pAirport->CountryID)->Name);

		WriteString(_T("LOCATION:")+Name);
		WriteString(_T(", ")+Country);
		WriteString(_T("\n"));
	}

	CString Time;
	SYSTEMTIME st;
	if ((Flight.From.Time.dwHighDateTime!=0) || (Flight.From.Time.dwLowDateTime!=0))
	{
		FileTimeToSystemTime(&Flight.From.Time, &st);
		Time.Format(_T("%04d%02d%02dT%02d%02d00"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

		WriteString(_T("DTSTART:")+Time);
		WriteString(_T("\n"));
	}

	if ((Flight.To.Time.dwHighDateTime!=0) || (Flight.To.Time.dwLowDateTime!=0))
	{
		FileTimeToSystemTime(&Flight.To.Time, &st);
		Time.Format(_T("%04d%02d%02dT%02d%02d00"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

		WriteString(_T("DTEND:")+Time);
		WriteString(_T("\n"));
	}

	if ((Flight.From.Code[0]!='\0') && (Flight.To.Code[0]!='\0'))
	{
		CString tmpStr;

		WriteString(_T("SUMMARY:"));

		tmpStr = Flight.From.Code;
		WriteString(tmpStr);
		WriteString(_T("-"));
		tmpStr = Flight.To.Code;
		WriteString(tmpStr);
		WriteString(_T("\n"));
	}

	if ((Flight.FlightNo[0]!='\0') || (Flight.Comments[0]!=L'\0'))
	{
		WriteString(_T("DESCRIPTION:"));

		if (Flight.FlightNo[0]!='\0')
		{
			CString tmpStr(Flight.FlightNo);
			WriteString(tmpStr);
			if (Flight.Comments[0]!=L'\0')
				WriteString(_T(", "));
		}

		if (Flight.Comments[0]!=L'\0')
			WriteString(Flight.Comments);

		WriteString(_T("\n"));
	}

	WriteString(_T("CLASS:PUBLIC\nEND:VEVENT\n"));
}

void CCalendarFile::Close()
{
	if (m_IsOpen)
	{
		WriteString(_T("END:VCALENDAR"));

		CStdioFile::Close();
		m_IsOpen = FALSE;
	}
}
