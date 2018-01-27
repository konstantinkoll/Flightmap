
// CMainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "CCalendarFile.h"
#include "CExcelFile.h"
#include "CGlobeWnd.h"
#include "CGoogleEarthFile.h"
#include "CKitchen.h"
#include "CMainWnd.h"
#include "CMapFactory.h"
#include "CMapWnd.h"
#include "Flightmap.h"
#include "GlobeSettingsDlg.h"
#include "GoogleEarthSettingsDlg.h"
#include "MapSettingsDlg.h"
#include "StatisticsDlg.h"
#include "StatisticsSettingsDlg.h"
#include <winspool.h>


// CMainWnd
//

CIcons CMainWnd::m_LargeIconsSidebar;
CIcons CMainWnd::m_SmallIconsSidebar;
CIcons CMainWnd::m_LargeIconsTaskbar;
CIcons CMainWnd::m_SmallIconsTaskbar;

CMainWnd::CMainWnd()
	: CBackstageWnd()
{
	m_pItinerary = NULL;
	m_pWndDataGrid = NULL;
	m_pWndFileMenu = NULL;
}

BOOL CMainWnd::Create(CItinerary* pItinerary)
{
	m_pItinerary = pItinerary;

	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_APPLICATION));

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, CString((LPCSTR)IDR_APPLICATION), _T("Main"), CSize(0, 0), TRUE);
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	// Hide file menu
	if (m_pWndFileMenu && (pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_ESCAPE))
	{
		HideFileMenu();
		return TRUE;
	}

	return CBackstageWnd::PreTranslateMessage(pMsg);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_pWndDataGrid && m_pWndDataGrid->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CBackstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainWnd::HasDocumentSheet() const
{
	return m_pItinerary || m_pWndFileMenu;
}

void CMainWnd::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	if (m_pWndFileMenu)
	{
		m_pWndFileMenu->SetWindowPos(&wndTop, rectLayout.left, rectLayout.top, rectLayout.Width(), rectLayout.Height(), nFlags & ~SWP_NOZORDER);

		m_wndTaskbar.ShowWindow(SW_HIDE);

		if (m_pWndDataGrid)
		{
			m_pWndDataGrid->ShowWindow(SW_HIDE);
			m_pWndDataGrid->EnableWindow(FALSE);
		}
	}
	else
		if (m_pWndDataGrid)
		{
			const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
			m_wndTaskbar.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), TaskHeight, nFlags | SWP_SHOWWINDOW);

			m_pWndDataGrid->SetWindowPos(NULL, rectLayout.left, rectLayout.top+TaskHeight, rectLayout.Width(), rectLayout.Height()-TaskHeight, nFlags | SWP_SHOWWINDOW);
			m_pWndDataGrid->EnableWindow(TRUE);
		}
		else
		{
			m_wndTaskbar.ShowWindow(SW_HIDE);
		}
}

void CMainWnd::HideFileMenu()
{
	if (m_pWndFileMenu)
	{
		theApp.HideTooltip();

		m_wndSidebar.SetSelection();

		m_pWndFileMenu->DestroyWindow();
		delete m_pWndFileMenu;
		m_pWndFileMenu = NULL;

		CBackstageWnd::AdjustLayout();

		if (m_pWndDataGrid)
			m_pWndDataGrid->SetFocus();
	}
}

void CMainWnd::UpdateWindowStatus()
{
	// Set window caption
	CString Caption((LPCSTR)IDR_APPLICATION);

	if (m_pItinerary)
		m_pItinerary->InsertDisplayName(Caption);

	SetWindowText(Caption);

	// Handle data grid
	if (m_pItinerary)
	{
		if (m_pWndDataGrid)
		{
			m_pWndDataGrid->SetItinerary(m_pItinerary);
		}
		else
		{
			m_pWndDataGrid = new CDataGrid();
			m_pWndDataGrid->Create(m_pItinerary, this, 3);

			InvalidateCaption(TRUE);

			// Adjust layout
			CBackstageWnd::AdjustLayout();

			m_pWndDataGrid->EnsureVisible();
		}
	}
	else
	{
		if (m_pWndDataGrid)
		{
			m_pWndDataGrid->DestroyWindow();
			delete m_pWndDataGrid;
			m_pWndDataGrid = NULL;

			InvalidateCaption(TRUE);

			// Adjust layout
			CBackstageWnd::AdjustLayout();
		}
	}

	// Set focus
	SetFocus();
}

