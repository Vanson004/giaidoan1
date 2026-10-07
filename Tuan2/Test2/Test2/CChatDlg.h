#pragma once


// CChatDlg dialog	

class CChatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChatDlg)

public:
	CChatDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChatDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:	
	CImageList m_imgListFriends;
	CString m_editMessage;
	CListCtrl m_listChat;
	CTreeCtrl m_treeFriends;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void LoadChatHistory(CString strFriendName);
	CListCtrl m_listFriends;
	afx_msg void OnNMClickListFriends(NMHDR* pNMHDR, LRESULT* pResult);
};
