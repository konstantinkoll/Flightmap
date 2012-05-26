
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

#define AIRX_AwardFlight            1
#define AIRX_BusinessTrip           2
#define AIRX_LeisureTrip            4
#define AIRX_DistanceCalculated     8
#define AIRX_DistanceValid          16
#define AIRX_UnknownFrom            32
#define AIRX_VisitedFrom            64
#define AIRX_UnknownTo              128
#define AIRX_VisitedTo              256

#define AIRX_Unknown                '\0'
#define AIRX_Economy                'Y'
#define AIRX_EconomyPlus            '+'
#define AIRX_Business               'J'
#define AIRX_First                  'F'
#define AIRX_Crew                   'C'

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
	UINT FlightTime;
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

#define FMAttributeCount 24

#define FMTypeUnicodeString     0
#define FMTypeAnsiString        1
#define FMTypeRating            2
#define FMTypeUINT              3
#define FMTypeDistance          4
#define FMTypeFlags             5
#define FMTypeDateTime          6
#define FMTypeTime              7
#define FMTypeClass             8
#define FMTypeColor             9

struct FMAttribute
{
	UINT nNameID;
	UINT Type;
	UINT Offset;
	UINT DataParameter;
	UINT RecommendedWidth;
	BOOL DefaultVisible;
	BOOL Sortable;
	BOOL Editable;
};

static const FMAttribute FMAttributes[FMAttributeCount] =
{
	{ IDS_COLUMN0, FMTypeAnsiString, offsetof(AIRX_Flight, From.Code), 3, 50, TRUE, TRUE, TRUE },				// From
	{ IDS_COLUMN1, FMTypeDateTime, offsetof(AIRX_Flight, From.Time), 0, 100, FALSE, TRUE, TRUE },				// Departure time
	{ IDS_COLUMN2, FMTypeUnicodeString, offsetof(AIRX_Flight, From.Gate), 7, 50, FALSE, TRUE, TRUE },			// Departure gate
	{ IDS_COLUMN3, FMTypeAnsiString, offsetof(AIRX_Flight, To.Code), 3, 50, TRUE, TRUE, TRUE },					// To
	{ IDS_COLUMN4, FMTypeDateTime, offsetof(AIRX_Flight, To.Time), 0, 100, FALSE, TRUE, TRUE },					// Arrival time
	{ IDS_COLUMN5, FMTypeUnicodeString, offsetof(AIRX_Flight, To.Gate), 7, 50, FALSE, TRUE, TRUE },				// Arrival gate
	{ IDS_COLUMN6, FMTypeDistance, offsetof(AIRX_Flight, DistanceNM), 0, 140, TRUE, TRUE, FALSE },				// Distance
	{ IDS_COLUMN7, FMTypeUnicodeString, offsetof(AIRX_Flight, Carrier), 63, 150, TRUE, TRUE, TRUE },			// Carrier
	{ IDS_COLUMN8, FMTypeAnsiString, offsetof(AIRX_Flight, FlightNo), 7, 100, FALSE, TRUE, TRUE },				// Flight no
	{ IDS_COLUMN9, FMTypeUnicodeString, offsetof(AIRX_Flight, Codeshares), 63, 100, FALSE, FALSE, TRUE },		// Codeshares
	{ IDS_COLUMN10, FMTypeUnicodeString, offsetof(AIRX_Flight, Equipment), 63, 150, TRUE, TRUE, TRUE },			// Equipment
	{ IDS_COLUMN11, FMTypeAnsiString, offsetof(AIRX_Flight, Registration), 15, 100, FALSE, TRUE, TRUE },		// Registration
	{ IDS_COLUMN12, FMTypeUnicodeString, offsetof(AIRX_Flight, Name), 63, 100, FALSE, TRUE, TRUE },				// Name
	{ IDS_COLUMN13, FMTypeClass, offsetof(AIRX_Flight, Class), 0, 75, TRUE, TRUE, TRUE },						// Class
	{ IDS_COLUMN14, FMTypeAnsiString, offsetof(AIRX_Flight, Seat), 3, 50, TRUE, TRUE, TRUE },					// Seat
	{ IDS_COLUMN15, FMTypeColor, offsetof(AIRX_Flight, Color), 0, 50, TRUE, FALSE, TRUE },						// Color
	{ IDS_COLUMN16, FMTypeAnsiString, offsetof(AIRX_Flight, EtixCode), 7, 100, FALSE, TRUE, TRUE },				// Etix code
	{ IDS_COLUMN17, FMTypeUINT, offsetof(AIRX_Flight, Fare), 15, 100, FALSE, TRUE, TRUE },						// Fare
	{ IDS_COLUMN18, FMTypeUINT, offsetof(AIRX_Flight, MilesAward), 0 , 70, FALSE, TRUE, TRUE },					// Award miles
	{ IDS_COLUMN19, FMTypeUINT, offsetof(AIRX_Flight, MilesStatus), 0, 70, FALSE, TRUE, TRUE },					// Status miles
	{ IDS_COLUMN20, FMTypeFlags, offsetof(AIRX_Flight, Flags), 0, 78, FALSE, FALSE, TRUE },						// Flags
	{ IDS_COLUMN21, FMTypeRating, offsetof(AIRX_Flight, Flags), 28, RatingBitmapWidth+7, FALSE, TRUE, TRUE },	// Rating
	{ IDS_COLUMN22, FMTypeUnicodeString, offsetof(AIRX_Flight, Comments), 255, 100, TRUE, TRUE, TRUE },			// Comments
	{ IDS_COLUMN23, FMTypeTime, offsetof(AIRX_Flight, FlightTime), 0, 100, FALSE, TRUE, TRUE }					// Flight time
};

void ResetFlight(AIRX_Flight& Flight);
void CalcDistance(AIRX_Flight& Flight, BOOL Force=FALSE);
void DistanceToString(WCHAR* pBuffer, SIZE_T cCount, DOUBLE DistanceNM);
void AttributeToString(AIRX_Flight& Flight, UINT Attr, WCHAR* pBuffer, SIZE_T cCount);
void StringToAttribute(WCHAR* pStr, AIRX_Flight& Flight, UINT Attr);


// CItinerary
//

class CItinerary
{
public:
	CItinerary(BOOL LoadAuthor=FALSE);
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
	static FILETIME MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute);
	void AddFlight(CHAR* From, CHAR* To, WCHAR* Carrier, WCHAR* Equipment, CHAR* FlightNo, CHAR Class, CHAR* Seat, CHAR* Registration, WCHAR* Name, UINT Miles, COLORREF Color, FILETIME Departure);
	void SetDisplayName(CString FileName);

	BOOL m_IsOpen;
};
