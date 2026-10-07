// CRegister.cpp : implementation file
//

#include "pch.h"
#include "Test2.h"
#include "CRegister.h"
#include "afxdialogex.h"


// CRegister dialog

IMPLEMENT_DYNAMIC(CRegister, CDialogEx)

CRegister::CRegister(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
	, m_editUsername(_T(""))
	, m_editPassword(_T(""))
	, m_editConfirm(_T(""))
{

}

CRegister::~CRegister()
{
}

void CRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Username, m_editUsername);
	DDX_Text(pDX, IDC_EDIT_Password, m_editPassword);
	DDX_Text(pDX, IDC_EDIT_Confirm, m_editConfirm);
}

BOOL CRegister::OnInitDialog() {
	CDialogEx::OnInitDialog();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CRegister, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_Register, &CRegister::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CRegister message handlers


void CRegister::OnBnClickedButtonRegister()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CString strUsername = m_editUsername;
	CString strPassword = m_editPassword;
	CString strConfirm = m_editConfirm;
	if (strUsername.IsEmpty() || strPassword.IsEmpty()) {
		AfxMessageBox(_T("Vui long dien day du thong tin"));
		return;
	}
	if (strConfirm != strPassword) {
		AfxMessageBox(_T("Xac nhan mat khau chua dung"));
		return;
	}
	CStdioFile fileWrite;
	if (fileWrite.Open(_T("accounts.txt"), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeText)) {
		fileWrite.SeekToEnd();
		CString strData = strUsername + _T(" ") + strPassword + _T("\r\n");
		fileWrite.WriteString(strData);
		fileWrite.Close();
		AfxMessageBox(_T("Tao tai khoan thanh cong"));
		CDialogEx::OnOK();
	}
	else {
		AfxMessageBox(_T("Tao tai khoan that bai"));
	}
}
