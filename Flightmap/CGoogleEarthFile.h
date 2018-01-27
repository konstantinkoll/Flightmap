
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

	BOOL Open(LPCTSTR lpszPath, LPCTSTR lpszDisplayName=NULL);
	void WriteAirport(LPCAIRPORT lpcAirport);
	void WriteRoute(const FlightRoute& Route, UINT MinRouteCount=0, UINT MaxRouteCount=0, BOOL UseCount=FALSE, BOOL UseColors=TRUE, BOOL ClampHeight=FALSE);
	void WriteRoutes(CKitchen* pKitchen, BOOL UseCount=FALSE, BOOL UseColors=TRUE, BOOL ClampHeight=FALSE);
	void WriteAirports(CKitchen* pKitchen);
	void Close();

private:
	void WriteAttribute(UINT ResID, const CString& Value);
	void WriteAttribute(UINT ResID, LPCSTR pValue);

	BOOL m_IsOpen;
};
