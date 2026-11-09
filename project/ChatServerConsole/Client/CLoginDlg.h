#pragma once
class CLoginDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CLoginDlg)

public:
    CLoginDlg(CWnd* pParent = nullptr);
    virtual ~CLoginDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_LOGIN };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

public:
    HICON m_hIcon;
    CEdit m_editLoginUser;
    CEdit m_editLoginPass;
    SOCKET  m_hSocket;         
    CString m_strUsername;    
    bool m_bLoginSuccess;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonLogin();
    afx_msg void OnBnClickedButtonGoRegister();
};
