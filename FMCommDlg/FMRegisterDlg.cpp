
// FMRegisterDlg.cpp: Implementierung der Klasse FMRegisterDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include <wininet.h>


// FMRegisterDlg
//

#define ARROW     _T("è")

FMRegisterDlg::FMRegisterDlg(CWnd* pParentWnd)
	: FMDialog(IDD_REGISTER, pParentWnd, TRUE)
{
	m_PictureHeight = m_ItemHeight = m_Indent = 0;
}

void FMRegisterDlg::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	FMDialog::PaintOnBackground(dc, g, rectLayout);

	const BOOL Themed = IsCtrlThemed();

	// Text
	CRect rectBorders(0, 0, 7, 7);
	MapDialogRect(&rectBorders);

	CRect rectItem(rectLayout);
	rectItem.left = rectLayout.left+rectBorders.right;
	rectItem.top = rectLayout.top+rectBorders.bottom;

	CFont* pOldFont = dc.SelectObject(&FMGetApp()->m_DefaultFont);

	for (UINT a=0; a<MAXREGISTERITEMS; a++)
	{
		CRect rectText(rectItem);
		if (a>0)
		{
			static const WCHAR Bullet = 0x25BA;

			dc.SetTextColor(Themed ? 0xC0C0C0 : GetSysColor(COLOR_WINDOWTEXT));
			dc.DrawText(&Bullet, 1, rectText, DT_SINGLELINE | DT_LEFT);

			rectText.left += m_Indent;
		}

		dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_RegisterItems[a], rectText, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS);

		rectItem.OffsetRect(0, a==0 ? 2*m_ItemHeight : a==1 ? 7*m_ItemHeight/4+m_PictureHeight : m_ItemHeight);
	}

	// Arrow
	dc.SelectObject(&m_ArrowFont);

	dc.SetTextColor(Themed ? 0xC0C0C0 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(ARROW, m_ArrowRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(pOldFont);
}

void FMRegisterDlg::CheckInternetConnection()
{
	DWORD Flags;
	GetDlgItem(IDC_PURCHASE)->EnableWindow(InternetGetConnectedState(&Flags, 0));
}

BOOL FMRegisterDlg::InitDialog()
{
	FMGetApp()->PlayAsteriskSound();

	SetBottomLeftControl(IDC_ENTERLICENSEKEY);
	AddBottomRightControl(IDC_PURCHASE);
	AddBottomRightControl(IDOK);

	// Hintergrund
	CRect rectBorders(0, 0, 7, 7);
	MapDialogRect(&rectBorders);

	m_ItemHeight = FMGetApp()->m_DefaultFont.GetFontHeight();
	m_PictureHeight = min(m_ItemHeight*10, 330)+4;
	m_Indent = m_ItemHeight*9/8;

	m_ArrowFont.CreateFont(-3*FMGetApp()->m_DefaultFont.GetFontHeight()/2, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("Wingdings"));

	const INT DynamicHeight = m_ItemHeight*MAXREGISTERITEMS+m_PictureHeight+(m_ItemHeight*7/4);
	INT DynamicWidth = 0;

	for (UINT a=0; a<MAXREGISTERITEMS; a++)
	{
		ENSURE(m_RegisterItems[a].LoadString(IDS_REGISTER_CAPTION+a));

		INT Width = FMGetApp()->m_DefaultFont.GetTextExtent(m_RegisterItems[a]).cx;
		if (a>0)
			Width += m_Indent;

		DynamicWidth = max(DynamicWidth, Width);
	}

	// Bilder
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	const INT PosX = rectBorders.right+m_Indent;
	const INT PosY = rectLayout.top+rectBorders.bottom+m_ItemHeight*13/4;

	m_ArrowRect = CRect(PosX+m_PictureHeight+rectBorders.right, PosY, PosX+m_PictureHeight+rectBorders.right+m_ArrowFont.GetTextExtent(ARROW).cx, PosY+m_PictureHeight);

	m_wndPictureUnregistered.Create(this, CRect(PosX, PosY, PosX+m_PictureHeight, m_ArrowRect.bottom), 1, IDB_NRT_UNREGISTERED, IDS_UNREGISTERED);
	m_wndPictureRegistered.Create(this, CRect(PosX+2*rectBorders.right+m_PictureHeight+m_ArrowRect.Width(), PosY, PosX+2*(rectBorders.right+m_PictureHeight)+m_ArrowRect.Width(), m_ArrowRect.bottom), 1, IDB_NRT_REGISTERED, IDS_REGISTERED);

	// Größe anpassen
	CRect rectWnd;
	GetWindowRect(rectWnd);
	SetWindowPos(NULL, 0, 0, DynamicWidth+rectBorders.right*2, rectWnd.Height()+DynamicHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return TRUE;
}


BEGIN_MESSAGE_MAP(FMRegisterDlg, FMDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(NM_CLICK, IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
	ON_BN_CLICKED(IDC_PURCHASE, OnPurchase)
END_MESSAGE_MAP()

void FMRegisterDlg::OnDestroy()
{
	KillTimer(1);

	FMDialog::OnDestroy();
}

void FMRegisterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	FMDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void FMRegisterDlg::OnEnterLicenseKey(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	FMLicenseDlg(this).DoModal();

	if (FMIsLicensed())
		EndDialog(IDOK);

	*pResult = 0;
}

void FMRegisterDlg::OnPurchase()
{
	CCmdUI cmd;
	cmd.m_nID = IDM_BACKSTAGE_PURCHASE;

	FMGetApp()->OnCmdMsg(IDM_BACKSTAGE_PURCHASE, CN_COMMAND, &cmd, NULL);
}
