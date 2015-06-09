
// CCalendarFile.h: Schnittstelle der Klasse CCalendarFile
//

#pragma once
#include "CItinerary.h"


// CCalendarFile
//

class CCalendarFile : public CStdioFile
{
public:
	CCalendarFile();

	BOOL Open(LPCTSTR lpszFileName, LPCTSTR lpszComment=NULL, LPCTSTR lpszDescription=NULL);
	void WriteRoute(AIRX_Flight& Flight);
	void Close();

private:
	BOOL m_IsOpen;
};
