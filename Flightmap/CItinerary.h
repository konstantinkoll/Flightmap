
// CItinerary.h: Schnittstelle der Klasse CItinerary
//

#pragma once
#include "FMCommDlg.h"
#include "rapidxml.hpp"
#include "resource.h"


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
	UINT CurrentRow;
};

struct AIRX_Location
{
	CHAR Code[4];
	FILETIME Time;
	WCHAR Gate[8];
};

struct AIRX_Route
{
	CHAR From[4];
	CHAR To[4];
	DOUBLE DistanceNM;
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
#define AIRX_GroundTransportation   512
#define AIRX_Selected               1024
#define AIRX_Cancelled              2048
#define AIRX_Upgrade                4096
#define AIRX_FutureFlight           8192

#define AIRX_Unknown                '\0'
#define AIRX_Economy                'Y'
#define AIRX_PremiumEconomy         '+'
#define AIRX_Business               'J'
#define AIRX_First                  'F'
#define AIRX_Crew                   'C'
#define AIRX_Charter                'H'

#define AIRX_MaxAttachmentCount     16
#define AIRX_Valid                  1
#define AIRX_Invalid                2

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
	UINT Fare;
	BYTE Dummy[28];
	WCHAR Codeshares[64];
	UINT FlightTime;
	UINT AttachmentCount;
	UINT Attachments[AIRX_MaxAttachmentCount];
	UINT UpgradeVoucher;
};

struct AIRX_Attachment
{
	WCHAR Name[MAX_PATH];
	FILETIME Created;
	FILETIME Modified;
	UINT Size;
	LPVOID pData;
	INT IconID;
	DWORD Flags;
};


// Attributes
//

#define FMAttributeCount        25

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
	BOOL Searchable;
};

