
// CMapWnd.cpp: Implementierung der Klasse CMapWnd
//

#include "stdafx.h"
#include "CLoungeView.h"
#include "CMapWnd.h"
#include "Flightmap.h"
#include <winspool.h>


// CMapWnd
//

CMapWnd::CMapWnd()
	: CMainWindow()
{
	m_pBitmap = NULL;
}

CMapWnd::~CMapWnd()
{
	if (m_pBitmap)
		delete m_pBitmap;
}

BOOL CMapWnd::Create()
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, FMGetApp()->LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_MAP));

	CString Caption((LPCSTR)IDR_MAP);

	return CMainWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, Caption, _T("Map"));
}

void CMapWnd::SetBitmap(CBitmap* pBitmap, CString DisplayName, CString Title)
{
	if (m_pBitmap)
		delete m_pBitmap;

	m_Title = Title;

	CString Caption((LPCSTR)IDR_MAP);

	if (!DisplayName.IsEmpty())
	{
		Caption.Insert(0, _T(" - "));
		Caption.Insert(0, DisplayName);
	}

	SetWindowText(Caption);
	m_wndMapView.SetBitmap(m_pBitmap=pBitmap);
}

void CMapWnd::ExportMap(CString Filename, GUID guidFileType)
{
	ASSERT(m_pBitmap);

	CWaitCursor csr;

	theApp.SaveBitmap(m_pBitmap, Filename, guidFileType, FALSE);
}

void CMapWnd::ExportMap(DWORD FilterIndex)
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
			ExportMap(dlg.GetPathName(), ImageFormatBMP);
		}
		else
			if (ext==_T("jpg"))
			{
				ExportMap(dlg.GetPathName(), ImageFormatJPEG);
			}
			else
				if (ext==_T("png"))
				{
					ExportMap(dlg.GetPathName(), ImageFormatPNG);
				}
				else
					if (ext==_T("tif"))
					{
						ExportMap(dlg.GetPathName(), ImageFormatTIFF);
					}
	}
}

void CMapWnd::Print(PRINTDLGEX pdex)
{
	ASSERT(m_pBitmap);

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
	di.lpszDocName = m_Title;

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

			Bitmap bmp((HBITMAP)m_pBitmap->m_hObject, NULL);
			const DOUBLE ScX = (DOUBLE)rectPage.Width()/(DOUBLE)bmp.GetWidth();
			const DOUBLE ScY = (DOUBLE)rectPage.Height()/(DOUBLE)bmp.GetHeight();
			const DOUBLE Scale = min(ScX, ScY);
			const INT w = (INT)((DOUBLE)bmp.GetWidth()*Scale);
			const INT h = (INT)((DOUBLE)bmp.GetHeight()*Scale);

			Graphics g(dc);
			g.SetPageUnit(UnitPixel);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			g.DrawImage(&bmp, rectPage.left, rectPage.top, w, h);

			CPen pen(PS_SOLID, 2, (COLORREF)0x000000);
			CPen* pOldPen = dc.SelectObject(&pen);
			dc.SelectStockObject(HOLLOW_BRUSH);
			dc.Rectangle(rectPage.left, rectPage.top, rectPage.left+w, rectPage.top+h);
			dc.SelectObject(pOldPen);

			dc.EndPage();
		}

		dc.EndDoc();
	}

	if (pdex.hDevMode)
		GlobalFree(pdex.hDevMode);
	if (pdex.hDevNames)
		GlobalFree(pdex.hDevNames);
}


BOOL CMapWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMapView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CMainWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMapWnd::AdjustLayout()
{
	CMainWindow::AdjustLayout();

	CRect rect;
	GetClientRect(rect);

	if (m_pDialogMenuBar)
		rect.top += m_pDialogMenuBar->GetPreferredHeight();

	m_wndMapView.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMapWnd, CMainWindow)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
	ON_REGISTERED_MESSAGE(theApp.m_UseBgImagesChangedMsg, OnUseBgImagesChanged)

	ON_COMMAND(IDM_MAPWND_COPY, OnMapWndCopy)
	ON_COMMAND(IDM_MAPWND_SAVEAS, OnMapWndSaveAs)
	ON_COMMAND(IDM_MAPWND_PRINT, OnMapWndPrint)
	ON_COMMAND(IDM_MAPWND_PRINT_QUICK, OnMapWndPrintQuick)
	ON_COMMAND(IDM_MAPWND_CLOSE, OnMapWndClose)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPWND_COPY, IDM_MAPWND_CLOSE, OnUpdateMapWndCommands)

	ON_COMMAND(IDM_MAP_EXPORT_BMP, OnMapExportBMP)
	ON_COMMAND(IDM_MAP_EXPORT_JPEG, OnMapExportJPEG)
	ON_COMMAND(IDM_MAP_EXPORT_PNG, OnMapExportPNG)
	ON_COMMAND(IDM_MAP_EXPORT_TIFF, OnMapExportTIFF)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAP_EXPORT_BMP, IDM_MAP_EXPORT_TIFF, OnUpdateMapExportCommands)
END_MESSAGE_MAP()

INT CMapWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMainWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	hAccelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_MAP));

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS_32, IDB_MENUBARICONS_16, 1);

	m_pDialogMenuBar->AddMenuLeft(IDM_MAPWND);
	m_pDialogMenuBar->AddMenuLeft(IDM_MAPVIEW);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 1);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 2);
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 3);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 4);

	if (!m_wndMapView.Create(this, 2))
		return -1;

	return 0;
}

void CMapWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	m_wndMapView.SetFocus();
}

