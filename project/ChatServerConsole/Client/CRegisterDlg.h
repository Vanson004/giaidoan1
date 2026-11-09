#pragma once
class CRegisterDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CRegisterDlg)

public:
    CRegisterDlg(CWnd* pParent = nullptr);
    virtual ~CRegisterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_REGISTER };
#endif
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
public:
    CEdit m_editRegisterUser;
    CEdit m_editRegisterPass;
    CEdit m_editRegisterConfirm;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonDoRegister();
};
