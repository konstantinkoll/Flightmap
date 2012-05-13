
// CExcelFile.h: Schnittstelle der Klasse CExcelFile
//

#pragma once
#include "Flightmap.h"
#include "FMCommDlg.h"
#include "CItinerary.h"


// CExcelFile
//

class CExcelFile : public CStdioFile
{
public:
	CExcelFile();

	BOOL Open(LPCTSTR lpszFileName);
	void WriteRoute(AIRX_Flight& Flight);
	void Close();

private:
	BOOL m_IsOpen;
};
