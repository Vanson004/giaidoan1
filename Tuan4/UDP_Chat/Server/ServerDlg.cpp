
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
	m_listChat.InsertColumn(0, _T("Người gửi"), LVCFMT_LEFT, 100);
	m_listChat.InsertColumn(1, _T("Nội dung tin nhắn"), LVCFMT_LEFT, 350);

	m_isServerRunning = true;
	m_hServerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerThreadProc, this, 0, NULL);

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

DWORD WINAPI CServerDlg::ServerThreadProc(LPVOID lpParam)
{
	CServerDlg* pDlg = (CServerDlg*)lpParam;
	if (pDlg) {
		pDlg->ServerWorkerThread();
	}
	return 0;
}

void CServerDlg::ServerWorkerThread()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) return;

	m_ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_ServerSocket == INVALID_SOCKET) {
		WSACleanup();
		return;
	}

	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(DEFAULT_PORT); 
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iResult = bind(m_ServerSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
	if (iResult != 0) {
		closesocket(m_ServerSocket);
		m_ServerSocket = INVALID_SOCKET;
		WSACleanup();
		return;
	}

	AfxMessageBox(_T("UDP Server đã sẵn sàng lắng nghe tại cổng 27015!"), MB_OK | MB_ICONINFORMATION);

	wchar_t RecvBuf[DEFAULT_BUFLEN / sizeof(wchar_t)];
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	while (m_isServerRunning)
	{
		iResult = recvfrom(m_ServerSocket, (char*)RecvBuf, DEFAULT_BUFLEN, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
		if (iResult > 0)
		{
			m_ClientAddr = SenderAddr;
			RecvBuf[(DEFAULT_BUFLEN / sizeof(wchar_t)) - 1] = L'\0';
			CString strMsg(RecvBuf);
			LogToUI(_T("Client"), strMsg);
		}
		else if (iResult == SOCKET_ERROR)
		{
			break;
		}
	}

	if (m_ServerSocket != INVALID_SOCKET) {
		closesocket(m_ServerSocket);
		m_ServerSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

void CServerDlg::OnBnClickedButtonSend()
{
	if (m_ServerSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Server chưa khởi tạo thành công!"), MB_OK | MB_ICONWARNING);
		return;
	}

	CString strText;
	m_editInput.GetWindowText(strText);
	if (strText.IsEmpty()) return;

	wchar_t SendBuf[DEFAULT_BUFLEN / sizeof(wchar_t)] = { 0 };
	wcscpy_s(SendBuf, DEFAULT_BUFLEN / sizeof(wchar_t), strText.GetString());

	int iResult = sendto(m_ServerSocket, (char*)SendBuf, DEFAULT_BUFLEN, 0, (SOCKADDR*)&m_ClientAddr, sizeof(m_ClientAddr));
	if (iResult != SOCKET_ERROR) {
		LogToUI(_T("Server"), strText);
		m_editInput.SetWindowText(_T(""));
	}
}

void CServerDlg::LogToUI(CString sender, CString msg)
{
	int nItem = m_listChat.GetItemCount();
	m_listChat.InsertItem(nItem, sender);
	m_listChat.SetItemText(nItem, 1, msg);
}

void CServerDlg::OnClose()
{
	m_isServerRunning = false;

	if (m_ServerSocket != INVALID_SOCKET) {
		closesocket(m_ServerSocket);
		m_ServerSocket = INVALID_SOCKET;
	}

	if (m_hServerThread != NULL) {
		WaitForSingleObject(m_hServerThread, 1000);
		CloseHandle(m_hServerThread);
		m_hServerThread = NULL;
	}

	CDialogEx::OnClose();
}