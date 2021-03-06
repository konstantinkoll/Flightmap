// ConvertIATA.cpp : Definiert den Einstiegspunkt f?r die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertIATA.h"
#include <io.h>
#include <wchar.h>


CString path;
SIZE_T maxCountryLength;
SIZE_T maxAirportLength;

using namespace std;

INT Compare(CString s1, CString s2)
{
	s1.Replace('?', 'A');
	s1.Replace('?', 'O');
	s1.Replace('?', 'U');
	s1.Replace('?', 'a');
	s1.Replace('?', 'o');
	s1.Replace('?', 'u');
	s1.Replace('?', 'z');
	s2.Replace('?', 'A');
	s2.Replace('?', 'O');
	s2.Replace('?', 'U');
	s2.Replace('?', 'a');
	s2.Replace('?', 'o');
	s2.Replace('?', 'u');
	s2.Replace('?', 'z');

	return s1.CompareNoCase(s2);
}

DOUBLE GetCoord(CString s)
{
	USES_CONVERSION;

	INT curPos = s.Find("*");
	DOUBLE c = atof(s.Mid(0, curPos));
	s = s.Mid(curPos+1);

	curPos = s.Find("'");
	c += atof(s.Mid(0, curPos))/60;
	s = s.Mid(curPos+1);

	curPos = s.Find("\"");
	c += atof(s.Mid(0, curPos))/3600;
	s = s.Mid(curPos+1);

	if ((s=="N") || (s=="W"))
		c = -c;

	return c;
}

