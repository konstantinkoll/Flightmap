
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
		CString Key;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(Key);

		FMGetApp()->WriteString(_T("License"), Key);

		CString Caption;
		CString Message;
		if (FMIsLicensed(NULL, TRUE))
		{
			ENSURE(Caption.LoadString(IDS_LICENSEVALID_CAPTION));
			ENSURE(Message.LoadString(IDS_LICENSEVALID_MSG));
			MessageBox(Message, Caption, MB_ICONINFORMATION);
		}
		else
		{
			ENSURE(Caption.LoadString(IDS_ERROR));
			ENSURE(Message.LoadString(IDS_INVALIDLICENSE));
			MessageBox(Message, Caption, MB_ICONWARNING);

			pDX->Fail();
		}
	}
}


BEGIN_MESSAGE_MAP(FMLicenseDlg, FMDialog)
	ON_BN_CLICKED(IDC_LOADLICENSE, OnLoadLicense)
	ON_EN_CHANGE(IDC_LICENSEKEY, OnChange)
END_MESSAGE_MAP()

void FMLicenseDlg::OnLoadLicense()
{
	CString tmpStr((LPCSTR)IDS_LICFILEFILTER);
	tmpStr += _T(" (*.lic)|*.lic||");

	CString Caption((LPCSTR)IDS_ERROR);
	CString Message((LPCSTR)IDS_CANNOTLOADLICENSE);

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString LicenseKey;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			MessageBox(Message, Caption, MB_OK | MB_ICONERROR);
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
				MessageBox(Message, Caption, MB_OK | MB_ICONERROR);
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
