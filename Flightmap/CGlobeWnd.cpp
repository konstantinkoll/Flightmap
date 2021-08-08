
// CGlobeWnd.cpp: Implementierung der Klasse CGlobeWnd
//

#include "stdafx.h"
#include "CGlobeWnd.h"
#include "Flightmap.h"


// CGlobeWnd
//

CIcons CGlobeWnd::m_LargeIcons;
CIcons CGlobeWnd::m_SmallIcons;

BOOL CGlobeWnd::Create()
{
	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_GLOBE));

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, CString((LPCSTR)IDR_GLOBE), _T("Globe"), CSize(0, 0), TRUE);
}

BOOL CGlobeWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The map view gets the command first
	if (m_wndGlobeView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CBackstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CGlobeWnd::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), TaskHeight, nFlags);

	m_wndGlobeView.SetWindowPos(NULL, rectLayout.left, rectLayout.top+TaskHeight, rectLayout.Width(), rectLayout.Height()-TaskHeight, nFlags);
}

void CGlobeWnd::SetFlights(CKitchen* pKitchen)
{
	ASSERT(pKitchen);

	// Set window caption
	CString Caption((LPCSTR)IDR_GLOBE);
	pKitchen->InsertDisplayName(Caption);

	SetWindowText(Caption);

	// Set flights
	m_wndGlobeView.SetFlights(pKitchen);
}


BEGIN_MESSAGE_MAP(CGlobeWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_3DSETTINGSCHANGED, On3DSettingsChanged)
END_MESSAGE_MAP()

INT CGlobeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	hAccelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDA_ACCELERATOR_GLOBE));

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_GLOBE_16, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_GLOBEWND_JUMPTOLOCATION, 0, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_ZOOMIN, 1);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_ZOOMOUT, 2);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_AUTOSIZE, 3);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_SAVEAS, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_GOOGLEEARTH, 5, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBEWND_LIQUIDFOLDERS, 6, TRUE);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 8, TRUE, TRUE);

	// Globe view
	if (!m_wndGlobeView.Create(this, 2))
		return -1;

	// Taskbar
	DisableTaskbarPinning(L"app.liquidFOLDERS.Flightmap.Globe");

	return 0;
}

void CGlobeWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	m_wndGlobeView.SetFocus();
}

void CGlobeWnd::On3DSettingsChanged()
{
	m_wndGlobeView.UpdateViewOptions();
}