void ConvertFile(const CString& LanguageSuffix)
{
	CString tmpStr;

	// Daten
	INT CountryCount = 0;
	CString CountryNames[512];

	struct Airport
	{
		INT CountryID;
		CString Code;
		CString MetroCode;
		CString Name;
		DOUBLE Latitude;
		DOUBLE Longitude;
		UINT Divider;
	};

	UINT AirportCount = 0;
	Airport Airports[26*26*26];

	// Einlesen
	CStdioFile input;
	input.Open(path+"..\\..\\res\\IATA-"+LanguageSuffix+".TXT", CFile::modeRead | CFile::typeText);

	while (input.ReadString(tmpStr))
	{
		INT curPos= 0;
		CString NamesAndCodes = tmpStr.Tokenize(";", curPos);
		CString Dummy = tmpStr.Tokenize(";", curPos);
		CString Coord = "";
		if (curPos!=-1)
			Coord = tmpStr.Tokenize(";", curPos);

		INT CountryPos = NamesAndCodes.Find(", ", 0);
		if (CountryPos)
		{
			Airports[AirportCount].Code = NamesAndCodes.Mid(0, 3);
			Airports[AirportCount].MetroCode = NamesAndCodes.Mid(4, 3).Trim();
			if ((Airports[AirportCount].Code!=Airports[AirportCount].MetroCode) && (Coord==""))
				continue;

			CString Country = NamesAndCodes.Mid(CountryPos+2);
			NamesAndCodes.Truncate(CountryPos);

			INT CountryID = -1;
			for (INT a=0; a<CountryCount; a++)
				if (Country==CountryNames[a])
				{
					CountryID = a;
					break;
				}

			if (CountryID==-1)
			{
				if (strlen(Country)>maxCountryLength)
					maxCountryLength = strlen(Country);

				if (CountryCount==0)
				{
					CountryID = 0;
					CountryNames[0] = Country;
				}
				else
				{
					for (INT InsertPos=0; InsertPos<=CountryCount; InsertPos++)
						if ((InsertPos==CountryCount) || (Compare(CountryNames[InsertPos],Country)>0))
						{
							for (INT a=CountryCount; a>InsertPos; a--)
								CountryNames[a] = CountryNames[a-1];
							CountryID = InsertPos;
							CountryNames[InsertPos] = Country;
							for (UINT a=0; a<AirportCount; a++)
								if (Airports[a].CountryID>=InsertPos)
									Airports[a].CountryID++;
							break;
						}
				}
				CountryCount++;
			}

			Airports[AirportCount].Name = NamesAndCodes.Mid(8);
			Airports[AirportCount].CountryID = CountryID;

			if (strlen(Airports[AirportCount].Name)>maxAirportLength)
				maxAirportLength = strlen(Airports[AirportCount].Name);

			if (Coord=="")
			{
				Airports[AirportCount].Latitude = 0;
				Airports[AirportCount].Longitude = 0;
				Airports[AirportCount].Divider = 0;
			}
			else
			{
				curPos = Coord.Find(", ");
				Airports[AirportCount].Latitude = GetCoord(Coord.Mid(0, curPos));
				Airports[AirportCount].Longitude = GetCoord(Coord.Mid(curPos+2));
				Airports[AirportCount].Divider = 1;
			}

			AirportCount++;
		}
	}

	input.Close();

	// Metropolitan-Codes berechnen
	for (UINT a=0; a<AirportCount; a++)
		if (Airports[a].MetroCode==Airports[a].Code)
			for (UINT b=0; b<AirportCount; b++)
				if ((a!=b) && (Airports[a].MetroCode==Airports[b].MetroCode))
				{
					Airports[a].Latitude += Airports[b].Latitude;
					Airports[a].Longitude += Airports[b].Longitude;
					Airports[a].Divider++;
				}

	// Ausgeben
	CStdioFile output;
	output.Open(path+"..\\..\\FMCommDlg\\IATA_"+LanguageSuffix+".h", CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	output.WriteString("\n// Countries\n");
	tmpStr.Format("#define CountryCount_%s %i\n", LanguageSuffix, CountryCount);
	output.WriteString(tmpStr);
	tmpStr.Format("FMCountry Countries_%s[CountryCount_%s] = {\n", LanguageSuffix, LanguageSuffix);
	output.WriteString(tmpStr);
	for (INT a=0; a<CountryCount; a++)
	{
		CString Delimiter = (a<CountryCount-1) ? "," : "";
		tmpStr.Format("\t{ \"%s\" }%s\n", CountryNames[a], Delimiter);
		output.WriteString(tmpStr);
	}
	output.WriteString("};\n\n");

	output.WriteString("// Airports\n");
	tmpStr.Format("#define AirportCount_%s %i\n", LanguageSuffix, AirportCount);
	output.WriteString(tmpStr);
	tmpStr.Format("FMAirport Airports_%s[AirportCount_%s] = {\n", LanguageSuffix, LanguageSuffix);
	output.WriteString(tmpStr);
	for (UINT a=0; a<AirportCount; a++)
	{
		tmpStr.Format("\t{ %3d, \"%s\", \"%s\", \"%s\", ",
			Airports[a].CountryID, Airports[a].Code, Airports[a].MetroCode, Airports[a].Name);
		output.WriteString(tmpStr);
		CString Delimiter = (a<AirportCount-1) ? "," : "";
		tmpStr.Format("{ %f, %f } }%s\n",
			Airports[a].Latitude/Airports[a].Divider, Airports[a].Longitude/Airports[a].Divider, Delimiter);
		output.WriteString(tmpStr);
	}
	output.WriteString("};\n");

	output.Close();
}

INT _tmain(INT /*argc*/, TCHAR* /*argv[]*/, TCHAR* /*envp[]*/)
{
	// Pfad
	TCHAR szPathName[MAX_PATH];
	::GetModuleFileName(NULL, szPathName, MAX_PATH);
	LPTSTR pszFileName = _tcsrchr(szPathName, '\\')+1;
	*pszFileName = '\0';
	path = szPathName;
	maxCountryLength = maxAirportLength = 0;

	// Konvertieren
	ConvertFile("DE");
	ConvertFile("EN");

	cout << "Country: " << maxCountryLength+1;
	cout << "\nAirport: " << maxAirportLength+1;
	cin.get();

	return 0;
}
