
// CItinerary.h: Schnittstelle der Klasse CItinerary
//

#pragma once
#include "Flightmap.h"


// CItinerary
//

struct AIRX_Header
{
	DWORD Magic;
	UINT MetadataRecordSize;
	UINT FlightCount;
	UINT FlightRecordSize;
	UINT AttachmentCount;
	UINT AttachmentRecordSize;
	UINT Unused[2];
};

struct AIRX_Metadata
{
	WCHAR Title[256];
	WCHAR Author[256];
	WCHAR Keywords[256];
	WCHAR Comments[256];
};

struct AIRX_Location
{
	CHAR Code[4];
	FILETIME Time;
	WCHAR Gate[8];
};

#define AIRX_AwardFlight      1
#define AIRX_BusinessTrip     2
#define AIRX_LeisureTrip      4

#define AIRX_Unknown          '\0'
#define AIRX_Economy          'Y'
#define AIRX_EconomyPlus      '+'
#define AIRX_Business         'J'
#define AIRX_First            'F'
#define AIRX_Crew             'C'

struct AIRX_Flight
{
	DWORD Flags;
	AIRX_Location From;
	AIRX_Location To;
	FMGeoCoordinates Waypoint;
	DOUBLE DistanceNM;
	WCHAR Carrier[64];
	WCHAR Equipment[64];
	WCHAR Comments[256];
	CHAR Registration[16];
	WCHAR Name[64];
	CHAR FlightNo[8];
	CHAR EtixCode[7];
	CHAR Class;
	CHAR Seat[4];
	COLORREF Color;
	UINT MilesAward;
	UINT MilesStatus;
	WCHAR Fare[16];
	WCHAR Codeshare[64];
};

struct AIRX_Attachment
{
	WCHAR Name[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	UINT Size;
	LPVOID pData;
};

class CItinerary
{
public:
	CItinerary();
	~CItinerary();

	void NewSampleAtlantic();
	void NewSamplePacific();
	void OpenAIRX(CString FileName);
	void OpenAIR(CString FileName);
	void OpenCSV(CString FileName);
	void SaveAIRX(CString FileName);

	static CString Flight2Text(AIRX_Flight& Flight);
	CString Flight2Text(UINT Idx);

	void AddFlight();

	AIRX_Metadata m_Metadata;
	DynArray<AIRX_Flight> m_Flights;
	DynArray<AIRX_Attachment> m_Attachments;

	BOOL m_IsModified;
	CString m_FileName;
	CString m_DisplayName;

private:
	static void ResetFlight(AIRX_Flight& Flight);
	static FILETIME MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute);
	void AddFlight(CHAR* From, CHAR* To, WCHAR* Carrier, WCHAR* Equipment, CHAR* FlightNo, CHAR Class, CHAR* Seat, CHAR* Registration, WCHAR* Name, UINT Miles, COLORREF Color, FILETIME Departure);
	void SetDisplayName(CString FileName);

	BOOL m_IsOpen;
};
