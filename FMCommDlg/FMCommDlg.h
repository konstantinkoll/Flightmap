#pragma once

// IATA database

struct FMCountry
{
	UINT ID;
	CHAR Name[64];
};

struct FMGeoCoordinates
{
	DOUBLE Latitude;
	DOUBLE Longitude;
};

struct FMAirport
{
	INT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[64];
	FMGeoCoordinates Location;
};

#ifdef FMCommDlg_EXPORTS
#define FMCommDlg_API __declspec(dllexport)
#else
#define FMCommDlg_API __declspec(dllimport)
#endif

FMCommDlg_API UINT FMIATAGetCountryCount();
FMCommDlg_API UINT FMIATAGetAirportCount();
FMCommDlg_API FMCountry* FMIATAGetCountry(UINT ID);
FMCommDlg_API INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer);
FMCommDlg_API INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer);
FMCommDlg_API BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer);
