
// CExcelFile.cpp: Implementierung der Klasse CExcelFile
//

#include "stdafx.h"
#include "CExcelFile.h"


// CExcelFile
//

CExcelFile::CExcelFile()
{
	m_IsOpen = FALSE;

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, m_Separator, 4);
}

BOOL CExcelFile::Open(LPCTSTR lpszPath)
{
	if (m_IsOpen)
		return FALSE;

	if ((m_IsOpen=CStdioFile::Open(lpszPath, CFile::modeCreate | CFile::modeWrite))==TRUE)
	{
		CString tmpStr(_T("From;Dept. time;Dept. gate;To;Arr. time;Arr. gate;Distance;Carrier;Flight;Codeshares;Equipment;Registration;Aircraft name;Class;Seat;Color;Etix code;Fare;Award miles;Status miles;Flags;Rating;Comments;Flight time;Voucher\n"));
		tmpStr.Replace(_T(";"), m_Separator);

		WriteString(tmpStr);
	}

	return m_IsOpen;
}

void CExcelFile::WriteFlight(const AIRX_Flight& Flight)
{
	CString tmpStr;

	for (UINT a=0; a<FMAttributeCount; a++)
	{
		CString tmpBuffer;

		if (FMAttributes[a].Type==FMTypeDistance)
		{
			tmpBuffer.Format(_T("%u"), (UINT)(*((DOUBLE*)(((LPBYTE)&Flight)+FMAttributes[a].Offset))));
		}
		else
		{
			WCHAR Buffer[256];
			AttributeToString(Flight, a, Buffer, 256);

			tmpBuffer = Buffer;
		}

		tmpBuffer.Replace(m_Separator, _T("_"));
		tmpStr += tmpBuffer;

		if (a<FMAttributeCount-1)
			tmpStr += m_Separator;
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
