
// CIcons.h: Schnittstelle der Klasse CIcons
//

#pragma once
#include "FMFont.h"


// CIcons

#define LI_NORMAL             (UINT)0
#define LI_SLIGHTLYLARGER     (UINT)1
#define LI_FORTOOLTIPS        (UINT)2

class CIcons sealed
{
public:
	CIcons();
	~CIcons();

	void Load(UINT nID, CSize Size);
	void Load(UINT nID, INT Size);
	INT Load(UINT nID, UINT Flags=LI_SLIGHTLYLARGER, FMFont* pFont=NULL);
	INT LoadSmall(UINT nID);
	INT LoadForSize(UINT nID, INT Height);
	void Create(const CSize& Size, UINT MaxIcons);
	void Create(const CImageList& ImageList, UINT MaxIcons);
	INT GetIconSize() const;
	INT AddIcon(HICON hIcon);
	INT AddIcon(CImageList& ImageList, INT nImage);
	void SetGammaMode(BOOL UseDarkBackgroundGamma);
	void Draw(CDC& dc, INT x, INT y, INT nImage, BOOL Hot=FALSE, BOOL Disabled=FALSE, BOOL Shadow=FALSE);
	static HBITMAP ExtractBitmap(CImageList& ImageList, INT nImage, BOOL WhiteBackground=FALSE);
	HBITMAP ExtractBitmap(INT nImage, BOOL Shadow=FALSE);
	HICON ExtractIcon(INT nImage, BOOL Shadow=FALSE);
	HIMAGELIST ExtractImageList() const;

	static const INT m_IconSizes[5];

protected:
	HBITMAP hBitmapNormal;
	HBITMAP hBitmapShadow;
	HBITMAP hBitmapHot;
	HBITMAP hBitmapDisabled;
	CSize m_Size;

private:
	HBITMAP CreateCopy();
	void CreateIconsShadow();
	void CreateIconsHot();
	void CreateIconsDisabled();

	UINT m_IconCount;
	UINT m_MaxIcons;

	BOOL m_UseDarkBackgroundGamma;
	static BOOL m_GammaTableCreated;
	static BYTE m_GammaTableDarkBackground[256];
	static BYTE m_GammaTableLightBackground[256];
};

inline void CIcons::SetGammaMode(BOOL UseDarkBackgroundGamma)
{
	m_UseDarkBackgroundGamma = UseDarkBackgroundGamma;
}