void CMainWnd::SetItinerary(CItinerary* pItinerary)
{
	CItinerary* pVictim = m_pItinerary;

	m_pItinerary = pItinerary;

	UpdateWindowStatus();
	HideFileMenu();

	delete pVictim;
}

void CMainWnd::Open(const CString& Path)
{
	if (CloseFile())
		SetItinerary(new CItinerary(Path));
}

BOOL CMainWnd::CloseFile()
{
	if (m_pItinerary && m_pItinerary->m_IsModified)
		switch (FMMessageBox(this, CString((LPCSTR)IDS_NOTSAVED), m_pItinerary->m_DisplayName, MB_YESNOCANCEL | MB_ICONWARNING))
		{
		case IDCANCEL:
			return FALSE;

		case IDYES:
			OnFileSave();

			if (m_pItinerary->m_IsModified)
				return FALSE;
		}

	return TRUE;
}


// Computation, export and printing

CKitchen* CMainWnd::GetKitchen(BOOL Limit, BOOL Selected, BOOL MergeMetro) const
{
	CKitchen* pKitchen = new CKitchen(m_pItinerary, MergeMetro);

	if (m_pItinerary)
	{
		Selected &= m_pWndDataGrid->HasSelection();

		const UINT FlightCount = m_pItinerary->GetFlightCount(Limit);

		for (UINT a=0; a<FlightCount; a++)
			if (!Selected || m_pWndDataGrid->IsSelected(a))
				if ((m_pItinerary->m_Flights[a].Flags & AIRX_Cancelled)==0)
					pKitchen->AddFlight(m_pItinerary->m_Flights[a], m_pItinerary->GetGPSPath(a));
	}

	return pKitchen;
}

CBitmap* CMainWnd::GetMap(BOOL Selected, BOOL MergeMetro) const
{
	return CMapFactory(theApp.m_MapSettings).RenderMap(GetKitchen(FALSE, Selected, MergeMetro));
}


void CMainWnd::ExportCalendar(const CString& Path)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CCalendarFile File;
	if (!File.Open(Path, m_pItinerary->m_Metadata.Comments, m_pItinerary->m_Metadata.Title))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		try
		{
			const UINT FlightCount = m_pItinerary->GetFlightCount();

			for (UINT a=0; a<FlightCount; a++)
				File.WriteFlight(m_pItinerary->m_Flights[a]);
		}
		catch(CFileException ex)
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}

		File.Close();
	}
}

void CMainWnd::ExportExcel(const CString& Path)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CExcelFile File;
	if (!File.Open(Path))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		try
		{
			const UINT FlightCount = m_pItinerary->GetFlightCount();

			for (UINT a=0; a<FlightCount; a++)
				File.WriteFlight(m_pItinerary->m_Flights[a]);
		}
		catch(CFileException ex)
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}

		File.Close();
	}
}

BOOL CMainWnd::ExportGoogleEarth(const CString& Path, BOOL Selected, BOOL MergeMetro)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CGoogleEarthFile File;
	if (!File.Open(Path, m_pItinerary->m_DisplayName))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);

		return FALSE;
	}
	else
	{
		CKitchen* pKitchen = GetKitchen(TRUE, Selected, MergeMetro);

		try
		{
			File.WriteRoutes(pKitchen, theApp.m_GoogleEarthUseCount, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClampHeight);
			File.WriteAirports(pKitchen);
		}
		catch(CFileException ex)
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}

		File.Close();
		delete pKitchen;

		return TRUE;
	}
}

void CMainWnd::ExportMap(const CString& Path, GUID guidFileType)
{
	CWaitCursor WaitCursor;

	theApp.SaveBitmap(GetMap(), Path, guidFileType);
}

void CMainWnd::ExportMap(DWORD FilterIndex)
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	dlg.m_ofn.nFilterIndex = FilterIndex;

	if (dlg.DoModal()==IDOK)
		Export(dlg.GetPathName(), dlg.GetFileExt().MakeLower());
}

void CMainWnd::ExportText(const CString& Path)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	FILE* fStream;
	if (_tfopen_s(&fStream, Path, _T("wt,ccs=UTF-8")))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		CStdioFile File(fStream);

		try
		{
			const UINT FlightCount = m_pItinerary->GetFlightCount();

			for (UINT a=0; a<FlightCount; a++)
				File.WriteString(m_pItinerary->Flight2Text(a));
		}
		catch(CFileException ex)
		{
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}

		File.Close();
	}
}

