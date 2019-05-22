
// CMapWnd.cpp: Implementierung der Klasse CMapWnd
//

#include "stdafx.h"
#include "CMapWnd.h"
#include "Flightmap.h"
#include <winspool.h>


// CMapWnd
//

CIcons CMapWnd::m_LargeIcons;
CIcons CMapWnd::m_SmallIcons;

CMapWnd::CMapWnd()
	: CBackstageWnd()
{
	m_pBitmap = NULL;
}

BOOL CMapWnd::Create()
{
	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_MAP));

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, CString((LPCSTR)IDR_MAP), _T("Map"), CSize(0, 0), TRUE);
}

BOOL CMapWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The map view gets the command first
	if (m_wndMapView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CBackstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMapWnd::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), TaskHeight, nFlags);

	m_wndMapView.SetWindowPos(NULL, rectLayout.left, rectLayout.top+TaskHeight, rectLayout.Width(), rectLayout.Height()-TaskHeight, nFlags);
}

void CMapWnd::SetBitmap(CBitmap* pBitmap, CItinerary* pItinerary)
{
	ASSERT(pItinerary);

	// Set window caption
	CString Caption((LPCSTR)IDR_MAP);
	pItinerary->InsertDisplayName(Caption);

	SetWindowText(Caption);

	// Set bitmap and title
	delete m_pBitmap;
	m_wndMapView.SetBitmap(m_pBitmap=pBitmap, m_Title=pItinerary->GetTitle());
}

void CMapWnd::PrintMap(PRINTDLGEX pdex)
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

	INT Width = dc.GetDeviceCaps(HORZRES);
	INT Height = dc.GetDeviceCaps(VERTRES);
	const DOUBLE Spacer = (Width/40.0);

	CRect rectPage(0, 0, Width, Height);
	rectPage.DeflateRect((INT)Spacer, (INT)Spacer);

	if (dc.StartDoc(&di)>=0)
	{
		if (dc.StartPage()>=0)
		{
			theApp.PrintPageHeader(dc, rectPage, Spacer, di);

			Bitmap Map(*m_pBitmap, NULL);
			const DOUBLE ScX = (DOUBLE)rectPage.Width()/(DOUBLE)Map.GetWidth();
			const DOUBLE ScY = (DOUBLE)rectPage.Height()/(DOUBLE)Map.GetHeight();
			const DOUBLE Scale = min(ScX, ScY);
			const INT Width = (INT)((DOUBLE)Map.GetWidth()*Scale);
			const INT Height = (INT)((DOUBLE)Map.GetHeight()*Scale);

			// Draw map
			Graphics g(dc);
			g.SetPageUnit(UnitPixel);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			g.DrawImage(&Map, rectPage.left, rectPage.top, Width, Height);

			// Draw border
			CPen pen(PS_SOLID, 2, (COLORREF)0x000000);
			CPen* pOldPen = dc.SelectObject(&pen);

			HBRUSH hOldBrush = (HBRUSH)dc.SelectStockObject(HOLLOW_BRUSH);

			dc.Rectangle(rectPage.left, rectPage.top, rectPage.left+Width, rectPage.top+Height);

			dc.SelectObject(hOldBrush);
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


BEGIN_MESSAGE_MAP(CMapWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()

	ON_COMMAND(IDM_MAPWND_SAVEAS, OnMapWndSaveAs)
	ON_COMMAND(IDM_MAPWND_PRINT, OnMapWndPrint)
	ON_COMMAND(IDM_MAPWND_COPY, OnMapWndCopy)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_MAPWND_SAVEAS, IDM_MAPWND_PRINT, OnUpdateMapWndCommands)
END_MESSAGE_MAP()

INT CMapWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	hAccelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDA_ACCELERATOR_MAP));

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_MAP_16, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_MAPWND_ZOOMIN, 0);
	m_wndTaskbar.AddButton(IDM_MAPWND_ZOOMOUT, 1);
	m_wndTaskbar.AddButton(IDM_MAPWND_SAVEAS, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_MAPWND_PRINT, 3);
	m_wndTaskbar.AddButton(IDM_MAPWND_COPY, 4);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_PURCHASE, 5, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ENTERLICENSEKEY, 6, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 8, TRUE, TRUE);

	// Map view
	if (!m_wndMapView.Create(this, 2))
		return -1;

	// Taskbar
	DisableTaskbarPinning(L"app.liquidFOLDERS.Flightmap.Map");

	return 0;
}

