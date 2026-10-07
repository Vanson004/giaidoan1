
// Test2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Test2.h"
#include "Test2Dlg.h"
#include "afxdialogex.h"
#include "CChatDlg.h"
#include "CRegister.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickListFriends(NMHDR* pNMHDR, LRESULT* pResult);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_LIST_Friends, &CAboutDlg::OnNMClickListFriends)
END_MESSAGE_MAP()


// CTest2Dlg dialog



CTest2Dlg::CTest2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST2_DIALOG, pParent)
	, m_editUsername(_T(""))
	, m_editPassword(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTest2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Username, m_editUsername);
	DDX_Text(pDX, IDC_EDIT_Password, m_editPassword);
}

BEGIN_MESSAGE_MAP(CTest2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Login, &CTest2Dlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_Register, &CTest2Dlg::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CTest2Dlg message handlers

BOOL CTest2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTest2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTest2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTest2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTest2Dlg::OnBnClickedButtonLogin()
{
	/*UpdateData(TRUE);

	CString strUsername = m_editUsername;
	CString strPassword = m_editPassword;
	if (strUsername.IsEmpty() || strPassword.IsEmpty()) {
		AfxMessageBox(_T("Vui long dien day du thong tin"));
		return;
	}

	CStdioFile fileRead;
	BOOL login = FALSE;
	if (fileRead.Open(_T("accounts.txt"), CFile::modeRead | CFile::typeText)) {
		CString line;
		CString strUsernameFile, strPasswordFile;

		while (fileRead.ReadString(line)) {
			AfxExtractSubString(strUsernameFile, line, 0, ' ');
			AfxExtractSubString(strPasswordFile, line, 1, ' ');

			strUsernameFile.Trim();
			strPasswordFile.Trim();
			if (strUsernameFile == strUsername && strPasswordFile == strPassword) {
				login = TRUE;
				break;
			}
		}
		fileRead.Close();
	}
	else {
		AfxMessageBox(_T("Loi doc file"));
	}

	if (login == TRUE) {
		AfxMessageBox(_T("Dang nhap thanh cong"));
		EndDialog(IDOK);
		CChatDlg cchat;
		cchat.DoModal();
	}
	else {
		AfxMessageBox(_T("Dang nhap that bai"));
	}
	*/
	UpdateData(TRUE);
	CString strUsername, strPassword;
	GetDlgItemText(IDC_EDIT_Username, strUsername);
	GetDlgItemText(IDC_EDIT_Password, strPassword);

	if (strUsername.IsEmpty() || strPassword.IsEmpty()) {
		AfxMessageBox(_T("Vui lòng điền đầy đủ thông tin"));
	}

	BOOL isLogin = FALSE;
	if (strUsername == "admin" && strPassword == "admin") {
		isLogin = TRUE;
	}
	
	if (isLogin) {
		AfxMessageBox(_T("Đăng nhập thành công"));
		CChatDlg chat;
		chat.DoModal();
	}
	else {
		AfxMessageBox(_T("Đăng nhập thất bại"));
	}
}


void CTest2Dlg::OnBnClickedButtonRegister()
{
	// TODO: Add your control notification handler code here
	CRegister dlg;
	if (dlg.DoModal() == IDOK) {

	}
}






void CAboutDlg::OnNMClickListFriends(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}