void CMainWnd::Export(const CString& Path, const CString& Extension)
{
	if (Extension==_T("airx"))
	{
		m_pItinerary->SaveAIRX(Path, m_pWndDataGrid->GetCurrentRow());

		UpdateWindowStatus();
	}

	if (Extension==_T("bmp"))
		ExportMap(Path, ImageFormatBMP);

	if (Extension==_T("csv"))
		ExportExcel(Path);

	if (Extension==_T("ics"))
		ExportCalendar(Path);

	if (Extension==_T("kml"))
		ExportGoogleEarth(Path);

	if (Extension==_T("jpg"))
		ExportMap(Path, ImageFormatJPEG);

	if (Extension==_T("png"))
		ExportMap(Path, ImageFormatPNG);

	if (Extension==_T("tif"))
		ExportMap(Path, ImageFormatTIFF);

	if (Extension==_T("txt"))
		ExportText(Path);
}

void CMainWnd::SaveAs(DWORD FilterIndex)
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_AIRX, _T("airx"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_CSV, _T("csv"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_ICS, _T("ics"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_KML, _T("kml"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TXT, _T("txt"), TRUE);

	CFileDialog dlg(FALSE, _T("airx"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	dlg.m_ofn.nFilterIndex = FilterIndex;

	if (dlg.DoModal()==IDOK)
	{
		Export(dlg.GetPathName(), dlg.GetFileExt().MakeLower());

		HideFileMenu();
	}
}

void CMainWnd::Print(const PRINTDLGEX& pdex)
{
	ASSERT(m_pItinerary);

	HideFileMenu();

	// Device Context
	CDC dc;
	dc.Attach(pdex.hDC);

	// Landscape
	ASSERT(pdex.hDevMode);
	LPDEVMODE lpDevMode = (LPDEVMODE)GlobalLock(pdex.hDevMode);

	lpDevMode->dmOrientation = DMORIENT_LANDSCAPE;
	lpDevMode->dmFields |= DM_ORIENTATION;
	dc.ResetDC(lpDevMode);

	GlobalUnlock(pdex.hDevMode);

	// Document
	DOCINFO di;
	ZeroMemory(&di, sizeof(di));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = m_pItinerary->m_Metadata.Title[0]!=L'\0' ? m_pItinerary->m_Metadata.Title : m_pItinerary->m_DisplayName.GetBuffer();

	// Printing
	dc.SetMapMode(MM_TEXT);
	dc.SetBkMode(TRANSPARENT);

	const INT Width = dc.GetDeviceCaps(HORZRES);
	const INT Height = dc.GetDeviceCaps(VERTRES);
	const DOUBLE Spacer = (Width/40.0);

	CRect rect(0, 0, Width, Height);
	rect.DeflateRect((INT)Spacer, (INT)Spacer);

	if (SUCCEEDED(dc.StartDoc(&di)))
	{
		if (SUCCEEDED(dc.StartPage()))
		{
			CRect rectPage(rect);

			theApp.PrintPageHeader(dc, rectPage, Spacer, di);

			CFont Font;
			Font.CreatePointFont(120, _T("Tahoma"), &dc);
			CFont* pOldFont = dc.SelectObject(&Font);

			CPen pen(PS_SOLID, 4, (COLORREF)0x606060);
			CPen* pOldPen = dc.SelectObject(&pen);

			const UINT cColumns = 9;
			const UINT ColumnIDs[cColumns] = { 0, 3, 1, 7, 8, 14, 16, 10, 22 };

			INT ColumnWidths[cColumns];
			INT TotalWidth = 0;
			for (UINT a=0; a<cColumns; a++)
				TotalWidth += FMAttributes[ColumnIDs[a]].DefaultColumnWidth;
			for (UINT a=0; a<cColumns; a++)
				ColumnWidths[a] = (INT)((DOUBLE)FMAttributes[ColumnIDs[a]].DefaultColumnWidth*(DOUBLE)rectPage.Width()/(DOUBLE)TotalWidth);

			const INT TextHeight = dc.GetTextExtent(_T("Wy")).cy;
			const INT LineHeight = (INT)((DOUBLE)TextHeight*1.2);

			BOOL FirstRow = TRUE;

			for (UINT a=0; a<m_pItinerary->m_Flights.m_ItemCount; a++)
			{
				if (pdex.Flags & PD_SELECTION)
					if (!m_pWndDataGrid->IsSelected(a))
						continue;

				if (rectPage.Height()<LineHeight)
				{
					if (dc.EndPage()<0)
						goto Ende;
					if (dc.StartPage()<0)
						goto Ende;

					rectPage = rect;
					FirstRow = TRUE;
				}

				if (FirstRow)
				{
					FirstRow = FALSE;
				}
				else
				{
					const INT Y = rectPage.top-(LineHeight-TextHeight)/2;
					dc.MoveTo(rectPage.left, Y);
					dc.LineTo(rectPage.right, Y);
				}

				INT Left = rectPage.left;
				for (UINT b=0; b<cColumns; b++)
				{
					const INT Width = ColumnWidths[b];
					CRect rectItem(Left, rectPage.top, Left+Width, rectPage.bottom);

					WCHAR tmpBuf[256];
					AttributeToString(m_pItinerary->m_Flights[a], ColumnIDs[b], tmpBuf, 256);
					dc.DrawText(tmpBuf, -1, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_TOP | DT_LEFT);

					Left += Width;
				}

				rectPage.top += LineHeight;
			}

Ende:
			dc.SelectObject(pOldPen);
			dc.SelectObject(pOldFont);
			dc.EndPage();
		}

		dc.EndDoc();
	}

	if (pdex.hDevMode)
		GlobalFree(pdex.hDevMode);

	if (pdex.hDevNames)
		GlobalFree(pdex.hDevNames);
}


BEGIN_MESSAGE_MAP(CMainWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 1, OnRequestTooltipData)

	ON_MESSAGE_VOID(WM_DISTANCESETTINGSCHANGED, OnDistanceSettingsChanged)

	ON_COMMAND(IDM_FILE_NEW, OnFileNew)
	ON_COMMAND(IDM_FILE_NEWSAMPLE1, OnFileNewSample1)
	ON_COMMAND(IDM_FILE_NEWSAMPLE2, OnFileNewSample2)
	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_OPENRECENT, OnFileOpenRecent)
	ON_COMMAND(IDM_FILE_SAVE, OnFileSave)
	ON_COMMAND(IDM_FILE_SAVEAS, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_SAVEAS_CSV, OnFileSaveCSV)
	ON_COMMAND(IDM_FILE_SAVEAS_TXT, OnFileSaveTXT)
	ON_COMMAND(IDM_FILE_SAVEAS_OTHER, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_PRINT, OnFilePrint)
	ON_COMMAND(IDM_FILE_PRINTQUICK, OnFilePrintQuick)
	ON_COMMAND(IDM_FILE_CLOSE, OnFileClose)

	ON_COMMAND(IDM_SIDEBAR_FILEMENU, OnFileMenu)
	ON_COMMAND(IDM_SIDEBAR_MAP_OPEN, OnMapOpen)
	ON_COMMAND(IDM_SIDEBAR_MAP_SETTINGS, OnMapSettings)
	ON_COMMAND(IDM_SIDEBAR_GLOBE_OPEN, OnGlobeOpen)
	ON_COMMAND(IDM_SIDEBAR_GLOBE_SETTINGS, OnGlobeSettings)
	ON_COMMAND(IDM_SIDEBAR_GOOGLEEARTH_OPEN, OnGoogleEarthOpen)
	ON_COMMAND(IDM_SIDEBAR_GOOGLEEARTH_SETTINGS, OnGoogleEarthSettings)
	ON_COMMAND(IDM_SIDEBAR_STATISTICS_OPEN, OnStatisticsOpen)
	ON_COMMAND(IDM_SIDEBAR_STATISTICS_SETTINGS, OnStatisticsSettings)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SIDEBAR_FILEMENU, IDM_SIDEBAR_STATISTICS_SETTINGS, OnUpdateSidebarCommands)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	hAccelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDA_ACCELERATOR_MAIN));

	// Sidebar
	if (!m_wndSidebar.Create(this, m_LargeIconsSidebar, m_SmallIconsSidebar, IDB_SIDEBAR_16, 1, FALSE))
		return -1;

	m_wndSidebar.AddCommand(IDM_SIDEBAR_FILEMENU, 0);
	m_wndSidebar.AddCaption(IDR_MAP);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_MAP_OPEN, 1);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_MAP_SETTINGS, 2);
	m_wndSidebar.AddCaption(IDR_GLOBE);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GLOBE_OPEN, 3);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GLOBE_SETTINGS, 2);
	m_wndSidebar.AddCaption(IDS_GOOGLEEARTH);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GOOGLEEARTH_OPEN, 4);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GOOGLEEARTH_SETTINGS, 2);
	m_wndSidebar.AddCaption(IDS_STATISTICS);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_STATISTICS_OPEN, 5);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_STATISTICS_SETTINGS, 2);

	SetSidebar(&m_wndSidebar);

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIconsTaskbar, m_SmallIconsTaskbar, IDB_TASKS_ITINERARY_16, 2))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_DATAGRID_EDITFLIGHT, 0, TRUE);
	m_wndTaskbar.AddButton(IDM_DATAGRID_ADDROUTE, 1);
	m_wndTaskbar.AddButton(IDM_DATAGRID_FINDREPLACE, 2);
	m_wndTaskbar.AddButton(IDM_DATAGRID_FILTER, 3);
	m_wndTaskbar.AddButton(IDM_DATAGRID_INSERTROW, 4);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_PURCHASE, 5, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ENTERLICENSEKEY, 6, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 8, TRUE, TRUE);

	UpdateWindowStatus();

	return 0;
}

