
// FMFont.cpp: Implementierung der Klasse FMFont
//

#pragma once
#include "stdafx.h"
#include "FMFont.h"


// FMFont
//

FMFont::FMFont()
	: CFont()
{
	m_FontHeight = 0;
}

BOOL FMFont::CreateFont(INT nHeight, BYTE nQuality, INT nWeight, BYTE bItalic, LPCTSTR lpszFacename)
{
	BOOL Result = CFont::CreateFont(nHeight, 0, 0, 0, nWeight, bItalic, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, nQuality, VARIABLE_PITCH | FF_SWISS, lpszFacename);

	if (Result)
		CalcFontHeight();

	return Result;
}

BOOL FMFont::CreateFontIndirect(const LOGFONT* lpLogFont)
{
	BOOL Result = CFont::CreateFontIndirect(lpLogFont);

	if (Result)
		CalcFontHeight();

	return Result;
}

INT FMFont::GetFontHeight() const
{
	return m_FontHeight;
}

CSize FMFont::GetTextExtent(LPCTSTR lpszString) const
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CFont* pOldFont = dc.SelectObject((FMFont*)this);
	CSize Size = dc.GetTextExtent(lpszString, (INT)wcslen(lpszString));
	dc.SelectObject(pOldFont);

	return Size;
}

CSize FMFont::GetTextExtent(const CString& str) const
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CFont* pOldFont = dc.SelectObject((FMFont*)this);
	CSize Size = dc.GetTextExtent(str);
	dc.SelectObject(pOldFont);

	return Size;
}

__forceinline void FMFont::CalcFontHeight()
{
	m_FontHeight = GetTextExtent(_T("Wy")).cy;
}
