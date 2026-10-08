
// ClientDlg.h : header file
//

#pragma once
#define WM_RECEIVE_PIPE_DATA (WM_USER + 101)

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
	static DWORD WINAPI CClientDlg::ClientWorkerThread(LPVOID lpParam);
	CEdit m_editInput;
	CListCtrl m_listChat;
	afx_msg LRESULT OnReceivePipeData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSend();
};