void CMainWnd::OnClose()
{
	if (m_pItinerary && !CloseFile())
		return;

	CBackstageWnd::OnClose();
}

void CMainWnd::OnDestroy()
{
	delete m_pItinerary;

	if (m_pWndDataGrid)
	{
		m_pWndDataGrid->DestroyWindow();
		delete m_pWndDataGrid;
	}

	if (m_pWndFileMenu)
	{
		m_pWndFileMenu->DestroyWindow();
		delete m_pWndFileMenu;
	}

	CBackstageWnd::OnDestroy();
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (m_pWndDataGrid)
		m_pWndDataGrid->SetFocus();
}

void CMainWnd::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	CString tmpStr((LPCSTR)pTooltipData->Item);

	INT Pos = tmpStr.Find(L'\n');
	if (Pos!=-1)
	{
		CString Hint = tmpStr.Left(Pos);
		
		if (Hint.GetLength()>40)
		{
			Pos = Hint.Find(L' ', Hint.GetLength()/2);
			if (Pos!=-1)
				Hint.SetAt(Pos, L'\n');
		}

		wcscpy_s(pTooltipData->Hint, 4096, Hint);
	}

	*pResult = TRUE;
}

void CMainWnd::OnDistanceSettingsChanged()
{
	if (m_pWndDataGrid)
		m_pWndDataGrid->Invalidate();
}


