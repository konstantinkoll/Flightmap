// ConvertRoutes.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "ConvertRoutes.h"
#include <io.h>
#include <wchar.h>


// Das einzige Anwendungsobjekt
//

CWinApp theApp;

using namespace std;

void ConvertFile(CString InputFN, CString OutputFN)
{
	CString tmpStr;

	// Einlesen
	CStdioFile input;
	if (!input.Open(InputFN, CFile::modeRead | CFile::typeText))
	{
		cout << _T("Cannot open ")+InputFN;
		return;
	}
	input.ReadString(tmpStr);

	// Ausgeben
	CStdioFile output;
	if (!output.Open(OutputFN, CFile::modeWrite | CFile::modeCreate | CFile::typeText))
	{
		cout << _T("Cannot open ")+OutputFN;
		return;
	}
	output.WriteString(_T("Description;From;To\n"));

	UINT Routes = 0;
	UINT Legs = 0;

	while (input.ReadString(tmpStr))
	{
		INT curPos = 0;
		CString Description = tmpStr.Tokenize(";", curPos);
		Routes++;

		tmpStr.Delete(0, curPos);

		CString From(_T(""));
		CString To(_T(""));

		while (!tmpStr.IsEmpty())
		{
			INT pos = tmpStr.FindOneOf(_T("/-;"));
			if (pos==-1)
				pos = tmpStr.GetLength()+1;

			if (pos>=3)
			{
				To = tmpStr;
				To.Delete(3, To.GetLength()-3);
				tmpStr.Delete(0, pos+1);

				if (!From.IsEmpty())
				{
					output.WriteString(Description+_T(";")+From+_T(";")+To+_T("\n"));
					Legs++;
				}
				From = To;
			}
			else
			{
				break;
			}
		}
	}

	input.Close();
	output.Close();

	tmpStr.Format(_T("Converted %d routes to %d legs\n"), Routes, Legs);
	cout << tmpStr;
}

INT _tmain(INT argc, TCHAR* argv[], TCHAR* /*envp[]*/)
{
	// MFC initialisieren und drucken. Bei Fehlschlag Fehlermeldung aufrufen.
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: Den Fehlercode an Ihre Anforderungen anpassen.
		_tprintf(_T("Schwerwiegender Fehler bei der MFC-Initialisierung\n"));
		return 1;
	}

	CString InputFN(argc>=2 ? argv[1] : _T("input.csv"));
	CString OutputFN(argc>=3 ? argv[2] : _T("output.csv"));
	ConvertFile(InputFN, OutputFN);

	return 0;
}