static const FMAttribute FMAttributes[FMAttributeCount] =
{
	{ IDS_COLUMN0, FMTypeAnsiString, offsetof(AIRX_Flight, From.Code), 3, 50, TRUE, TRUE, TRUE, TRUE },					// From
	{ IDS_COLUMN1, FMTypeDateTime, offsetof(AIRX_Flight, From.Time), 0, 150, FALSE, TRUE, TRUE, TRUE },					// Departure time
	{ IDS_COLUMN2, FMTypeUnicodeString, offsetof(AIRX_Flight, From.Gate), 7, 50, FALSE, TRUE, TRUE, TRUE },				// Departure gate
	{ IDS_COLUMN3, FMTypeAnsiString, offsetof(AIRX_Flight, To.Code), 3, 50, TRUE, TRUE, TRUE, TRUE },					// To
	{ IDS_COLUMN4, FMTypeDateTime, offsetof(AIRX_Flight, To.Time), 0, 150, FALSE, TRUE, TRUE, TRUE },					// Arrival time
	{ IDS_COLUMN5, FMTypeUnicodeString, offsetof(AIRX_Flight, To.Gate), 7, 50, FALSE, TRUE, TRUE, TRUE },				// Arrival gate
	{ IDS_COLUMN6, FMTypeDistance, offsetof(AIRX_Flight, DistanceNM), 0, 140, TRUE, TRUE, FALSE, TRUE },				// Distance
	{ IDS_COLUMN7, FMTypeUnicodeString, offsetof(AIRX_Flight, Carrier), 63, 150, TRUE, TRUE, TRUE, TRUE },				// Carrier
	{ IDS_COLUMN8, FMTypeAnsiString, offsetof(AIRX_Flight, FlightNo), 7, 70, FALSE, TRUE, TRUE, TRUE },					// Flight no
	{ IDS_COLUMN9, FMTypeUnicodeString, offsetof(AIRX_Flight, Codeshares), 63, 100, FALSE, FALSE, TRUE, TRUE },			// Codeshares
	{ IDS_COLUMN10, FMTypeUnicodeString, offsetof(AIRX_Flight, Equipment), 63, 150, TRUE, TRUE, TRUE, TRUE },			// Equipment
	{ IDS_COLUMN11, FMTypeAnsiString, offsetof(AIRX_Flight, Registration), 15, 100, FALSE, TRUE, TRUE, TRUE },			// Registration
	{ IDS_COLUMN12, FMTypeUnicodeString, offsetof(AIRX_Flight, Name), 63, 100, FALSE, TRUE, TRUE, TRUE },				// Name
	{ IDS_COLUMN13, FMTypeClass, offsetof(AIRX_Flight, Class), 0, 75, TRUE, TRUE, TRUE, FALSE },						// Class
	{ IDS_COLUMN14, FMTypeAnsiString, offsetof(AIRX_Flight, Seat), 3, 50, TRUE, TRUE, TRUE, TRUE },						// Seat
	{ IDS_COLUMN15, FMTypeColor, offsetof(AIRX_Flight, Color), 0, 50, TRUE, FALSE, TRUE, FALSE },						// Color
	{ IDS_COLUMN16, FMTypeAnsiString, offsetof(AIRX_Flight, EtixCode), 6, 70, FALSE, TRUE, TRUE, TRUE },				// Etix code
	{ IDS_COLUMN17, FMTypeUINT, offsetof(AIRX_Flight, Fare), 15, 100, FALSE, TRUE, TRUE, TRUE },						// Fare
	{ IDS_COLUMN18, FMTypeUINT, offsetof(AIRX_Flight, MilesAward), 0 , 70, FALSE, TRUE, TRUE, TRUE },					// Award miles
	{ IDS_COLUMN19, FMTypeUINT, offsetof(AIRX_Flight, MilesStatus), 0, 70, FALSE, TRUE, TRUE, TRUE },					// Status miles
	{ IDS_COLUMN20, FMTypeFlags, offsetof(AIRX_Flight, Flags), 0, 132, FALSE, FALSE, TRUE, FALSE },						// Flags
	{ IDS_COLUMN21, FMTypeRating, offsetof(AIRX_Flight, Flags), 28, RatingBitmapWidth+7, FALSE, TRUE, TRUE, FALSE },	// Rating
	{ IDS_COLUMN22, FMTypeUnicodeString, offsetof(AIRX_Flight, Comments), 255, 100, TRUE, TRUE, TRUE, TRUE },			// Comments
	{ IDS_COLUMN23, FMTypeTime, offsetof(AIRX_Flight, FlightTime), 0, 100, FALSE, TRUE, TRUE, TRUE },					// Flight time
	{ IDS_COLUMN24, FMTypeUINT, offsetof(AIRX_Flight, UpgradeVoucher), 0, 70, FALSE, TRUE, TRUE, TRUE }					// Voucher used
};


// CGPXFile
//

using namespace rapidxml;

class CGPXFile : public xml_document<>
{
};


// CItinerary
//

class CItinerary
{
public:
	CItinerary(const CString& Filename=_T(""));
	CItinerary(CItinerary* pItinerary);
	~CItinerary();

	void NewSampleAtlantic();
	void NewSamplePacific();
	void SaveAIRX(CString Filename);

	static CString Flight2Text(AIRX_Flight& Flight);
	CString Flight2Text(UINT Index);

	void Sort(UINT Attr, BOOL Descending);
	AIRX_Attachment* GetGPSPath(AIRX_Flight& Flight);
	AIRX_Attachment* GetGPSPath(UINT Index);

	void AddFlight();
	void AddFlight(CItinerary* pItinerary, UINT Row);
	void InsertFlights(UINT Row, UINT Count=1, AIRX_Flight* pFlights=NULL);
	void DeleteFlight(UINT Row);
	void DeleteSelectedFlights();

