
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
#define AIRX_FutureFlight           8
#define AIRX_DistanceValid          16
#define AIRX_UnknownFrom            32
#define AIRX_UnknownTo              64
#define AIRX_RESERVED1              128
#define AIRX_RESERVED2              256
#define AIRX_GroundTransportation   512
#define AIRX_Selected               1024
#define AIRX_Cancelled              2048
#define AIRX_Upgrade                4096

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
	UINT FileSize;
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
	UINT DefaultColumnWidth;
	BOOL DefaultVisible;
	BOOL Sortable;
	BOOL Editable;
	BOOL Searchable;
};

static const FMAttribute FMAttributes[FMAttributeCount] =
{
	{ IDS_COLUMN0, FMTypeAnsiString, offsetof(AIRX_Flight, From.Code), 3, 50, TRUE, TRUE, TRUE, TRUE },					// From
	{ IDS_COLUMN1, FMTypeDateTime, offsetof(AIRX_Flight, From.Time), 0, 150, TRUE, TRUE, TRUE, TRUE },					// Departure time
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
	{ IDS_COLUMN15, FMTypeColor, offsetof(AIRX_Flight, Color), 0, 50, FALSE, FALSE, TRUE, FALSE },						// Color
	{ IDS_COLUMN16, FMTypeAnsiString, offsetof(AIRX_Flight, EtixCode), 6, 70, FALSE, TRUE, TRUE, TRUE },				// Etix code
	{ IDS_COLUMN17, FMTypeUINT, offsetof(AIRX_Flight, Fare), 15, 100, FALSE, FALSE, TRUE, TRUE },						// Fare
	{ IDS_COLUMN18, FMTypeUINT, offsetof(AIRX_Flight, MilesAward), 0 , 70, FALSE, TRUE, TRUE, TRUE },					// Award miles
	{ IDS_COLUMN19, FMTypeUINT, offsetof(AIRX_Flight, MilesStatus), 0, 70, FALSE, TRUE, TRUE, TRUE },					// Status miles
	{ IDS_COLUMN20, FMTypeFlags, offsetof(AIRX_Flight, Flags), 0, 132, FALSE, FALSE, TRUE, FALSE },						// Flags
	{ IDS_COLUMN21, FMTypeRating, offsetof(AIRX_Flight, Flags), 28, RATINGBITMAPWIDTH+7, FALSE, TRUE, TRUE, FALSE },	// Rating
	{ IDS_COLUMN22, FMTypeUnicodeString, offsetof(AIRX_Flight, Comments), 255, 100, TRUE, TRUE, TRUE, TRUE },			// Comments
	{ IDS_COLUMN23, FMTypeTime, offsetof(AIRX_Flight, FlightTime), 0, 100, FALSE, TRUE, TRUE, TRUE },					// Flight time
	{ IDS_COLUMN24, FMTypeUINT, offsetof(AIRX_Flight, UpgradeVoucher), 0, 70, FALSE, TRUE, TRUE, TRUE }					// Voucher used
};


// Helpers
//

void PrepareEditCtrl(CMFCMaskedEdit* pMaskedEdit, UINT Attr, AIRX_Flight* pFlight=NULL);
void DDX_MaskedText(CDataExchange* pDX, INT nIDC, CMFCMaskedEdit& rControl, UINT Attr, AIRX_Flight* pFlight=NULL);

BOOL Tokenize(const CString& strSrc, CString& strDst, INT& Pos, const CString& Delimiter, WCHAR* DelimiterFound=NULL);


// ToString
//

void DistanceToString(LPWSTR pStr, SIZE_T cCount, DOUBLE DistanceNM);
void TimeToString(LPWSTR pStr, SIZE_T cCount, UINT Time);
void DateTimeToString(LPWSTR pStr, SIZE_T cCount, const FILETIME& FileTime);
void RouteToString(LPWSTR pStr, SIZE_T cCount, AIRX_Route& Route);
void MilesToString(LPWSTR pStr, SIZE_T cCount, LONG AwardMiles, LONG StatusMiles);
void AttributeToString(const AIRX_Flight& Flight, UINT Attr, LPWSTR pStr, SIZE_T cCount);

// FromString
//

void StringToAttribute(CString Str, AIRX_Flight& Flight, UINT Attr);


// CGPXFile
//

using namespace rapidxml;

class CGPXFile : public xml_document<>
{
};


// CItinerary
//

class CItinerary sealed
{
public:
	CItinerary(const CString& Path=_T(""));
	CItinerary(CItinerary* pItinerary);
	~CItinerary();

	void NewSampleAtlantic();
	void NewSamplePacific();

	void SaveAIRX(const CString& Path, UINT CurrentRow);
	LPCWSTR GetTitle() const;
	void InsertDisplayName(CString& Caption) const;

