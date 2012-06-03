
// CMapWnd.cpp: Implementierung der Klasse CMapWnd
//

#include "stdafx.h"
#include "CLoungeView.h"
#include "CMapWnd.h"
#include "Flightmap.h"


// CMapWnd
//

CMapWnd::CMapWnd()
	: CMainWindow()
{
	m_hIcon = NULL;
	m_pBitmap = NULL;
}

CMapWnd::~CMapWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	if (m_pBitmap)
		delete m_pBitmap;
}

BOOL CMapWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_MAP);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_MAP));

	return CMainWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

void CMapWnd::SetBitmap(CBitmap* pBitmap, CString DisplayName)
{
	if (m_pBitmap)
		delete m_pBitmap;

	CString caption;
	ENSURE(caption.LoadString(IDR_MAP));

	if (!DisplayName.IsEmpty())
	{
		caption.Insert(0, _T(" - "));
		caption.Insert(0, DisplayName);
	}

	SetWindowText(caption);
	m_wndMapView.SetBitmap(m_pBitmap=pBitmap);
}

void CMapWnd::ExportMap(CString Filename, GUID guidFileType)
{
	ASSERT(m_pBitmap);

	CWaitCursor csr;

	theApp.SaveBitmap(m_pBitmap, Filename, guidFileType, FALSE);
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
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_REQUESTSUBMENU, OnRequestSubmenu)
	ON_REGISTERED_MESSAGE(theApp.msgUseBgImagesChanged, OnUseBgImagesChanged)

	ON_COMMAND(IDM_MAPWND_COPY, OnMapWndCopy)
	ON_COMMAND(IDM_MAPWND_SAVEAS, OnMapWndSaveAs)
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

	m_pDialogMenuBar = new CDialogMenuBar();
	m_pDialogMenuBar->Create(this, IDB_MENUBARICONS, 1);

	m_pDialogMenuBar->AddMenuLeft(IDM_MAPWND);
	m_pDialogMenuBar->AddMenuLeft(IDM_MAPVIEW);

	m_pDialogMenuBar->AddMenuRight(ID_APP_PURCHASE, 0);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ENTERLICENSEKEY, 1);
	m_pDialogMenuBar->AddMenuRight(ID_APP_SUPPORT, 2);
	m_pDialogMenuBar->AddMenuRight(ID_APP_ABOUT, 3);

	if (!m_wndMapView.Create(this, 2))
		return -1;

	theApp.AddFrame(this);

	return 0;
}

void CMapWnd::OnDestroy()
{
	CMainWindow::OnDestroy();
	theApp.KillFrame(this);
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
		pPopup->AddCommand(IDM_MAPWND_CLOSE, 6, CDMB_MEDIUM);
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
		pPopup->AddCaption(IDS_PRINTPREVIEW);
		pPopup->AddCommand(IDM_MAPWND_PRINT, 2, CDMB_LARGE);
		pPopup->AddCommand(IDM_MAPWND_PRINT_QUICK, 3, CDMB_LARGE);
		pPopup->AddCommand(IDM_MAPWND_PRINT_PREVIEW, 4, CDMB_LARGE);
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
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
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

void CMapWnd::OnMapWndClose()
{
	SendMessage(WM_CLOSE);
}

void CMapWnd::OnUpdateMapWndCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(pCmdUI->m_nID==IDM_MAPWND_CLOSE ? TRUE : m_pBitmap!=NULL);
}


// Map export commands

void CMapWnd::OnMapExportBMP()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"), TRUE);

	CFileDialog dlg(FALSE, _T("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatBMP);
}

void CMapWnd::OnMapExportJPEG()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"), TRUE);

	CFileDialog dlg(FALSE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatJPEG);
}

void CMapWnd::OnMapExportPNG()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatPNG);
}

void CMapWnd::OnMapExportTIFF()
{
	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("tif"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
		ExportMap(dlg.GetPathName(), ImageFormatTIFF);
}

void CMapWnd::OnUpdateMapExportCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pBitmap!=NULL);
}
