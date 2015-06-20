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

struct FMAirport
{
	INT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[44];
	FMGeoCoordinates Location;
};

#pragma pack(pop)
