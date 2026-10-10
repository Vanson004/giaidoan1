
// ClientDlg.cpp : implementation file
//
#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
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


// CClientDlg dialog



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editInput);
	DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
	DDX_Control(pDX, IDC_EDIT_IP, m_editIP);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDlg::OnBnClickedButtonSend)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
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

	m_editIP.SetWindowTextW(_T("127.0.0.1"));
	m_isClientRunning = true;
	m_hClientThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientThreadProc, this, 0, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI CClientDlg::ClientThreadProc(LPVOID lpParam)
{
	CClientDlg* pDlg = (CClientDlg*)lpParam;
	if (pDlg) {
		pDlg->ClientWorkerThread();
	}
	return 0;
}

void CClientDlg::LogToUI(CString sender, CString msg)
{
	int nItem = m_listChat.GetItemCount();
	m_listChat.InsertItem(nItem, sender);
	m_listChat.SetItemText(nItem, 1, msg);
}

void CClientDlg::ClientWorkerThread()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) return;

	m_ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_ClientSocket == INVALID_SOCKET) {
		WSACleanup();
		return;
	}

	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(0);
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(m_ClientSocket, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

	wchar_t RecvBuf[DEFAULT_BUFLEN / sizeof(wchar_t)];
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	while (m_isClientRunning)
	{
		iResult = recvfrom(m_ClientSocket, (char*)RecvBuf, DEFAULT_BUFLEN, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
		if (iResult > 0)
		{
			RecvBuf[(DEFAULT_BUFLEN / sizeof(wchar_t)) - 1] = L'\0';
			CString strMsg(RecvBuf);
			LogToUI(_T("Server"), strMsg);
		}
		else if (iResult == SOCKET_ERROR)
		{
			break;
		}
	}

	if (m_ClientSocket != INVALID_SOCKET) {
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

void CClientDlg::OnBnClickedButtonSend()
{
	if (m_ClientSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket chưa sẵn sàng!"), MB_OK | MB_ICONWARNING);
		return;
	}

	CString strIP, strText;
	m_editIP.GetWindowText(strIP);
	m_editInput.GetWindowText(strText);

	if (strText.IsEmpty()) return;
	if (strIP.IsEmpty()) strIP = _T("127.0.0.1");

	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(DEFAULT_PORT);
	CT2A ipAscii(strIP);
	if (inet_pton(AF_INET, ipAscii, &RecvAddr.sin_addr) <= 0) {
		AfxMessageBox(_T("Địa chỉ IP không hợp lệ!"), MB_OK | MB_ICONWARNING);
		return;
	}

	wchar_t SendBuf[DEFAULT_BUFLEN / sizeof(wchar_t)] = { 0 };
	wcscpy_s(SendBuf, DEFAULT_BUFLEN / sizeof(wchar_t), strText.GetString());

	int iResult = sendto(m_ClientSocket, (char*)SendBuf, DEFAULT_BUFLEN, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
	if (iResult != SOCKET_ERROR) {
		LogToUI(_T("Client"), strText);
		m_editInput.SetWindowText(_T(""));
	}
}

void CClientDlg::OnClose()
{
	m_isClientRunning = false;

	if (m_ClientSocket != INVALID_SOCKET) {
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}

	if (m_hClientThread != NULL) {
		WaitForSingleObject(m_hClientThread, 1000);
		CloseHandle(m_hClientThread);
		m_hClientThread = NULL;
	}

	CDialogEx::OnClose();
}
