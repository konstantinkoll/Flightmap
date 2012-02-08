
// CGoogleEarthFile.h: Schnittstelle der Klasse CGoogleEarthFile
//

#pragma once
#include "Flightmap.h"
#include "FMCommDlg.h"
#include "CKitchen.h"


// CGoogleEarthFile
//

class CGoogleEarthFile : public CStdioFile
{
public:
	CGoogleEarthFile();

	BOOL Open(LPCTSTR lpszFileName, LPCTSTR lpszDisplayName=NULL);
	void WriteRoute(FlightSegments* pSegments, BOOL UseColors=TRUE);
	void WriteRoutes(CKitchen* pKitchen, BOOL UseColors=TRUE);
	void Close();

private:
	BOOL m_IsOpen;
};
