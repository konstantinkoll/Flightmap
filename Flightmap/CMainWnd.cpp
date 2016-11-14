
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

CMainWnd::~CMainWnd()
{
	delete m_pItinerary;
}

BOOL CMainWnd::Create(CItinerary* pItinerary)
{
	m_pItinerary = pItinerary;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_APPLICATION));

	CString Caption((LPCSTR)IDR_APPLICATION);

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, Caption, _T("Main"), CSize(0, 0), TRUE);
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	// Hide file menu
	if (m_pWndFileMenu)
		if ((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_ESCAPE))
		{
			HideFileMenu();
			return TRUE;
		}

	return CBackstageWnd::PreTranslateMessage(pMsg);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_pWndDataGrid)
		if (m_pWndDataGrid->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	return CBackstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainWnd::GetLayoutRect(LPRECT lpRect) const
{
	CBackstageWnd::GetLayoutRect(lpRect);

	return (m_pItinerary!=NULL) || (m_pWndFileMenu!=NULL);
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
		FMGetApp()->HideTooltip();

		m_ForceSidebarAlwaysVisible = FALSE;
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
		if (!m_pItinerary->m_DisplayName.IsEmpty())
		{
			Caption.Insert(0, _T(" - "));
			Caption.Insert(0, m_pItinerary->m_DisplayName);
		}

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

		m_ShowSidebar = FALSE;
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
	if (m_pItinerary)
		if (m_pItinerary->m_IsModified)
		{
			CString Text((LPCSTR)IDS_NOTSAVED);

			switch (FMMessageBox(this, Text, m_pItinerary->m_DisplayName, MB_YESNOCANCEL | MB_ICONWARNING))
			{
			case IDCANCEL:
				return FALSE;

			case IDYES:
				OnFileSave();

				if (m_pItinerary->m_IsModified)
					return FALSE;
			}
		}

	return TRUE;
}

CKitchen* CMainWnd::GetKitchen(BOOL Limit, BOOL Selected, BOOL MergeMetro)
{
	CKitchen* pKitchen = new CKitchen(m_pItinerary ? m_pItinerary->m_Metadata.Title[0]!=L'\0' ? m_pItinerary->m_Metadata.Title : m_pItinerary->m_DisplayName : _T(""), MergeMetro);

	if (m_pItinerary)
	{
		Selected &= m_pWndDataGrid->HasSelection();

		UINT Count = m_pItinerary->m_Flights.m_ItemCount;
		if (!FMIsLicensed() && Limit)
			Count = min(Count, 10);

		for (UINT a=0; a<Count; a++)
		{
			if (Selected)
				if (!m_pWndDataGrid->IsSelected(a))
					continue;

			if (m_pItinerary->m_Flights[a].Flags & AIRX_Cancelled)
				continue;

			pKitchen->AddFlight(m_pItinerary->m_Flights[a], m_pItinerary->GetGPSPath(a));
		}
	}

	return pKitchen;
}

CBitmap* CMainWnd::GetMap(BOOL Selected, BOOL MergeMetro)
{
	CMapFactory f(&theApp.m_MapSettings);

	return f.RenderMap(GetKitchen(FALSE, Selected, MergeMetro));
}

void CMainWnd::ExportMap(const CString& Filename, GUID guidFileType, BOOL Selected, BOOL MergeMetro)
{
	CWaitCursor csr;

	theApp.SaveBitmap(GetMap(Selected, MergeMetro), Filename, guidFileType);
}

void CMainWnd::ExportExcel(const CString& Filename)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CExcelFile f;

	if (!f.Open(Filename))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		for (UINT a=0; a<Limit; a++)
			f.WriteRoute(m_pItinerary->m_Flights[a]);

		f.Close();
	}
}

void CMainWnd::ExportCalendar(const CString& Filename)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	CCalendarFile f;

	if (!f.Open(Filename, m_pItinerary->m_Metadata.Comments, m_pItinerary->m_Metadata.Title))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		for (UINT a=0; a<Limit; a++)
			f.WriteRoute(m_pItinerary->m_Flights[a]);

		f.Close();
	}
}

