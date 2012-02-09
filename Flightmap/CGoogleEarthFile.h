
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
	void WriteAirport(FMAirport* pAirport);
	void WriteRoute(FlightSegments* pSegments, BOOL UseColors=TRUE, BOOL Clamp=FALSE, BOOL FreeSegments=TRUE);
	void WriteRoutes(CKitchen* pKitchen, BOOL UseColors=TRUE, BOOL Clamp=FALSE, BOOL FreeKitchen=TRUE);
	void Close();

private:
	void WriteAttribute(UINT ResID, CString Value);
	void WriteAttribute(UINT ResID, CHAR* Value);

	BOOL m_IsOpen;
};
