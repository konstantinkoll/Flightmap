
// CItinerary.h: Schnittstelle der Klasse CItinerary
//

#pragma once
#include "FMCommDlg.h"
#include "Resource.h"


// AIRX format
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
	WCHAR Codeshares[64];
};

struct AIRX_Attachment
{
	WCHAR Name[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	UINT Size;
	LPVOID pData;
};


// Attributes
//

#define FMAttributeCount 23

struct FMAttribute
{
	UINT nNameID;
	UINT Offset;
	UINT DataParameter;
	UINT RecommendedWidth;
	BOOL DefaultVisible;
	BOOL Sortable;
	BOOL Editable;
};

static const FMAttribute FMAttributes[FMAttributeCount] = {
	{ IDS_COLUMN0, offsetof(AIRX_Flight, From.Code), 4, 100, TRUE, TRUE, TRUE },		// From
	{ IDS_COLUMN1, offsetof(AIRX_Flight, From.Time), 0, 100, FALSE, TRUE, TRUE },		// Departure time
	{ IDS_COLUMN2, offsetof(AIRX_Flight, From.Gate), 8, 100, FALSE, TRUE, TRUE },		// Departure gate
	{ IDS_COLUMN3, offsetof(AIRX_Flight, To.Code), 4, 100, TRUE, TRUE, TRUE },			// To
	{ IDS_COLUMN4, offsetof(AIRX_Flight, To.Time), 0, 100, FALSE, TRUE, TRUE },			// Arrival time
	{ IDS_COLUMN5, offsetof(AIRX_Flight, To.Gate), 8, 100, FALSE, TRUE, TRUE },			// Arrival gate
	{ IDS_COLUMN6, offsetof(AIRX_Flight, DistanceNM), 0, 100, TRUE, TRUE, FALSE },		// Distance
	{ IDS_COLUMN7, offsetof(AIRX_Flight, Carrier), 64, 100, TRUE, TRUE, TRUE },			// Carrier
	{ IDS_COLUMN8, offsetof(AIRX_Flight, FlightNo), 8, 100, FALSE, TRUE, TRUE },		// Flight no
	{ IDS_COLUMN9, offsetof(AIRX_Flight, Codeshares), 64, 100, FALSE, FALSE, TRUE },	// Codeshares
	{ IDS_COLUMN10, offsetof(AIRX_Flight, Equipment), 64, 100, TRUE, TRUE, TRUE },		// Equipment
	{ IDS_COLUMN11, offsetof(AIRX_Flight, Registration), 16, 100, FALSE, TRUE, TRUE },	// Registration
	{ IDS_COLUMN12, offsetof(AIRX_Flight, Name), 64, 100, FALSE, TRUE, TRUE },			// Name
	{ IDS_COLUMN13, offsetof(AIRX_Flight, Class), 0, 100, TRUE, TRUE, TRUE },			// Class
	{ IDS_COLUMN14, offsetof(AIRX_Flight, Seat), 4, 100, TRUE, TRUE, TRUE },			// Seat
	{ IDS_COLUMN15, offsetof(AIRX_Flight, Color), 0, 100, TRUE, FALSE, TRUE },			// Color
	{ IDS_COLUMN16, offsetof(AIRX_Flight, EtixCode), 7, 100, FALSE, TRUE, TRUE },		// Etix code
	{ IDS_COLUMN17, offsetof(AIRX_Flight, Fare), 16, 100, FALSE, TRUE, TRUE },			// Fare
	{ IDS_COLUMN18, offsetof(AIRX_Flight, MilesAward), 0 ,100, FALSE, TRUE, TRUE },		// Award miles
	{ IDS_COLUMN19, offsetof(AIRX_Flight, MilesStatus), 0, 100, FALSE, TRUE, TRUE },	// Status miles
	{ IDS_COLUMN20, offsetof(AIRX_Flight, Flags), 0, 100, FALSE, FALSE, FALSE },		// Flags
	{ IDS_COLUMN21, offsetof(AIRX_Flight, Flags), 29, 100, FALSE, TRUE, TRUE },			// Rating
	{ IDS_COLUMN22,offsetof(AIRX_Flight, Comments), 256, 100, TRUE, TRUE, TRUE }		// Comments
};


// CItinerary
//

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
