
// CGoogleEarthFile.h: Schnittstelle der Klasse CGoogleEarthFile
//

#pragma once
#include "CKitchen.h"


// CGoogleEarthFile
//

class CGoogleEarthFile : public CStdioFile
{
public:
	CGoogleEarthFile();

	BOOL Open(LPCTSTR lpszFileName, LPCTSTR lpszDisplayName=NULL);
	void WriteAirport(FMAirport* pAirport);
	void WriteRoute(FlightSegments* pSegments, UINT MinRouteCount=0, UINT MaxRouteCount=0, BOOL UseCount=FALSE, BOOL UseColors=TRUE, BOOL ClampHeight=FALSE, BOOL FreeSegments=TRUE);
	void WriteRoutes(CKitchen* pKitchen, BOOL UseCount=FALSE, BOOL UseColors=TRUE, BOOL ClampHeight=FALSE, BOOL FreeKitchen=TRUE);
	void Close();

private:
	void WriteAttribute(UINT ResID, const CString& Value);
	void WriteAttribute(UINT ResID, LPCSTR pValue);

	BOOL m_IsOpen;
};