// File commands

void CMainWnd::OnFileNew()
{
	if (CloseFile())
		SetItinerary(new CItinerary());
}

void CMainWnd::OnFileNewSample1()
{
	if (CloseFile())
	{
		CItinerary* pItinerary = new CItinerary();
		pItinerary->NewSampleAtlantic();

		SetItinerary(pItinerary);
	}
}

void CMainWnd::OnFileNewSample2()
{
	if (CloseFile())
	{
		CItinerary* pItinerary = new CItinerary();
		pItinerary->NewSamplePacific();

		SetItinerary(pItinerary);
	}
}

void CMainWnd::OnFileOpen()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_AIRX, _T("airx; *.air"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_CSV, _T("csv"), TRUE);

	CFileDialog dlg(TRUE, _T("airx"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, Extensions, this);
	if (dlg.DoModal()==IDOK)
		Open(dlg.GetPathName());
}

void CMainWnd::OnFileOpenRecent()
{
	if (m_pWndFileMenu)
	{
		LPCWSTR Path = m_pWndFileMenu->GetSelectedFilePath();
		ASSERT(Path);

		if (Path[0]!=L'\0')
			Open(Path);
	}
}

void CMainWnd::OnFileSave()
{
	ASSERT(m_pItinerary);

	if (m_pItinerary->m_FileName.IsEmpty())
	{
		SaveAs();
	}
	else
	{
		m_pItinerary->SaveAIRX(m_pItinerary->m_FileName, m_pWndDataGrid->GetCurrentRow());
		UpdateWindowStatus();

		HideFileMenu();
	}
}

void CMainWnd::OnFileSaveAs()
{
	ASSERT(m_pItinerary);

	SaveAs();
}

