
// FMLicenseDlg.cpp: Implementierung der Klasse FMLicenseDlg
//

#include "stdafx.h"
#include "FMCommDlg.h"
#include "Resource.h"


// FMLicenseDlg
//

FMLicenseDlg::FMLicenseDlg(CWnd* pParent)
	: FMDialog(IDD_ENTERLICENSEKEY, FMDS_Blue, pParent)
{
}

void FMLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	FMDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CString key;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(key);

		((FMApplication*)AfxGetApp())->WriteString(_T("License"), key);

		CString caption;
		CString message;
		if (FMIsLicensed(NULL, true))
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

void FMLicenseDlg::OnLoadLicense()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_LICFILEFILTER));
	tmpStr += _T(" (*.lic)|*.lic||");

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString key;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
//			LFErrorBox(LFDriveNotReady);
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
//				LFErrorBox(LFDriveNotReady);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(key);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}


BEGIN_MESSAGE_MAP(FMLicenseDlg, FMDialog)
	ON_BN_CLICKED(IDC_LOADLICENSE, OnLoadLicense)
END_MESSAGE_MAP()
