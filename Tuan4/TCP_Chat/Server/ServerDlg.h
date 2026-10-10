
// ServerDlg.h : header file
//

#pragma once

#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

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
	SOCKET m_ListenSocket = INVALID_SOCKET;
	SOCKET m_ClientSocket = INVALID_SOCKET;
	bool   m_isServerRunning = false;
	CListCtrl m_listChat;
	CEdit m_editInput;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnDestroy();
	void LogToUI(const CString& sender, const CString& message);
	void ServerWorkerThread();
};
