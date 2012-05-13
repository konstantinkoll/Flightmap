
// CExcelFile.cpp: Implementierung der Klasse CExcelFile
//

#pragma once
#include "stdafx.h"
#include "CExcelFile.h"


// CExcelFile
//

CExcelFile::CExcelFile()
{
	m_IsOpen = FALSE;
}

BOOL CExcelFile::Open(LPCTSTR lpszFileName)
{
	if (m_IsOpen)
		return FALSE;

	m_IsOpen = CStdioFile::Open(lpszFileName, CFile::modeCreate | CFile::modeWrite);
	if (m_IsOpen)
		WriteString(_T("From;Dept. time;Dept. gate;To;Arr. time;Arr. gate;Distance;Airline;Flight #;Codeshares;Equipment;Aircraft registration;Aircraft name;Class;Seat;Color;Etix booking code;Fare;Award miles;Status miles;Flags;Rating;Comments;Flight time\n"));

	return m_IsOpen;
}

void CExcelFile::WriteRoute(AIRX_Flight& Flight)
{
	CString tmpStr;

	for (UINT a=0; a<FMAttributeCount; a++)
	{
		CString tmpBuffer;

		if (FMAttributes[a].Type==FMTypeDistance)
		{
			tmpBuffer.Format(_T("%d"), (UINT)(*((DOUBLE*)(((BYTE*)&Flight)+FMAttributes[a].Offset))));
		}
		else
		{
			WCHAR Buffer[256];
			AttributeToString(Flight, a, Buffer, 256);

			tmpBuffer = Buffer;
		}

		tmpBuffer.Replace(_T(";"), _T("_"));
		tmpStr += tmpBuffer;

		if (a<FMAttributeCount-1)
			tmpStr += _T(";");
	}

	WriteString(tmpStr+_T("\n"));
}

void CExcelFile::Close()
{
	if (m_IsOpen)
	{
		CStdioFile::Close();
		m_IsOpen = FALSE;
	}
}