	UINT GetFlightCount(BOOL Limit=TRUE) const;
	static void ResetFlight(AIRX_Flight& Flight);
	void UpdateFlight(UINT Row, LPSYSTEMTIME pTime=NULL);
	void AddFlight();
	void AddFlight(AIRX_Flight& Flight, LPSYSTEMTIME pTime=NULL);
	void AddFlight(CItinerary* pItinerary, UINT Row);
	void InsertFlights(UINT Row, UINT Count=1, AIRX_Flight* pFlights=NULL);
	void DeleteFlight(UINT Row);
	void DeleteSelectedFlights();

	CString Flight2Text(UINT Row) const;
	AIRX_Attachment* GetGPSPath(UINT Row);

	void SortItems(UINT Attr, BOOL Descending);

	BOOL AddAttachment(AIRX_Flight& Flight, const CString& Path);
	INT AddAttachment(CItinerary* pItinerary, UINT Index);
	static CGPXFile* DecodeGPXAttachment(const AIRX_Attachment& Attachment);
	static Bitmap* DecodePictureAttachment(const AIRX_Attachment& Attachment);
	static void ValidateAttachment(AIRX_Attachment& Attachment, BOOL Force=FALSE);
	void RenameAttachment(AIRX_Attachment& Attachment, const CString& Name);
	void RenameAttachment(UINT Index, const CString& Name);
	void DeleteAttachment(UINT Index, AIRX_Flight* pFlight=NULL);
	void DeleteAttachments(AIRX_Flight& Flight);

	void PrepareCarrierCtrl(CComboBox& ComboBox, BOOL IncludeDatabase=TRUE) const;
	void PrepareEquipmentCtrl(CComboBox& ComboBox, BOOL IncludeDatabase=TRUE) const;

	AIRX_Metadata m_Metadata;
	FMDynArray<AIRX_Flight, 128, 128> m_Flights;
	FMDynArray<AIRX_Attachment, 16, 16> m_Attachments;

	BOOL m_IsModified;
	CString m_FileName;
	CString m_DisplayName;

protected:
	static FILETIME MakeTime(WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute);

	void SetDisplayName(const CString& Path);
	static void CompressFile(HANDLE hFile, WCHAR cDrive, BOOL bCompress);
	static BOOL ReadRecord(CFile& File, LPVOID pBuffer, UINT BufferSize, UINT OnDiscSize);

	void ResetFlight(UINT Row);
	static void UpdateFlight(AIRX_Flight& Flight, LPSYSTEMTIME pTime=NULL);
	void AddFlight(LPCSTR From, LPCSTR To, LPCWSTR Carrier, LPCWSTR Equipment, LPCSTR FlightNo, CHAR Class, LPCSTR Seat, LPCSTR Registration, LPCWSTR Name, UINT Miles, COLORREF Color, const FILETIME& Departure);

private:
	void OpenAIRX(const CString& Path);
	void OpenCSV(const CString& Path);

	static CString ExportAttribute(const AIRX_Flight& Flight, UINT Attr, BOOL Label=TRUE, BOOL Colon=TRUE, BOOL NewLine=TRUE);
	static CString ExportLocation(const AIRX_Flight& Flight, UINT AttrBase);

	static INT __stdcall CompareItems(const AIRX_Flight* pFlight1, const AIRX_Flight* pFlight2, const SortParameters& Parameters);

	static void RemoveAttachmentFromFlight(UINT Index, AIRX_Flight& Flight);
	static void FreeAttachment(AIRX_Attachment& Attachment);

	BOOL m_IsOpen;
};

inline LPCWSTR CItinerary::GetTitle() const
{
	return m_Metadata.Title[0] ? m_Metadata.Title : m_DisplayName;
}

inline void CItinerary::InsertDisplayName(CString& Caption) const
{
	if (!m_DisplayName.IsEmpty())
		Caption.Insert(0, m_DisplayName+_T(" - "));
}

inline void CItinerary::ResetFlight(UINT Row)
{
	ASSERT(Row<m_Flights.m_ItemCount);

	ResetFlight(m_Flights[Row]);
}

inline void CItinerary::UpdateFlight(UINT Row, LPSYSTEMTIME pTime)
{
	ASSERT(Row<m_Flights.m_ItemCount);

	UpdateFlight(m_Flights[Row], pTime);
}

inline void CItinerary::AddFlight()
{
	AIRX_Flight Flight;
	ResetFlight(Flight);

	m_Flights.AddItem(Flight);
}

inline void CItinerary::AddFlight(AIRX_Flight& Flight, LPSYSTEMTIME pTime)
{
	UpdateFlight(Flight, pTime);

	m_Flights.AddItem(Flight);
}

inline void CItinerary::SortItems(UINT Attr, BOOL Descending)
{
	m_Flights.SortItems((PFNCOMPARE)CompareItems, Attr, Descending);
}

inline void CItinerary::RenameAttachment(UINT Index, const CString& Name)
{
	ASSERT(Index<m_Attachments.m_ItemCount);

	RenameAttachment(m_Attachments[Index], Name);
}

inline void CItinerary::FreeAttachment(AIRX_Attachment& Attachment)
{
	free(Attachment.pData);
}