BOOL CMainWnd::ExportGoogleEarth(const CString& Filename, BOOL UseCount, BOOL UseColors, BOOL ClampHeight, BOOL Selected, BOOL MergeMetro)
{
	CGoogleEarthFile f;

	theApp.ShowNagScreen(NAG_FORCE, this);

	if (!f.Open(Filename, m_pItinerary ? m_pItinerary->m_DisplayName : NULL))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
		return FALSE;
	}
	else
	{
		BOOL Result = FALSE;
		CKitchen* pKitchen = GetKitchen(TRUE, Selected, MergeMetro);

		try
		{
			f.WriteRoutes(pKitchen, UseCount, UseColors, ClampHeight, FALSE);

			CFlightAirports::CPair* pPair = pKitchen->m_FlightAirports.PGetFirstAssoc();
			while (pPair)
			{
				f.WriteAirport(pPair->value.pAirport);

				pPair = pKitchen->m_FlightAirports.PGetNextAssoc(pPair);
			}

			f.Close();
			Result = TRUE;
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}

		delete pKitchen;
		return Result;
	}
}

void CMainWnd::ExportText(const CString& Filename)
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_FORCE, this);

	FILE *fStream;
	if (_tfopen_s(&fStream, Filename, _T("wt,ccs=UTF-8")))
	{
		FMErrorBox(this, IDS_DRIVENOTREADY);
	}
	else
	{
		UINT Limit = FMIsLicensed() ? m_pItinerary->m_Flights.m_ItemCount : min(m_pItinerary->m_Flights.m_ItemCount, 10);

		CStdioFile f(fStream);
		try
		{
			for (UINT a=0; a<Limit; a++)
				f.WriteString(m_pItinerary->Flight2Text(a));

			f.Close();
		}
		catch(CFileException ex)
		{
			f.Close();
			FMErrorBox(this, IDS_DRIVENOTREADY);
		}
	}
}

void CMainWnd::ExportMap(DWORD FilterIndex, BOOL Selected, BOOL MergeMetro)
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	dlg.m_ofn.nFilterIndex = FilterIndex;
	if (dlg.DoModal()==IDOK)
	{
		CString ext = dlg.GetFileExt();

		if (ext==_T("bmp"))
		{
			ExportMap(dlg.GetPathName(), ImageFormatBMP, Selected, MergeMetro);
		}
		else
			if (ext==_T("jpg"))
			{
				ExportMap(dlg.GetPathName(), ImageFormatJPEG, Selected, MergeMetro);
			}
			else
				if (ext==_T("png"))
				{
					ExportMap(dlg.GetPathName(), ImageFormatPNG, Selected, MergeMetro);
				}
				else
					if (ext==_T("tif"))
					{
						ExportMap(dlg.GetPathName(), ImageFormatTIFF, Selected, MergeMetro);
					}
	}
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
		CString Ext = dlg.GetFileExt().MakeLower();
		if (Ext==_T("airx"))
		{
			m_pItinerary->m_Metadata.CurrentRow = m_pWndDataGrid->GetCurrentRow();

			m_pItinerary->SaveAIRX(dlg.GetPathName());
			UpdateWindowStatus();
		}

		if (Ext==_T("bmp"))
			ExportMap(dlg.GetPathName(), ImageFormatBMP);

		if (Ext==_T("csv"))
			ExportExcel(dlg.GetPathName());

		if (Ext==_T("ics"))
			ExportCalendar(dlg.GetPathName());

		if (Ext==_T("kml"))
			ExportGoogleEarth(dlg.GetPathName(), theApp.m_GoogleEarthUseCount, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClampHeight);

		if (Ext==_T("jpg"))
			ExportMap(dlg.GetPathName(), ImageFormatJPEG);

		if (Ext==_T("png"))
			ExportMap(dlg.GetPathName(), ImageFormatPNG);

		if (Ext==_T("tif"))
			ExportMap(dlg.GetPathName(), ImageFormatTIFF);

		if (Ext==_T("txt"))
			ExportText(dlg.GetPathName());

		HideFileMenu();
	}
}

