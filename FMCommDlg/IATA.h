// IATA database

#pragma once

struct FMGeoCoordinates
{
	DOUBLE Latitude;
	DOUBLE Longitude;
};

#pragma pack(push,1)

struct FMCountry
{
	UINT ID;
	CHAR Name[31];
};

typedef const FMCountry* LPCCOUNTRY;

struct FMAirport
{
	INT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[44];
	FMGeoCoordinates Location;
};

typedef const FMAirport* LPCAIRPORT;

#pragma pack(pop)
