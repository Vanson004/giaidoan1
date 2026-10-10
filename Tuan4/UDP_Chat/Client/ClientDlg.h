
// ClientDlg.h : header file
//

#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

// CClientDlg dialog
class CClientDlg : public CDialogEx
{
// Construction
public:
	CClientDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
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
	SOCKET m_ClientSocket = INVALID_SOCKET;         
	bool m_isClientRunning = false;     
	HANDLE m_hClientThread = NULL;
	static DWORD WINAPI ClientThreadProc(LPVOID lpParam);
	void ClientWorkerThread();     
	void LogToUI(CString sender, CString msg);
	CEdit m_editInput;
	CListCtrl m_listChat;
	CEdit m_editIP;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnClose();
	
};
