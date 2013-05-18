// IATA database

#pragma once

struct FMCountry
{
	UINT ID;
	CHAR Name[31];
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
	CHAR Name[44];
	FMGeoCoordinates Location;
};
