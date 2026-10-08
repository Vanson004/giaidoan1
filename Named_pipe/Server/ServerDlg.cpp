
// ServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "ServerDlg.h"
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


// CServerDlg dialog



CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editInput);
	DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_RECEIVE_PIPE_DATA, &CServerDlg::OnReceivePipeData)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CServerDlg::OnBnClickedButtonSend)
END_MESSAGE_MAP()


// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog()
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

	m_listChat.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listChat.InsertColumn(0, _T("Người chat"), LVCFMT_LEFT, 100);
	m_listChat.InsertColumn(1, _T("Nội dung chat"), LVCFMT_LEFT, 200);

	m_bIsRunning = TRUE;
	DWORD dwThreadId = 0;
	m_hMainServerThread = ::CreateThread(NULL, 0, CServerDlg::MainListenThread, this, 0, &dwThreadId);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CServerDlg::OnPaint()
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
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI CServerDlg::MainListenThread(LPVOID lpParam)
{
	CServerDlg* pDlg = (CServerDlg*)lpParam;
	if (!pDlg) return 0;

	LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\MySamplePipe");

	while (pDlg->m_bIsRunning)
	{
		HANDLE hPipe = ::CreateNamedPipe(lpszPipename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			512, 512, 0, NULL);

		if (hPipe == INVALID_HANDLE_VALUE)
			break;

		BOOL fConnected = ::ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected && pDlg->m_bIsRunning)
		{
			struct InstanceParams {
				CServerDlg* pDlg;
				HANDLE hPipe;
			};

			InstanceParams* pParams = new InstanceParams;
			pParams->pDlg = pDlg;
			pParams->hPipe = hPipe;

			DWORD dwThreadId = 0;
			HANDLE hThread = ::CreateThread(NULL, 0, CServerDlg::InstanceThread, pParams, 0, &dwThreadId);
			if (hThread != NULL)
			{
				::CloseHandle(hThread);
			}
		}
		else
		{
			::CloseHandle(hPipe);
		}
	}

	return 0;
}

DWORD WINAPI CServerDlg::InstanceThread(LPVOID lpParam)
{
	struct InstanceParams {
		CServerDlg* pDlg;
		HANDLE hPipe;
	};

	InstanceParams* pParams = (InstanceParams*)lpParam;
	if (!pParams) return 0;

	CServerDlg* pDlg = pParams->pDlg;
	HANDLE hPipe = pParams->hPipe;
	delete pParams;

	TCHAR szRequest[512];
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	while (pDlg->m_bIsRunning)
	{
		fSuccess = ::ReadFile(hPipe, szRequest, sizeof(szRequest) - sizeof(TCHAR), &cbBytesRead, NULL);
		if (!fSuccess || cbBytesRead == 0)
		{
			break;
		}

		szRequest[cbBytesRead / sizeof(TCHAR)] = _T('\0');

		TCHAR* pData = _tcsdup(szRequest);
		::PostMessage(pDlg->GetSafeHwnd(), WM_RECEIVE_PIPE_DATA, (WPARAM)pData, 0);

		pDlg->m_hCurrentClientPipe = hPipe;

		while (pDlg->m_hCurrentClientPipe != NULL && pDlg->m_bIsRunning)
		{
			Sleep(50);
		}
	}

	::FlushFileBuffers(hPipe);
	::DisconnectNamedPipe(hPipe);
	::CloseHandle(hPipe);

	return 1;
}

LRESULT CServerDlg::OnReceivePipeData(WPARAM wParam, LPARAM lParam)
{
	TCHAR* pText = (TCHAR*)wParam;
	if (pText)
	{
		int nIndex = m_listChat.GetItemCount();
		m_listChat.InsertItem(nIndex, _T("Client"));
		m_listChat.SetItemText(nIndex, 1, pText);

		free(pText);
	}
	return 0;
}


void CServerDlg::OnBnClickedButtonSend()
{
	CString strReply;
	m_editInput.GetWindowText(strReply);

	if (strReply.IsEmpty()) return;

	if (m_hCurrentClientPipe != INVALID_HANDLE_VALUE && m_hCurrentClientPipe != NULL)
	{
		DWORD cbToWrite = (strReply.GetLength() + 1) * sizeof(TCHAR);
		DWORD cbWritten = 0;

		BOOL fSuccess = ::WriteFile(m_hCurrentClientPipe, strReply.GetString(), cbToWrite, &cbWritten, NULL);

		if (fSuccess)
		{
			int nIndex = m_listChat.GetItemCount();
			m_listChat.InsertItem(nIndex, _T("Server"));
			m_listChat.SetItemText(nIndex, 1, strReply);

			m_editInput.SetWindowText(_T(""));
			m_hCurrentClientPipe = NULL;
		}
		else
		{
			AfxMessageBox(_T("Gửi phản hồi thất bại"));
		}
	}
	else
	{
		AfxMessageBox(_T("Chưa có tin nhắn Client nào tới để phản hồi"));
	}
}
