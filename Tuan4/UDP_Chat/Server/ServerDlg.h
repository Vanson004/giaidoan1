
// ServerDlg.h : header file
//

#pragma 

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

// CServerDlg dialog
class CServerDlg : public CDialogEx
{
// Construction
public:
	CServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	SOCKET m_ServerSocket = INVALID_SOCKET;    
	bool m_isServerRunning = false;         
	sockaddr_in m_ClientAddr = {};
	HANDLE m_hServerThread = NULL;
	CEdit m_editInput;
	CListCtrl m_listChat;
	static DWORD WINAPI ServerThreadProc(LPVOID lpParam);
	void ServerWorkerThread();
	void LogToUI(CString sender, CString msg);
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnClose();
};
