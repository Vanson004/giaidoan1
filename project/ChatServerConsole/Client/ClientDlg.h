#pragma once
#include <string>
#include <vector>
#include <richedit.h>
using namespace std;

struct TinNhan {
    CString sender;
    CString receiver;
    CString content;
    CString time;
    int daXem;
};

struct TrangThaiUser {
    CString username;
    bool isOnline;
};

struct LichSuChat {
    CString friendName;
    vector<TinNhan> dsTinNhan;
};

struct ThongBaoDaXem {
    CString nguoiXem;
};
#define WM_RECEIVE_MSG     (WM_USER + 100)
#define WM_SERVER_DISCONN  (WM_USER + 101)
#define WM_USER_STATUS     (WM_USER + 102)
#define WM_RECEIVE_HISTORY (WM_USER + 103)
#define WM_MESSAGE_STATUS  (WM_USER + 104)
class CChatClientDlg : public CDialogEx
{
public:
    CChatClientDlg(CWnd* pParent = nullptr);
    virtual ~CChatClientDlg();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    void enterSend();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CLIENT_DIALOG };
#endif

    SOCKET m_hSocket;
    CString m_strCurrentUsername;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    HICON m_hIcon;
    CRichEditCtrl m_richChat;
    CListCtrl m_listOnline;
    CImageList m_imageList;
    CEdit m_editMessage;
    CString m_strSelectedFriend;
    HANDLE m_hRecvThread;
    DWORD m_dwRecvThreadId;
    volatile LONG m_bRunning;
    static DWORD WINAPI RecvThreadProc(LPVOID lpParam);
    void RecvThreadFunc();
    bool sendMessage(const string& receiver, const string& content);
    void loadLichSu(const CString& strFriend);
    void hienThiTin(const CString& sender, const CString& content, const CString& time, int isRead = 0);
    void themDongChat(const CString& text, COLORREF color, bool bBold, WORD alignment, int fontSize = 200);
    void updateDanhSachBan(const CString& user, bool bOnline);
    int layIconUser(const CString& username);
    void chonBan(int nIndex);

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedButtonSend();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnReceiveMessage(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnServerDisconnect(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUserStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnReceiveHistory(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageStatus(WPARAM wParam, LPARAM lParam);
    afx_msg void OnNMClickListFriends(NMHDR *pNMHDR, LRESULT *pResult);

    DECLARE_MESSAGE_MAP()
};
