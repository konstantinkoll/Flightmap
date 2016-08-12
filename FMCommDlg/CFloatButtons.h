
// CFloatButtons.h: Schnittstelle der Klasse CFloatButtons
//

#pragma once
#include "CFrontstageWnd.h"
#include "CHoverButton.h"
#include "FMDynArray.h"


// CFloatButton
//

class CFloatButton : public CHoverButton
{
public:
	BOOL Create(CWnd* pParentWnd, UINT nID, CIcons* pButtonIcons, CIcons* pTooltipIcons, INT IconID, BOOL Small);
	void GetPreferredSize(LPSIZE lpSize) const;
	BOOL IsSmall() const;

protected:
	void OnDrawButtonForeground(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	CString m_Caption;
	CString m_Hint;
	INT m_IconID;
	BOOL m_Small;
};

inline BOOL CFloatButton::IsSmall() const
{
	return m_Small;
}


// CButtonGroup
//

struct ButtonGroupItem
{
	CFloatButton* pFloatButton;
	WCHAR Text[256];
	BOOL Bullet;
	BYTE BeginNewColumn;
	SIZE Size;
	RECT Rect;
};

class CFloatButtons;

class CButtonGroup
{
public:
	CButtonGroup(CFloatButtons* pFloatButtons, LPCWSTR Caption, BOOL Alert=FALSE);
	~CButtonGroup();

	void BeginNewColumn();
	void AddButton(UINT nID, INT IconID, BOOL Small=FALSE);
	void AddText(LPCWSTR Text, BOOL Bullet=FALSE);
	void SetGroupAlert(BOOL Alert=TRUE);
	void SetText(UINT Index, LPCWSTR Text, BOOL Bullet=FALSE);
	void OnIdleUpdateCmdUI();
	void GetPreferredSize(LPSIZE lpSize, INT MaxWidth);
	void AdjustLayout(INT VScrollPos);
	void Draw(CDC& dc, Graphics& g, INT VScrollPos, BOOL Themed) const;
	BOOL SetFocus();

	RECT m_Rect;

protected:
	FMDynArray<ButtonGroupItem, 4, 4> m_Items;
	WCHAR m_Caption[256];
	CFloatButtons* p_FloatButtons;
	BOOL m_Alert;

private:
	void CalcSize(ButtonGroupItem& Item) const;

	BOOL m_BeginNewColumn;
	static INT m_Top;
};


// CFloatButtons
//

class CFloatButtons : public CFrontstageWnd
{
friend class CButtonGroup;

public:
	CFloatButtons();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID);
	void BeginGroup(LPCWSTR Caption, BOOL Alert=FALSE);
	void BeginGroup(UINT ResID, BOOL Alert=FALSE);
	void BeginNewColumn();
	void AddButton(UINT nID, INT IconID, BOOL AddRight=FALSE);
	void AddText(LPCWSTR Text, BOOL Bullet=FALSE);
	void AddText(UINT ResID, BOOL Bullet=FALSE);
	void SetGroupAlert(UINT nGroup, BOOL Alert=TRUE);
	void SetText(UINT nGroup, UINT nID, LPCWSTR Text, BOOL Bullet=FALSE);
	void SetText(UINT nGroup, UINT nID, UINT ResID, BOOL Bullet=FALSE);
	void AdjustLayout();
	void AdjustScrollbars();

protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()

	FMDynArray<CButtonGroup*, 32, 32> m_ButtonGroups;
	INT m_Indent;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;

private:
	CIcons* p_SmallIcons;
	CIcons* p_LargeIcons;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
