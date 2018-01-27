
// CExcelFile.h: Schnittstelle der Klasse CExcelFile
//

#pragma once
#include "CItinerary.h"


// CExcelFile
//

class CExcelFile : public CStdioFile
{
public:
	CExcelFile();

	BOOL Open(LPCTSTR lpszPath);
	void WriteFlight(const AIRX_Flight& Flight);
	void Close();

protected:
	WCHAR m_Separator[4];

private:
	BOOL m_IsOpen;
};
