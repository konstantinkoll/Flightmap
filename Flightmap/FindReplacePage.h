
// FindReplacePage.h: Schnittstelle der Klasse FindReplacePage
//

#pragma once


// FindReplacePage
//

class FindReplacePage : public CPropertyPage
{
public:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
