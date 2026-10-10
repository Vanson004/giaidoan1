
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
	DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editInput);
	DDX_Control(pDX, IDC_EDIT_IP, m_editIP);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CClientDlg::OnBnClickedButtonConnect)
	ON_WM_DESTROY()
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
	m_listChat.InsertColumn(0, _T("Người gửi"), LVCFMT_LEFT, 110);
	m_listChat.InsertColumn(1, _T("Nội dung"), LVCFMT_LEFT, 360);

	m_ConnectSocket = INVALID_SOCKET;
	m_isClientRunning = false;

	m_editIP.SetWindowText(_T("127.0.0.1"));

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

void CClientDlg::LogToUI(const CString& sender, const CString& message)
{
	int nIndex = m_listChat.GetItemCount();
	m_listChat.InsertItem(nIndex, sender);
	m_listChat.SetItemText(nIndex, 1, message);
	m_listChat.EnsureVisible(nIndex, FALSE);
}


void CClientDlg::OnBnClickedButtonSend()
{
	if (m_ConnectSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Chưa kết nối tới Server!"), MB_OK | MB_ICONWARNING);
		return;
	}

	CString strText;
	m_editInput.GetWindowText(strText);
	if (strText.IsEmpty()) return;

	wchar_t sendbuf[DEFAULT_BUFLEN / sizeof(wchar_t)] = { 0 };

	wcscpy_s(sendbuf, DEFAULT_BUFLEN / sizeof(wchar_t), strText.GetString());

	int iResult = send(m_ConnectSocket, (char*)sendbuf, DEFAULT_BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(m_ConnectSocket);
		m_ConnectSocket = INVALID_SOCKET;
		WSACleanup();
	}
	else {
		LogToUI(_T("Client"), strText);
		m_editInput.SetWindowText(_T(""));
	}
}


void CClientDlg::OnBnClickedButtonConnect()
{
	if (m_ConnectSocket != INVALID_SOCKET) {
		AfxMessageBox(_T("Đã kết nối tới Server từ trước!"));
		return;
	}

	CString strIP;
	m_editIP.GetWindowText(strIP);
	if (strIP.IsEmpty()) {
		AfxMessageBox(_T("Vui lòng nhập địa chỉ IP Server!"));
		return;
	}

	m_isClientRunning = true;
	std::thread(&CClientDlg::ClientWorkerThread, this, strIP).detach();
}

void CClientDlg::ClientWorkerThread(CString strIP)
{
	WSADATA wsaData;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	CT2A ipAscii(strIP);

	iResult = getaddrinfo(ipAscii, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		m_ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			return;
		}

		iResult = connect(m_ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(m_ConnectSocket);
			m_ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_ConnectSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Không thể kết nối tới Server!"), MB_OK | MB_ICONERROR);
		WSACleanup();
		return;
	}

	AfxMessageBox(_T("Đã kết nối thành công tới Server!"), MB_OK | MB_ICONINFORMATION);

	wchar_t recvbuf[DEFAULT_BUFLEN / sizeof(wchar_t)];

	while (m_isClientRunning)
	{
		int totalBytesReceived = 0;

		while (totalBytesReceived < DEFAULT_BUFLEN)
		{
			iResult = recv(m_ConnectSocket, ((char*)recvbuf) + totalBytesReceived, DEFAULT_BUFLEN - totalBytesReceived, 0);
			if (iResult > 0)
			{
				totalBytesReceived += iResult;
			}
			else
			{
				goto CLEANUP_LABEL;
			}
		}

		recvbuf[(DEFAULT_BUFLEN / sizeof(wchar_t)) - 1] = L'\0';

		CString strMsg(recvbuf);
		LogToUI(_T("Server"), strMsg);
	}

CLEANUP_LABEL:
	if (m_ConnectSocket != INVALID_SOCKET) {
		shutdown(m_ConnectSocket, SD_SEND);
		closesocket(m_ConnectSocket);
		m_ConnectSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

void CClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_isClientRunning = false;

	if (m_ConnectSocket != INVALID_SOCKET) {
		closesocket(m_ConnectSocket);
	}
	WSACleanup();
}