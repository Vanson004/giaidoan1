
// Tuan1Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Tuan1.h"
#include "Tuan1Dlg.h"
#include "afxdialogex.h"

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
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTuan1Dlg dialog



CTuan1Dlg::CTuan1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TUAN1_DIALOG, pParent)
	, m_strContent(_T(""))
	, m_strPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTuan1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CONTENT, m_strContent);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
}

BEGIN_MESSAGE_MAP(CTuan1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CTuan1Dlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CTuan1Dlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CTuan1Dlg::OnBnClickedButtonLoad)
END_MESSAGE_MAP()


// CTuan1Dlg message handlers

BOOL CTuan1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(_T("Hello world MFC"));
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

void CTuan1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTuan1Dlg::OnPaint()
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
HCURSOR CTuan1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTuan1Dlg::OnBnClickedButtonOpen()
{
	UpdateData(TRUE);
	CFileDialog dlg(TRUE, _T("txt"), NULL, OFN_PATHMUSTEXIST, NULL, this);
	if (dlg.DoModal() == IDOK) {
		m_strPath = dlg.GetPathName();
		UpdateData(FALSE);
	}
}


void CTuan1Dlg::OnBnClickedButtonSave()
{
	HWND hEditPath = ::GetDlgItem(m_hWnd, IDC_EDIT_PATH);
	HWND hEditContent = ::GetDlgItem(m_hWnd, IDC_EDIT_CONTENT);

	int nPathLen = ::GetWindowTextLength(hEditPath);
	if (nPathLen == 0) return;

	int nContentLen = ::GetWindowTextLength(hEditContent);

	TCHAR* szFilePath = new TCHAR[nPathLen + 1];
	::GetDlgItemText(m_hWnd, IDC_EDIT_PATH, szFilePath, nPathLen + 1);

	TCHAR* szContent = NULL;
	if (nContentLen > 0)
	{
		szContent = new TCHAR[nContentLen + 1];
		::GetDlgItemText(m_hWnd, IDC_EDIT_CONTENT, szContent, nContentLen + 1);
	}

	HANDLE hFile = ::CreateFile(szFilePath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	delete[] szFilePath;

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesWritten = 0;
		DWORD dwFileSize = ::SetFilePointer(hFile, 0, NULL, FILE_END);
		if (nContentLen > 0)
		{
			TCHAR szNewLine[] = _T("\r\n");
			::WriteFile(hFile, szNewLine, (DWORD)(_tcslen(szNewLine) * sizeof(TCHAR)), &dwBytesWritten, NULL);
		}
		if (szContent != NULL)
		{
			DWORD dwBytesToWrite = (DWORD)(nContentLen * sizeof(TCHAR));
			::WriteFile(hFile, szContent, dwBytesToWrite, &dwBytesWritten, NULL);
			delete[] szContent;
		}
		::CloseHandle(hFile);

		::MessageBox(m_hWnd, _T("Lưu thành công!"), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
		::SetDlgItemText(m_hWnd, IDC_EDIT_PATH, _T(""));
		::SetDlgItemText(m_hWnd, IDC_EDIT_CONTENT, _T(""));
	}
	else
	{
		::MessageBox(m_hWnd, _T("Không thể mở/tạo file!"), _T("Lỗi"), MB_OK | MB_ICONERROR);
		if (szContent != NULL) delete[] szContent;
	}
}


void CTuan1Dlg::OnBnClickedButtonLoad()
{
	HWND hEditPath = ::GetDlgItem(m_hWnd, IDC_EDIT_PATH);
	int nPathLen = ::GetWindowTextLength(hEditPath);
	if (nPathLen == 0) return;

	TCHAR* szFilePath = new TCHAR[nPathLen + 1];
	::GetDlgItemText(m_hWnd, IDC_EDIT_PATH, szFilePath, nPathLen + 1);

	HANDLE hFile = ::CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	delete[] szFilePath;
	if (hFile == INVALID_HANDLE_VALUE) return;

	LARGE_INTEGER liSize;
	if (::GetFileSizeEx(hFile, &liSize) && liSize.QuadPart > 0)
	{
		DWORD dwFileSize = (DWORD)liSize.QuadPart;

		BYTE* pRawBuffer = new BYTE[dwFileSize + sizeof(TCHAR)];
		::ZeroMemory(pRawBuffer, dwFileSize + sizeof(TCHAR));

		DWORD dwBytesRead = 0;
		if (::ReadFile(hFile, pRawBuffer, dwFileSize, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			TCHAR* pText = (TCHAR*)pRawBuffer;
			::SetDlgItemText(m_hWnd, IDC_EDIT_CONTENT, pText);
		}
		else
		{
			::SetDlgItemText(m_hWnd, IDC_EDIT_CONTENT, _T(""));
		}

		delete[] pRawBuffer;
	}
	else
	{
		::SetDlgItemText(m_hWnd, IDC_EDIT_CONTENT, _T(""));
	}
	::CloseHandle(hFile);
}



