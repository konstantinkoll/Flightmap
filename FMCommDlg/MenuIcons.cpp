
// MenuIcons.cpp: Implementierung von Methoden zur Darstellung von Menu-Icons
//

#include "stdafx.h"


void SetMenuItemBitmap(HMENU hMenu, UINT item, HBITMAP hBmp)
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = hBmp;
	SetMenuItemInfo(hMenu, item, TRUE, &mii);

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_CHECKORBMP;
	SetMenuInfo(hMenu, &mi);
}

HBITMAP IconToBitmap(HICON hIcon, INT cx, INT cy)
{
	if (!hIcon)
		return NULL;

	HDC hDC = CreateCompatibleDC(NULL);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = cx;
	dib.bmiHeader.biHeight = -cy;
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(hDC, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP pOldBitmap = (HBITMAP)SelectObject(hDC, hBmp);

	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	if (osInfo.dwMajorVersion<6)
	{
		CRect rect(0, 0, cx, cy);
		FillRect(hDC, rect, (HBRUSH)GetSysColorBrush(COLOR_MENU));
	}
	DrawIconEx(hDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);

	SelectObject(hDC, pOldBitmap);
	DeleteDC(hDC);

	return hBmp;
}

HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, HICON hIcon, INT cx, INT cy)
{
	HBITMAP hBmp = IconToBitmap(hIcon, cx, cy);
	SetMenuItemBitmap(hMenu, item, hBmp);

	return hBmp;
}

HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, WORD ResID)
{
	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);
	HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(ResID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);

	HBITMAP res = SetMenuItemIcon(hMenu, item, hIcon, cx, cy);
	DestroyIcon(hIcon);

	return res;
}