
// ServerDlg.h : header file
//
#define WM_RECEIVE_PIPE_DATA (WM_USER + 101)
#pragma once


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
	BOOL m_bIsRunning = FALSE;
	HANDLE m_hCurrentClientPipe = INVALID_HANDLE_VALUE;
	HANDLE m_hMainServerThread = NULL;
	static DWORD WINAPI MainListenThread(LPVOID lpParam);
	static DWORD WINAPI InstanceThread(LPVOID lpParam);
	CEdit m_editInput;
	CListCtrl m_listChat;
	afx_msg LRESULT OnReceivePipeData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSend();
};