	BOOL AddAttachment(AIRX_Flight& Flight, CString Filename);
	UINT AddAttachment(CItinerary* pItinerary, UINT Index);
	Bitmap* DecodePictureAttachment(UINT Index) const;
	static Bitmap* DecodePictureAttachment(const AIRX_Attachment& Attachment);
	static CGPXFile* DecodeGPXAttachment(const AIRX_Attachment& Attachment);
	static void ValidateAttachment(AIRX_Attachment& Attachment, BOOL Force=FALSE);
	void DeleteAttachment(UINT Index, AIRX_Flight* pFlight=NULL);
	void DeleteAttachments(AIRX_Flight* pFlight=NULL);

	AIRX_Metadata m_Metadata;
	FMDynArray<AIRX_Flight, 128, 128> m_Flights;
	FMDynArray<AIRX_Attachment, 16, 16> m_Attachments;

	BOOL m_IsModified;
	CString m_FileName;
	CString m_DisplayName;

private:
	static void Swap(AIRX_Flight& Eins, AIRX_Flight& Zwei);
	void OpenAIRX(const CString& Filename);
	void OpenAIR(const CString& Filename);
	void OpenCSV(const CString& Filename);
	static FILETIME MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute);
	static INT Compare(AIRX_Flight* Eins, AIRX_Flight* Zwei, const UINT Attr, const BOOL Descending);
	void Heap(UINT Wurzel, const UINT Anzahl, const UINT Attr, const BOOL Descending);
	void AddFlight(CHAR* From, CHAR* To, WCHAR* Carrier, WCHAR* Equipment, CHAR* FlightNo, CHAR Class, CHAR* Seat, CHAR* Registration, WCHAR* Name, UINT Miles, COLORREF Color, FILETIME Departure);
	void SetDisplayName(const CString& Filename);
	static void RemoveAttachment(UINT Index, AIRX_Flight* pFlight);
	static void FreeAttachment(AIRX_Attachment& Attachment);

	BOOL m_IsOpen;
};

inline void CItinerary::Swap(AIRX_Flight& Eins, AIRX_Flight& Zwei)
{
	AIRX_Flight Temp = Eins;
	Eins = Zwei;
	Zwei = Temp;
}

inline void CItinerary::FreeAttachment(AIRX_Attachment& Attachment)
{
	free(Attachment.pData);
}


// Helpers
//

void ResetFlight(AIRX_Flight& Flight);
void CalcDistance(AIRX_Flight& Flight, BOOL Force=FALSE);
void CalcFuture(AIRX_Flight& Flight, SYSTEMTIME* pTime=NULL);
void PrepareEditCtrl(CMFCMaskedEdit* pEdit, UINT Attr, AIRX_Flight* pFlight=NULL);
void PrepareCarrierCtrl(CComboBox* pComboBox, CItinerary* pItinerary=NULL, BOOL IncludeDatabase=TRUE);
void PrepareEquipmentCtrl(CComboBox* pComboBox, CItinerary* pItinerary=NULL, BOOL IncludeDatabase=TRUE);
void DDX_MaskedText(CDataExchange* pDX, INT nIDC, CMFCMaskedEdit& rControl, UINT Attr, AIRX_Flight* pFlight=NULL);

void DistanceToString(WCHAR* pBuffer, SIZE_T cCount, DOUBLE DistanceNM);
void TimeToString(WCHAR* pBuffer, SIZE_T cCount, UINT Time);
void DateTimeToString(WCHAR* pBuffer, SIZE_T cCount, FILETIME ft);
void RouteToString(WCHAR* pBuffer, SIZE_T cCount, AIRX_Route& Route);
void MilesToString(CString &tmpStr, LONG AwardMiles, LONG StatusMiles);
void AttributeToString(AIRX_Flight& Flight, UINT Attr, WCHAR* pBuffer, SIZE_T cCount);
void StringToAttribute(LPWSTR lpszStr, AIRX_Flight& Flight, UINT Attr);

BOOL Tokenize(const CString& strSrc, CString& strDst, INT& Pos, const CString& Delimiter, WCHAR* pDelimiterFound=NULL);