void CMainWnd::Print(PRINTDLGEX pdex)
{
	ASSERT(m_pItinerary);

	HideFileMenu();

	// Device Context
	CDC dc;
	dc.Attach(pdex.hDC);

	// Landscape
	LPDEVMODE lp = (LPDEVMODE)GlobalLock(pdex.hDevMode);
	ASSERT(lp);
	lp->dmOrientation = DMORIENT_LANDSCAPE;
	lp->dmFields |= DM_ORIENTATION;
	dc.ResetDC(lp);
	GlobalUnlock(pdex.hDevMode);

	// Document
	DOCINFO di;
	ZeroMemory(&di, sizeof(di));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = m_pItinerary->m_Metadata.Title[0]!=L'\0' ? m_pItinerary->m_Metadata.Title : m_pItinerary->m_DisplayName.GetBuffer();

	// Printing
	dc.SetMapMode(MM_TEXT);
	dc.SetBkMode(TRANSPARENT);

	INT w = dc.GetDeviceCaps(HORZRES);
	INT h = dc.GetDeviceCaps(VERTRES);
	const DOUBLE Spacer = (w/40.0);

	CRect rect(0, 0, w, h);
	rect.DeflateRect((INT)Spacer, (INT)Spacer);

	if (dc.StartDoc(&di)>=0)
	{
		if (dc.StartPage()>=0)
		{
			CRect rectPage(rect);

			theApp.PrintPageHeader(dc, rectPage, Spacer, di);

			CFont fnt;
			fnt.CreatePointFont(120, _T("Tahoma"), &dc);
			CFont* pOldFont = dc.SelectObject(&fnt);

			CPen pen(PS_SOLID, 4, (COLORREF)0x606060);
			CPen* pOldPen = dc.SelectObject(&pen);

			const UINT cColumns = 9;
			const UINT ColumnIDs[cColumns] = { 0, 3, 1, 7, 8, 14, 16, 10, 22 };

			INT ColumnWidths[cColumns];
			INT TotalWidth = 0;
			for (UINT a=0; a<cColumns; a++)
				TotalWidth += FMAttributes[ColumnIDs[a]].RecommendedWidth;
			for (UINT a=0; a<cColumns; a++)
				ColumnWidths[a] = (INT)((DOUBLE)FMAttributes[ColumnIDs[a]].RecommendedWidth*(DOUBLE)rectPage.Width()/(DOUBLE)TotalWidth);

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
					const INT z = rectPage.top-(LineHeight-TextHeight)/2;
					dc.MoveTo(rectPage.left, z);
					dc.LineTo(rectPage.right, z);
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
	ON_REGISTERED_MESSAGE(theApp.m_DistanceSettingChangedMsg, OnDistanceSettingChanged)

	ON_COMMAND(IDM_FILE_NEW, OnFileNew)
	ON_COMMAND(IDM_FILE_NEWSAMPLE1, OnFileNewSample1)
	ON_COMMAND(IDM_FILE_NEWSAMPLE2, OnFileNewSample2)
	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_OPENRECENT, OnFileOpenRecent)
	ON_COMMAND(IDM_FILE_SAVE, OnFileSave)
	ON_COMMAND(IDM_FILE_SAVEAS, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_SAVEAS_CSV, OnFileSaveCSV)
	ON_COMMAND(IDM_FILE_SAVEAS_TXT, OnFileSaveTXT)
	ON_COMMAND(IDM_FILE_SAVEAS_OTHER, OnFileSaveOther)
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
	m_wndSidebar.AddCommand(IDM_SIDEBAR_MAP_SETTINGS, -1);
	m_wndSidebar.AddCaption(IDR_GLOBE);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GLOBE_OPEN, 2);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GLOBE_SETTINGS, -1);
	m_wndSidebar.AddCaption(IDS_GOOGLEEARTH);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GOOGLEEARTH_OPEN, 3);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_GOOGLEEARTH_SETTINGS, -1);
	m_wndSidebar.AddCaption(IDS_STATISTICS);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_STATISTICS_OPEN, 4);
	m_wndSidebar.AddCommand(IDM_SIDEBAR_STATISTICS_SETTINGS, -1);

	SetSidebar(&m_wndSidebar);

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIconsTaskbar, m_SmallIconsTaskbar, IDB_TASKS_ITINERARY_16, 2))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_TOGGLESIDEBAR, 0, TRUE, FALSE, TRUE);
	m_wndTaskbar.AddButton(IDM_DATAGRID_EDITFLIGHT, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_DATAGRID_ADDROUTE, 2);
	m_wndTaskbar.AddButton(IDM_DATAGRID_FINDREPLACE, 3);
	m_wndTaskbar.AddButton(IDM_DATAGRID_FILTER, 4);
	m_wndTaskbar.AddButton(IDM_DATAGRID_INSERTROW, 5);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_PURCHASE, 6, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ENTERLICENSEKEY, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 9, TRUE, TRUE);

	UpdateWindowStatus();

	return 0;
}

void CMainWnd::OnClose()
{
	if (m_pItinerary)
		if (!CloseFile())
			return;

	CBackstageWnd::OnClose();
}