LRESULT CMapWnd::OnRequestSubmenu(WPARAM wParam, LPARAM /*lParam*/)
{
	CDialogMenuPopup* pPopup = new CDialogMenuPopup();

	switch ((UINT)wParam)
	{
	case IDM_MAPWND:
		pPopup->Create(this, IDB_MENUMAPWND_32, IDB_MENUMAPWND_16);
		pPopup->AddCommand(IDM_MAPWND_COPY, 0, CDMB_MEDIUM);
		pPopup->AddSubmenu(IDM_MAPWND_SAVEAS, 1, CDMB_MEDIUM, TRUE);
		pPopup->AddSeparator();
		pPopup->AddSubmenu(IDM_MAPWND_PRINT, 2, CDMB_MEDIUM, TRUE);
		pPopup->AddSeparator();
		pPopup->AddCommand(IDM_MAPWND_CLOSE, 5, CDMB_MEDIUM);
		break;
	case IDM_MAPVIEW:
		pPopup->Create(this, IDB_MENUMAPVIEW_32, IDB_MENUMAPVIEW_16);
		pPopup->AddCommand(IDM_MAPVIEW_ZOOMIN, 0, CDMB_SMALL, FALSE);
		pPopup->AddCommand(IDM_MAPVIEW_ZOOMOUT, 1, CDMB_SMALL, FALSE);
		pPopup->AddSeparator();
		pPopup->AddCheckbox(IDM_MAPVIEW_AUTOSIZE, FALSE, TRUE);
		break;
	case IDM_MAPWND_SAVEAS:
		pPopup->Create(this, IDB_MENUMAPWND_32, IDB_MENUMAPWND_16);
		pPopup->AddCaption(IDS_EXPORT);
		pPopup->AddFileType(IDM_MAP_EXPORT_BMP, _T("bmp"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_JPEG, _T("jpg"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_PNG, _T("png"), CDMB_LARGE);
		pPopup->AddFileType(IDM_MAP_EXPORT_TIFF, _T("tiff"), CDMB_LARGE);
		break;
	case IDM_MAPWND_PRINT:
		pPopup->Create(this, IDB_MENUMAPWND_32, IDB_MENUMAPWND_16);
		pPopup->AddCaption(IDS_PRINT);
		pPopup->AddCommand(IDM_MAPWND_PRINT, 3, CDMB_LARGE);
		pPopup->AddCommand(IDM_MAPWND_PRINT_QUICK, 4, CDMB_LARGE);
		break;
	}

	if (!pPopup->HasItems())
	{
		delete pPopup;
		pPopup = NULL;
	}

	return (LRESULT)pPopup;
}

LRESULT CMapWnd::OnUseBgImagesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (IsCtrlThemed())
		m_wndMapView.Invalidate();

	return NULL;
}


void CMapWnd::OnMapWndCopy()
{
	CWaitCursor csr;

	// Create device-dependent bitmap
	CDC* pDC = GetWindowDC();
	CSize sz = m_pBitmap->GetBitmapDimension();

	CDC dc;
	dc.CreateCompatibleDC(pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(pDC, sz.cx, sz.cy);
	CBitmap* pOldBitmap1 = dc.SelectObject(&buffer);

	CDC dcMap;
	dcMap.CreateCompatibleDC(pDC);

	CBitmap* pOldBitmap2 = dcMap.SelectObject(m_pBitmap);
	dc.BitBlt(0, 0, sz.cx, sz.cy, &dcMap, 0, 0, SRCCOPY);

	dcMap.SelectObject(pOldBitmap2);
	dc.SelectObject(pOldBitmap1);
	ReleaseDC(pDC);

	// Copy to clipboard
	if (OpenClipboard())
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, buffer.Detach());

		CloseClipboard();
	}
}

void CMapWnd::OnMapWndSaveAs()
{
	ExportMap();
}

void CMapWnd::OnMapWndPrint()
{
	CPrintDialogEx dlg(PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC, this);
	if (SUCCEEDED(dlg.DoModal()))
	{
		if (dlg.m_pdex.dwResultAction!=PD_RESULT_PRINT)
			return;

		Print(dlg.m_pdex);
	}
}

void CMapWnd::OnMapWndPrintQuick()
{
	CPrintDialogEx dlg(FALSE);

	ZeroMemory(&dlg.m_pdex, sizeof(dlg.m_pdex));
	dlg.m_pdex.lStructSize = sizeof(dlg.m_pdex);
	dlg.m_pdex.hwndOwner = GetSafeHwnd();
	dlg.m_pdex.Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC;
	dlg.m_pdex.nStartPage = (DWORD)-1;

	if (dlg.GetDefaults())
		Print(dlg.m_pdex);
}

void CMapWnd::OnMapWndClose()
{
	SendMessage(WM_CLOSE);
}

void CMapWnd::OnUpdateMapWndCommands(CCmdUI* pCmdUI)
{
	TCHAR szPrinterName[256];
	DWORD cchPrinterName;

	BOOL b = (m_pBitmap!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_CLOSE:
		b = TRUE;
		break;
	case IDM_MAPWND_PRINT_QUICK:
		b &= GetDefaultPrinter(szPrinterName, &cchPrinterName);
		break;
	}

	pCmdUI->Enable(b);
}


// Map export commands

void CMapWnd::OnMapExportBMP()
{
	ExportMap(1);
}

void CMapWnd::OnMapExportJPEG()
{
	ExportMap(2);
}

void CMapWnd::OnMapExportPNG()
{
	ExportMap(3);
}

void CMapWnd::OnMapExportTIFF()
{
	ExportMap(4);
}

void CMapWnd::OnUpdateMapExportCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pBitmap!=NULL);
}