void CMainWnd::OnFileSaveCSV()
{
	ASSERT(m_pItinerary);

	SaveAs(3);
}

void CMainWnd::OnFileSaveTXT()
{
	ASSERT(m_pItinerary);

	SaveAs(9);
}

void CMainWnd::OnFilePrint()
{
	ASSERT(m_pItinerary);

	DWORD Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC;
	if (m_pWndDataGrid->HasSelection())
		Flags &= ~PD_NOSELECTION;

	CPrintDialogEx dlg(Flags, this);
	if (SUCCEEDED(dlg.DoModal()) && (dlg.m_pdex.dwResultAction==PD_RESULT_PRINT))
		Print(dlg.m_pdex);
}

void CMainWnd::OnFilePrintQuick()
{
	ASSERT(m_pItinerary);

	CPrintDialogEx dlg(FALSE);

	ZeroMemory(&dlg.m_pdex, sizeof(dlg.m_pdex));
	dlg.m_pdex.lStructSize = sizeof(dlg.m_pdex);
	dlg.m_pdex.hwndOwner = GetSafeHwnd();
	dlg.m_pdex.Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC;
	dlg.m_pdex.nStartPage = (DWORD)-1;

	if (dlg.GetDefaults())
		Print(dlg.m_pdex);
}

void CMainWnd::OnFileClose()
{
	if (CloseFile())
		SetItinerary(NULL);
}


// Sidebar commands

void CMainWnd::OnFileMenu()
{
	if (!m_pWndFileMenu)
	{
		m_wndSidebar.SetSelection(IDM_SIDEBAR_FILEMENU);

		m_pWndFileMenu = new CFileMenu();
		m_pWndFileMenu->Create(this, 4, m_pItinerary);

		CBackstageWnd::AdjustLayout();

		m_pWndFileMenu->SetFocus();
	}
	else
	{
		HideFileMenu();
	}
}

void CMainWnd::OnMapOpen()
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_EXPIRED, this);

	CWaitCursor WaitCursor;

	CBitmap* pBitmap = GetMap(TRUE, theApp.m_MapMergeMetro);

	CMapWnd* pFrameWnd = new CMapWnd();
	pFrameWnd->Create();
	pFrameWnd->SetBitmap(pBitmap, m_pItinerary);
}

void CMainWnd::OnMapSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_MAP_SETTINGS);

	MapSettingsDlg(this).DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnGlobeOpen()
{
	ASSERT(m_pItinerary);

	CKitchen* pKitchen = GetKitchen(TRUE, TRUE, theApp.m_GlobeMergeMetro);

	CGlobeWnd* pFrameWnd = new CGlobeWnd();
	pFrameWnd->Create();
	pFrameWnd->SetFlights(pKitchen);
}

void CMainWnd::OnGlobeSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_GLOBE_SETTINGS);

	GlobeSettingsDlg(this).DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnGoogleEarthOpen()
{
	ASSERT(m_pItinerary);

	// Create file name
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	srand(rand());

	CString TempName;
	TempName.Format(_T("%sFlightmap%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	// Export .kml file and open Google Earth
	if (ExportGoogleEarth(TempName, TRUE, theApp.m_GoogleEarthMergeMetro))
		ShellExecute(GetSafeHwnd(), _T("open"), TempName, NULL, NULL, SW_SHOWNORMAL);
}

void CMainWnd::OnGoogleEarthSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_GOOGLEEARTH_SETTINGS);

	GoogleEarthSettingsDlg(this).DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnStatisticsOpen()
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_COUNTER, this);

	StatisticsDlg(m_pItinerary, this).DoModal();
}

void CMainWnd::OnStatisticsSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_STATISTICS_SETTINGS);

	StatisticsSettingsDlg(this).DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnUpdateSidebarCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_pWndFileMenu==NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_SIDEBAR_FILEMENU:
		bEnable = TRUE;
		break;

	case IDM_SIDEBAR_GOOGLEEARTH_OPEN:
		bEnable &= (theApp.m_PathGoogleEarth[0]!=L'\0');

	case IDM_SIDEBAR_MAP_OPEN:
	case IDM_SIDEBAR_GLOBE_OPEN:
	case IDM_SIDEBAR_STATISTICS_OPEN:
		bEnable &= (m_pItinerary!=NULL);
		break;
	}

	pCmdUI->Enable(bEnable);
}
