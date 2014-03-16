
// FMLicenseDlg.cpp: Implementierung der Klasse FMLicenseDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMLicenseDlg
//

FMLicenseDlg::FMLicenseDlg(CWnd* pParentWnd)
	: FMDialog(IDD_ENTERLICENSEKEY, pParentWnd)
{
}

void FMLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CString key;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(key);

		p_App->WriteString(_T("License"), key);

		CString caption;
		CString message;
		if (FMIsLicensed(NULL, TRUE))
		{
			ENSURE(caption.LoadString(IDS_LICENSEVALID_CAPTION));
			ENSURE(message.LoadString(IDS_LICENSEVALID_MSG));
			MessageBox(message, caption, MB_ICONINFORMATION);
		}
		else
		{
			ENSURE(caption.LoadString(IDS_ERROR));
			ENSURE(message.LoadString(IDS_INVALIDLICENSE));
			MessageBox(message, caption, MB_ICONWARNING);

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
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_LICFILEFILTER));
	tmpStr += _T(" (*.lic)|*.lic||");

	CString caption;
	ENSURE(caption.LoadString(IDS_ERROR));
	CString msg;
	ENSURE(msg.LoadString(IDS_CANNOTLOADLICENSE));

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString key;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			MessageBox(msg, caption, MB_OK | MB_ICONERROR);
		}
		else
		{
			try
			{
				CString line;

				UINT lines = 0;
				while ((f.ReadString(line)) && (lines++<128))
					key.Append(line+_T("\r\n"));
			}
			catch(CFileException ex)
			{
				MessageBox(msg, caption, MB_OK | MB_ICONERROR);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(key);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}

void FMLicenseDlg::OnChange()
{
	CString key;
	GetDlgItem(IDC_LICENSEKEY)->GetWindowText(key);

	GetDlgItem(IDOK)->EnableWindow(!key.IsEmpty());
}
