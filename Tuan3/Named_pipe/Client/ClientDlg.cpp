
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
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDlg::OnBnClickedButtonSend)
    ON_MESSAGE(WM_RECEIVE_PIPE_DATA, &CClientDlg::OnReceivePipeData)
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
    m_listChat.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    m_listChat.InsertColumn(0, _T("Người chat"), LVCFMT_LEFT, 100);
    m_listChat.InsertColumn(1, _T("Nội dung chat"), LVCFMT_LEFT, 200);

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

struct ClientThreadParams {
	HWND hWndDlg = NULL;       
	CString strMessage;
};


DWORD WINAPI CClientDlg::ClientWorkerThread(LPVOID lpParam)
{
    ClientThreadParams* pParams = (ClientThreadParams*)lpParam;
    if (!pParams) return 0;

    HWND hWndDlg = pParams->hWndDlg;
    CString strMsgToSend = pParams->strMessage;
    delete pParams;

    HANDLE hPipe = INVALID_HANDLE_VALUE;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\MySamplePipe");
    BOOL fSuccess = FALSE;
    DWORD cbToWrite, cbWritten, cbRead, dwMode;
    TCHAR chBuf[512];

    while (1)
    {
        hPipe = ::CreateFile(lpszPipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);                        
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        if (::GetLastError() != ERROR_PIPE_BUSY)
        {
            TCHAR* pErrMsg = _tcsdup(_T("Lỗi: Không thể mở Pipe kết nối Server."));
            ::PostMessage(hWndDlg, WM_RECEIVE_PIPE_DATA, (WPARAM)pErrMsg, 0);
            return 0;
        }
        if (!::WaitNamedPipe(lpszPipename, 20000))
        {
            TCHAR* pErrMsg = _tcsdup(_T("Lỗi: Hết 20s chờ Server phản hồi (Time out)."));
            ::PostMessage(hWndDlg, WM_RECEIVE_PIPE_DATA, (WPARAM)pErrMsg, 0);
            return 0;
        }
    }

    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = ::SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);    
    if (!fSuccess)
    {
        ::CloseHandle(hPipe);
        return 0;
    }

    cbToWrite = (strMsgToSend.GetLength() + 1) * sizeof(TCHAR);
    fSuccess = ::WriteFile(hPipe, strMsgToSend.GetString(), cbToWrite, &cbWritten, NULL);
    if (!fSuccess)
    {
        ::CloseHandle(hPipe);
        return 0;
    }

    CString strServerReply = _T("");
    do
    {
        fSuccess = ::ReadFile(hPipe, chBuf, sizeof(chBuf) - sizeof(TCHAR), &cbRead, NULL);
        if (!fSuccess && ::GetLastError() != ERROR_MORE_DATA)
            break;

        chBuf[cbRead / sizeof(TCHAR)] = _T('\0');
        strServerReply += chBuf;

    } while (!fSuccess);

    if (strServerReply.GetLength() > 0)
    {
        size_t nLen = static_cast<size_t>(strServerReply.GetLength());

        TCHAR* pData = new TCHAR[nLen + 1];
        _tcscpy_s(pData, nLen + 1, strServerReply.GetString());

        ::PostMessage(hWndDlg, WM_RECEIVE_PIPE_DATA, (WPARAM)pData, 0);
    }
    ::CloseHandle(hPipe);

    return 0;
}

LRESULT CClientDlg::OnReceivePipeData(WPARAM wParam, LPARAM lParam)
{
    TCHAR* pText = (TCHAR*)wParam;
    if (pText)
    {
        int nIndex = m_listChat.GetItemCount();
        m_listChat.InsertItem(nIndex, _T("Server"));
        m_listChat.SetItemText(nIndex, 1, pText);

        m_listChat.EnsureVisible(nIndex, FALSE);

        free(pText);
    }
    return 0;
}

void CClientDlg::OnBnClickedButtonSend()
{
    CString strMsg;
    m_editInput.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    int nIndex = m_listChat.GetItemCount();
    m_listChat.InsertItem(nIndex, _T("Client"));
    m_listChat.SetItemText(nIndex, 1, strMsg);

    m_editInput.SetWindowText(_T(""));

    ClientThreadParams* pParams = new ClientThreadParams;
    pParams->hWndDlg = this->GetSafeHwnd();
    pParams->strMessage = strMsg;

    DWORD dwThreadId = 0;
    HANDLE hThread = ::CreateThread(NULL, 0, CClientDlg::ClientWorkerThread, pParams, 0, &dwThreadId);
    if (hThread != NULL)
    {
        ::CloseHandle(hThread);
    }
}