void CMapWnd::OnDestroy()
{
	delete m_pBitmap;

	CBackstageWnd::OnDestroy();
}

void CMapWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	m_wndMapView.SetFocus();
}


void CMapWnd::OnMapWndSaveAs()
{
	ASSERT(m_pBitmap);

	CString Extensions;
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_BMP, _T("bmp"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_JPEG, _T("jpg"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_PNG, _T("png"));
	theApp.AddFileExtension(Extensions, IDS_FILEFILTER_TIFF, _T("tif"), TRUE);

	CFileDialog dlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	dlg.m_ofn.nFilterIndex = 3;

	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor WaitCursor;
	
		const CString Ext = dlg.GetFileExt().MakeLower();

		if (Ext==_T("bmp"))
		{
			theApp.SaveBitmap(m_pBitmap, dlg.GetPathName(), ImageFormatBMP, FALSE);
		}
		else
			if (Ext==_T("jpg"))
			{
				theApp.SaveBitmap(m_pBitmap, dlg.GetPathName(), ImageFormatJPEG, FALSE);
			}
			else
				if (Ext==_T("png"))
				{
					theApp.SaveBitmap(m_pBitmap, dlg.GetPathName(), ImageFormatPNG, FALSE);
				}
				else
					if (Ext==_T("tif"))
					{
						theApp.SaveBitmap(m_pBitmap, dlg.GetPathName(), ImageFormatTIFF, FALSE);
					}
	}
}

void CMapWnd::OnMapWndPrint()
{
	CPrintDialogEx dlg(PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_RETURNDC, this);
	if (SUCCEEDED(dlg.DoModal()) && (dlg.m_pdex.dwResultAction==PD_RESULT_PRINT))
		PrintMap(dlg.m_pdex);
}

void CMapWnd::OnMapWndCopy()
{
	ASSERT(m_pBitmap);

	CWaitCursor WaitCursor;

	if (!OpenClipboard())
		return;

	EmptyClipboard();

	// Create copy in global memory
	CSize Size = m_pBitmap->GetBitmapDimension();
	
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Size.cx;
	DIB.bmiHeader.biHeight = Size.cy;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 24;
	DIB.bmiHeader.biCompression = BI_RGB;

	HDC hDC = ::GetDC(NULL);
	GetDIBits(hDC, *m_pBitmap, 0, Size.cy, NULL, &DIB, DIB_RGB_COLORS);

	if (!DIB.bmiHeader.biSizeImage)
		DIB.bmiHeader.biSizeImage = ((((Size.cx*DIB.bmiHeader.biBitCount)+31) & ~31)>>3)*Size.cy;

	HGLOBAL hDIB = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER)+DIB.bmiHeader.biSizeImage);
	if (hDIB)
	{
		union
		{
			LPVOID Ptr;
			LPBYTE pByte;
			LPBITMAPINFO pInfo;
		} Hdr;

		Hdr.Ptr = GlobalLock(hDIB);
		memcpy(Hdr.Ptr, &DIB.bmiHeader, sizeof(BITMAPINFOHEADER));

		// Convert/copy the image bits
		if (GetDIBits(hDC, *m_pBitmap, 0, Size.cy, Hdr.pByte+sizeof(BITMAPINFOHEADER), Hdr.pInfo, DIB_RGB_COLORS))
		{
			GlobalUnlock(hDIB);

			SetClipboardData(CF_DIB, hDIB);
		}
		else
		{
			GlobalUnlock(hDIB);
			GlobalFree(hDIB);
		}
	}

	::ReleaseDC(NULL, hDC);

	CloseClipboard();
}

void CMapWnd::OnUpdateMapWndCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pBitmap!=NULL);
}
