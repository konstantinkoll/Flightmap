
// FMLicenseDlg.cpp: Implementierung der Klasse FMLicenseDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"


// FMLicenseDlg
//

FMLicenseDlg::FMLicenseDlg(CWnd* pParentWnd)
	: FMDialog(IDD_LICENSE, pParentWnd)
{
}

void FMLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CString LicenseKey;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(LicenseKey);

		FMGetApp()->WriteString(_T("License"), LicenseKey);

		if (FMIsLicensed(NULL, TRUE))
		{
			::PostMessage(HWND_BROADCAST, FMGetApp()->m_LicenseActivatedMsg, NULL, NULL);

			FMMessageBox(this, CString((LPCSTR)IDS_LICENSEVALID_MSG), CString((LPCSTR)IDS_LICENSEVALID_CAPTION), MB_ICONINFORMATION | MB_OK);
		}
		else
		{
			FMMessageBox(this, CString((LPCSTR)IDS_INVALIDLICENSE), CString((LPCSTR)IDS_ERROR), MB_ICONWARNING | MB_OK);

			pDX->Fail();
		}
	}
}

BOOL FMLicenseDlg::InitDialog()
{
	GetDlgItem(IDC_INSTRUCTIONS)->SetFont(&FMGetApp()->m_DefaultFont);

	return TRUE;
}


BEGIN_MESSAGE_MAP(FMLicenseDlg, FMDialog)
	ON_BN_CLICKED(IDC_LOADLICENSE, OnLoadLicense)
	ON_EN_CHANGE(IDC_LICENSEKEY, OnChange)
END_MESSAGE_MAP()

void FMLicenseDlg::OnLoadLicense()
{
	CString tmpStr((LPCSTR)IDS_LICFILEFILTER);
	tmpStr += _T(" (*.lic)|*.lic||");

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString LicenseKey;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			FMErrorBox(this, IDS_CANNOTLOADLICENSE);
		}
		else
		{
			try
			{
				CString Line;

				UINT cLines = 0;
				while ((f.ReadString(Line)) && (cLines++<128))
					LicenseKey.Append(Line+_T("\r\n"));
			}
			catch(CFileException ex)
			{
				FMErrorBox(this, IDS_CANNOTLOADLICENSE);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(LicenseKey);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}

void FMLicenseDlg::OnChange()
{
	CString LicenseKey;
	GetDlgItem(IDC_LICENSEKEY)->GetWindowText(LicenseKey);

	GetDlgItem(IDOK)->EnableWindow(!LicenseKey.IsEmpty());
}