void CMainWnd::OnDestroy()
{
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

LRESULT CMainWnd::OnDistanceSettingChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	theApp.m_UseStatuteMiles = (BOOL)wParam;

	if (m_pWndDataGrid)
		m_pWndDataGrid->Invalidate();

	return NULL;
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
		CString Path = m_pWndFileMenu->GetSelectedRecentFilePath();

		if (!Path.IsEmpty())
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
		m_pItinerary->m_Metadata.CurrentRow = m_pWndDataGrid->GetCurrentRow();

		m_pItinerary->SaveAIRX(m_pItinerary->m_FileName);
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

void CMainWnd::OnFileSaveOther()
{
	ASSERT(m_pItinerary);

	SaveAs();
}

void CMainWnd::OnFilePrint()
{
	ASSERT(m_pItinerary);

	DWORD Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC;
	if (m_pWndDataGrid->HasSelection())
		Flags &= ~PD_NOSELECTION;

	CPrintDialogEx dlg(Flags, this);
	if (SUCCEEDED(dlg.DoModal()))
	{
		if (dlg.m_pdex.dwResultAction!=PD_RESULT_PRINT)
			return;

		Print(dlg.m_pdex);
	}
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
		m_ForceSidebarAlwaysVisible = TRUE;
		m_wndSidebar.SetSelection(IDM_SIDEBAR_FILEMENU);

		m_pWndFileMenu = new CFileMenu();
		m_pWndFileMenu->Create(this, 4, m_pItinerary);
		m_pWndFileMenu->ShowWindow(SW_SHOW);

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
	theApp.ShowNagScreen(NAG_EXPIRED, this);

	CWaitCursor csr;

	CBitmap* pBitmap = GetMap(TRUE, theApp.m_MapMergeMetro);

	CMapWnd* pFrameWnd = new CMapWnd();
	pFrameWnd->Create();
	pFrameWnd->SetBitmap(pBitmap, m_pItinerary->m_DisplayName, m_pItinerary->m_Metadata.Title[0]!=L'\0' ? m_pItinerary->m_Metadata.Title : m_pItinerary->m_DisplayName);
	pFrameWnd->ShowWindow(SW_SHOW);
}

void CMainWnd::OnMapSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_MAP_SETTINGS);

	MapSettingsDlg dlg(this);
	dlg.DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnGlobeOpen()
{
	ASSERT(m_pItinerary);

	CGlobeWnd* pFrameWnd = new CGlobeWnd();

	CKitchen* pKitchen = GetKitchen(TRUE, TRUE, theApp.m_GlobeMergeMetro);
	pKitchen->m_DisplayName = m_pItinerary->m_DisplayName;

	pFrameWnd->Create();
	pFrameWnd->SetFlights(pKitchen);
	pFrameWnd->ShowWindow(SW_SHOW);
}

void CMainWnd::OnGlobeSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_GLOBE_SETTINGS);

	GlobeSettingsDlg dlg(this);
	dlg.DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnGoogleEarthOpen()
{
	ASSERT(m_pItinerary);

	// Dateinamen finden
	TCHAR Pathname[MAX_PATH];
	if (!GetTempPath(MAX_PATH, Pathname))
		return;

	CString szTempName;
	srand(rand());
	szTempName.Format(_T("%sFlightmap%.4X%.4X.kml"), Pathname, 32768+rand(), 32768+rand());

	if (ExportGoogleEarth(szTempName, theApp.m_GoogleEarthUseCount, theApp.m_GoogleEarthUseColors, theApp.m_GoogleEarthClampHeight, TRUE, theApp.m_GoogleEarthMergeMetro))
		ShellExecute(GetSafeHwnd(), _T("open"), szTempName, NULL, NULL, SW_SHOWNORMAL);
}

void CMainWnd::OnGoogleEarthSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_GOOGLEEARTH_SETTINGS);

	GoogleEarthSettingsDlg dlg(this);
	dlg.DoModal();

	m_wndSidebar.SetSelection();
}

void CMainWnd::OnStatisticsOpen()
{
	ASSERT(m_pItinerary);

	theApp.ShowNagScreen(NAG_COUNTER, this);

	StatisticsDlg dlg(m_pItinerary, this);
	dlg.DoModal();
}

void CMainWnd::OnStatisticsSettings()
{
	m_wndSidebar.SetSelection(IDM_SIDEBAR_STATISTICS_SETTINGS);

	StatisticsSettingsDlg dlg(this);
	dlg.DoModal();

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
