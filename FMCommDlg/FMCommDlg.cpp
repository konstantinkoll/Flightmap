
// FMCommDlg.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "FMCommDlg.h"


// IATA-Datenbank
//

#include "IATA_DE.h"
#include "IATA_EN.h"

static BOOL UseGermanDB = (GetUserDefaultUILanguage() & 0x1FF)==LANG_GERMAN;

FMCommDlg_API UINT FMIATAGetCountryCount()
{
	return UseGermanDB ? CountryCount_DE : CountryCount_EN;
}

FMCommDlg_API UINT FMIATAGetAirportCount()
{
	return UseGermanDB ? AirportCount_DE : AirportCount_EN;
}

FMCommDlg_API FMCountry* FMIATAGetCountry(UINT ID)
{
	return UseGermanDB ? &Countries_DE[ID] : &Countries_EN[ID];
}

FMCommDlg_API INT FMIATAGetNextAirport(INT Last, FMAirport** pBuffer)
{
	if (Last>=(INT)FMIATAGetAirportCount()-1)
		return -1;

	*pBuffer = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	return Last;
}

FMCommDlg_API INT FMIATAGetNextAirportByCountry(INT CountryID, INT Last, FMAirport** pBuffer)
{
	INT Count = (INT)FMIATAGetAirportCount();

	do
	{
		if (Last>=Count-1)
			return -1;

		*pBuffer = UseGermanDB ? &Airports_DE[++Last] : &Airports_EN[++Last];
	}
	while ((*pBuffer)->CountryID!=CountryID);

	return Last;
}

FMCommDlg_API BOOL FMIATAGetAirportByCode(CHAR* Code, FMAirport** pBuffer)
{
	if (!Code)
		return FALSE;
	if (*Code=='\0')
		return FALSE;

	INT First = 0;
	INT Last = (INT)FMIATAGetAirportCount()-1;

	while (First<=Last)
	{
		INT Mid = (First+Last)/2;

		*pBuffer = UseGermanDB ? &Airports_DE[Mid] : &Airports_EN[Mid];
		INT Res = strcmp((*pBuffer)->Code, Code);
		if (Res==0)
			return TRUE;

		if (Res<0)
		{
			First = Mid+1;
		}
		else
		{
			Last = Mid-1;
		}
	}

	return FALSE;
}
