#pragma once


// CRegister dialog

class CRegister : public CDialogEx
{
	DECLARE_DYNAMIC(CRegister)

public:
	CRegister(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRegister();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CString m_editUsername;
	CString m_editPassword;
	CString m_editConfirm;
	afx_msg void OnBnClickedButtonRegister();
};
