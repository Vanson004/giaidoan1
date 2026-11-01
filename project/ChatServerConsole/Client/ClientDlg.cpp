#include "pch.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "KetNoi.h"
#include "nlohmann/json.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using json = nlohmann::json;

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif
protected:
    virtual void DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CChatClientDlg::CChatClientDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_CLIENT_DIALOG, pParent),
      m_hSocket(INVALID_SOCKET),
      m_hRecvThread(NULL),
      m_dwRecvThreadId(0),
      m_bRunning(0)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    InitializeCriticalSection(&m_csQueue);
}

CChatClientDlg::~CChatClientDlg()
{
    DeleteCriticalSection(&m_csQueue);
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHAT, m_richChat);
    DDX_Control(pDX, IDC_LIST_FRIENDS, m_listOnline);
    DDX_Control(pDX, IDC_EDIT_MESSAGES, m_editMessage);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON_SEND, &CChatClientDlg::OnBnClickedButtonSend)
    ON_LBN_SELCHANGE(IDC_LIST_FRIENDS, &CChatClientDlg::OnSelectFriend)
    ON_LBN_DBLCLK(IDC_LIST_FRIENDS, &CChatClientDlg::OnSelectFriend)
    ON_MESSAGE(WM_RECEIVE_MSG, &CChatClientDlg::OnReceiveMessage)
    ON_MESSAGE(WM_SERVER_DISCONN, &CChatClientDlg::OnServerDisconnect)
    ON_MESSAGE(WM_USER_STATUS, &CChatClientDlg::OnUserStatus)
    ON_MESSAGE(WM_RECEIVE_HISTORY, &CChatClientDlg::OnReceiveHistory)
    ON_MESSAGE(WM_MESSAGE_STATUS, &CChatClientDlg::OnMessageStatus)
END_MESSAGE_MAP()

void CChatClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CChatClientDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        dc.DrawIcon((rect.Width() - cxIcon + 1) / 2, (rect.Height() - cyIcon + 1) / 2, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CChatClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

BOOL CChatClientDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);
    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr) {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    m_richChat.SetBackgroundColor(FALSE, RGB(255, 255, 255));
    m_richChat.SetReadOnly(TRUE);
    m_richChat.SetEventMask(m_richChat.GetEventMask() | ENM_SELCHANGE | ENM_MOUSEEVENTS);
    m_richChat.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 20));
    if (m_hSocket == INVALID_SOCKET) {
        AfxMessageBox(_T("Socket khong hop le"), MB_ICONERROR);
        EndDialog(IDCANCEL);
        return FALSE;
    }
    InterlockedExchange(&m_bRunning, 1L);
    m_hRecvThread = CreateThread(NULL, 0, RecvThreadProc, (LPVOID)this, 0, &m_dwRecvThreadId);
    if (m_hRecvThread == NULL) {
        AfxMessageBox(_T("Khong the tao recv thread"), MB_ICONERROR);
        EndDialog(IDCANCEL);
        return FALSE;
    }

    return TRUE;
}

DWORD WINAPI CChatClientDlg::RecvThreadProc(LPVOID lpParam)
{
    CChatClientDlg* pDlg = reinterpret_cast<CChatClientDlg*>(lpParam);
    if (pDlg) pDlg->RecvThreadFunc();
    return 0;
}

void CChatClientDlg::RecvThreadFunc()
{
    while (InterlockedCompareExchange(&m_bRunning, 1L, 1L) == 1L) {
        uint32_t action = 0;
        string dataNhan;
        if (!KetNoi::receivePacket(m_hSocket, action, dataNhan)) {
            if (InterlockedCompareExchange(&m_bRunning, 1L, 1L) == 1L) {
                if (::IsWindow(m_hWnd)) PostMessage(WM_SERVER_DISCONN, 0, 0);
            }
            break;
        }
        if (InterlockedCompareExchange(&m_bRunning, 1L, 1L) == 0L) break;
        json resJson;
        try {
            if (!dataNhan.empty()) {
                resJson = json::parse(dataNhan);
            }
        } catch (...) {
            continue;
        }
        if (action == ACTION_CHAT) {
            string status = resJson.value("status", "");
            if (status == RES_SUCCESS || status == RES_FAIL) {
                continue;
            }
            string sender = resJson.value("sender", "");
            string receiver = resJson.value("receiver", "");
            string content = resJson.value("content", "");
            string timeStr = resJson.value("time", "");
            int isRead = resJson.value("isRead", 0);
            if (!sender.empty() && !content.empty()) {
                TinNhan msg;
                msg.sender = KetNoi::toCString(sender);
                msg.receiver = KetNoi::toCString(receiver);
                msg.content = KetNoi::toCString(content);
                msg.time = timeStr.empty() ? KetNoi::toCString(KetNoi::getTime()) : KetNoi::toCString(timeStr);
                msg.daXem = isRead;
                EnterCriticalSection(&m_csQueue);
                m_queueTin.push(msg);
                LeaveCriticalSection(&m_csQueue);
                if (::IsWindow(m_hWnd)) PostMessage(WM_RECEIVE_MSG, 0, 0);
            }
        }
        else if (action == ACTION_USER_STATUS) {
            string user = resJson.value("user","");
            string status = resJson.value("status","");
            if (!user.empty()) {
                TrangThaiUser evt;
                evt.username = KetNoi::toCString(user);
                evt.isOnline = (status == "online");
                EnterCriticalSection(&m_csQueue);
                m_queueTrangThai.push(evt);
                LeaveCriticalSection(&m_csQueue);

                if (::IsWindow(m_hWnd)) PostMessage(WM_USER_STATUS, 0, 0);
            }
        }
        else if (action == ACTION_MSG_READ) {
            string reader = resJson.value("reader", "");
            if (!reader.empty()) {
                ThongBaoDaXem evt;
                evt.nguoiXem = KetNoi::toCString(reader);
                EnterCriticalSection(&m_csQueue);
                m_queueDaXem.push(evt);
                LeaveCriticalSection(&m_csQueue);
                if (::IsWindow(m_hWnd)) PostMessage(WM_MESSAGE_STATUS, 0, 0);
            }
        }
        else if (action == ACTION_GET_HISTORY) {
            string status = resJson.value("status", "");
            string friendName = resJson.value("friend", "");
            if (status == RES_SUCCESS) {
                LichSuChat hEvt;
                hEvt.friendName = KetNoi::toCString(friendName);
                if (resJson.contains("messages") && resJson["messages"].is_array()) {
                    for (const auto& item : resJson["messages"]) {
                        TinNhan m;
                        m.sender = KetNoi::toCString(item.value("sender", ""));
                        m.receiver = KetNoi::toCString(item.value("receiver", ""));
                        m.content = KetNoi::toCString(item.value("content", ""));
                        m.time = KetNoi::toCString(item.value("time", ""));
                        m.daXem = item.value("isRead", 0);
                        hEvt.dsTinNhan.push_back(m);
                    }
                }
                EnterCriticalSection(&m_csQueue);
                m_queueLichSu.push(hEvt);
                LeaveCriticalSection(&m_csQueue);
                if (::IsWindow(m_hWnd)) PostMessage(WM_RECEIVE_HISTORY, 0, 0);
            }
        }
    }
}

LRESULT CChatClientDlg::OnReceiveMessage(WPARAM, LPARAM)
{
    queue<TinNhan> localQueue;
    {
        EnterCriticalSection(&m_csQueue);
        swap(localQueue, m_queueTin);
        LeaveCriticalSection(&m_csQueue);
    }
    while (!localQueue.empty()) {
        TinNhan msg = localQueue.front();
        localQueue.pop();
        if (m_strSelectedFriend.CompareNoCase(msg.sender) == 0 || m_strSelectedFriend.CompareNoCase(msg.receiver) == 0) {
            if (m_strSelectedFriend.CompareNoCase(msg.sender) == 0) {
                loadLichSu(m_strSelectedFriend);
            } else {
                hienThiTin(msg.sender, msg.content, msg.time, msg.daXem);
            }
        }
    }
    return 0;
}

LRESULT CChatClientDlg::OnReceiveHistory(WPARAM, LPARAM)
{
    queue<LichSuChat> localQueue;
    {
        EnterCriticalSection(&m_csQueue);
        swap(localQueue, m_queueLichSu);
        LeaveCriticalSection(&m_csQueue);
    }
    while (!localQueue.empty()) {
        LichSuChat evt = localQueue.front();
        localQueue.pop();
        if (evt.friendName.CompareNoCase(m_strSelectedFriend) == 0) {
            m_richChat.SetWindowTextW(_T(""));
            for (const auto&m : evt.dsTinNhan) {
                hienThiTin(m.sender,m.content,m.time, m.daXem);
            }
        }
    }
    return 0;
}

LRESULT CChatClientDlg::OnMessageStatus(WPARAM, LPARAM)
{
    queue<ThongBaoDaXem> localQueue;
    {
        EnterCriticalSection(&m_csQueue);
        swap(localQueue, m_queueDaXem);
        LeaveCriticalSection(&m_csQueue);
    }
    while (!localQueue.empty()) {
        ThongBaoDaXem evt = localQueue.front();
        localQueue.pop();

        if (!m_strSelectedFriend.IsEmpty() && evt.nguoiXem.CompareNoCase(m_strSelectedFriend) == 0) {
            loadLichSu(m_strSelectedFriend);
        }
    }
    return 0;
}

LRESULT CChatClientDlg::OnUserStatus(WPARAM, LPARAM)
{
    queue<TrangThaiUser> localQueue;
    {
        EnterCriticalSection(&m_csQueue);
        swap(localQueue, m_queueTrangThai);
        LeaveCriticalSection(&m_csQueue);
    }
    while (!localQueue.empty()) {
        TrangThaiUser evt = localQueue.front();
        localQueue.pop();
        updateDanhSachBan(evt.username, evt.isOnline);
    }
    return 0;
}

LRESULT CChatClientDlg::OnServerDisconnect(WPARAM, LPARAM)
{
    if (InterlockedCompareExchange(&m_bRunning, 0L, 0L) == 0L) return 0;
    InterlockedExchange(&m_bRunning, 0L);
    CWnd* pSend = GetDlgItem(IDC_BUTTON_SEND);
    if (pSend) pSend->EnableWindow(FALSE);
    AfxMessageBox(_T("Mat ket noi voi Server"), MB_ICONWARNING);
    return 0;
}

void CChatClientDlg::OnSelectFriend()
{
    int sel = m_listOnline.GetCurSel();
    if (sel == LB_ERR) return;
    CString strItem;
    m_listOnline.GetText(sel, strItem);

    int spacePos = strItem.Find(_T(' '));
    CString friendName = (spacePos != -1) ? strItem.Left(spacePos) : strItem;
    friendName.Trim();
    if (friendName.IsEmpty() || friendName.CompareNoCase(m_strCurrentUsername) == 0) {
        return;
    }
    if (m_strSelectedFriend.CompareNoCase(friendName) == 0) {
        loadLichSu(m_strSelectedFriend);
        m_editMessage.SetFocus();
        return;
    }
    m_strSelectedFriend = friendName;
    SetWindowTextW(m_strSelectedFriend);
    m_richChat.SetWindowTextW(_T(""));
    loadLichSu(m_strSelectedFriend);
    m_editMessage.SetFocus();
}

void CChatClientDlg::loadLichSu(const CString& strFriend)
{
    if (m_hSocket == INVALID_SOCKET || strFriend.IsEmpty()) return;
    string u1 = KetNoi::toUtf8(m_strCurrentUsername);
    string u2 = KetNoi::toUtf8(strFriend);
    json req = {{"user1", u1}, {"user2", u2}};
    KetNoi::sendPacket(m_hSocket, ACTION_GET_HISTORY, req.dump());
}

void CChatClientDlg::OnBnClickedButtonSend()
{
    if (m_hSocket == INVALID_SOCKET || m_strSelectedFriend.IsEmpty()) {
        return;
    }
    CString strContent;
    m_editMessage.GetWindowTextW(strContent);
    strContent.Trim();
    if (strContent.IsEmpty()) {
        m_editMessage.SetFocus();
        return;
    }
    string receiverUtf8 = KetNoi::toUtf8(m_strSelectedFriend);
    string contentUtf8 = KetNoi::toUtf8(strContent);
    string timeStr = KetNoi::getTime();
    if (!sendMessage(receiverUtf8, contentUtf8)) {
        return;
    }
    hienThiTin(m_strCurrentUsername, strContent, KetNoi::toCString(timeStr), 0);
    m_editMessage.SetWindowTextW(_T(""));
    m_editMessage.SetFocus();
}

bool CChatClientDlg::sendMessage(const string& receiver, const string& content)
{
    string senderUtf8 = KetNoi::toUtf8(m_strCurrentUsername);
    string timeStr = KetNoi::getTime();
    json packet = {{"sender", senderUtf8},{"receiver", receiver},{"content", content},{"time", timeStr}};
    return KetNoi::sendPacket(m_hSocket, ACTION_CHAT, packet.dump());
}

void CChatClientDlg::themDongChat(const CString& text, COLORREF color, bool bBold, WORD alignment)
{
    long nStart = m_richChat.GetTextLength();
    m_richChat.SetSel(nStart, nStart);

    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = alignment;
    if (alignment == PFA_RIGHT) {
        pf.dwMask |= PFM_RIGHTINDENT;
        pf.dxRightIndent = 300;
    }
    m_richChat.SendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
    CHARFORMAT2 cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_FACE;
    cf.crTextColor = color;
    cf.dwEffects = bBold ? CFE_BOLD : 0;
    cf.yHeight = 200;
    _tcscpy_s(cf.szFaceName, _T("Segoe UI"));
    m_richChat.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    m_richChat.ReplaceSel(text + _T("\r\n"));
    m_richChat.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CChatClientDlg::hienThiTin(const CString& sender, const CString& content, const CString& time, int isRead)
{
    CString shortTime = time;
    int spaceIndex = shortTime.Find(_T(' '));
    if (spaceIndex != -1) {
        shortTime = shortTime.Mid(spaceIndex + 1);
    }
    if (sender.CompareNoCase(m_strCurrentUsername) == 0) {
        CString header = _T("Ban");
        if (!shortTime.IsEmpty()) {
            if (isRead == 1) {
                header.Format(_T("Ban (%s) (Da xem)"), static_cast<LPCTSTR>(shortTime));
            } else {
                header.Format(_T("Ban (%s) (Chua xem)"), static_cast<LPCTSTR>(shortTime));
            }
        }
        themDongChat(header, RGB(0, 102, 204), true, PFA_RIGHT);
        themDongChat(content, RGB(20, 20, 20), false, PFA_RIGHT);
        themDongChat(_T(""), RGB(0, 0, 0), false, PFA_RIGHT);
    } else {
        CString header = sender;
        if (!shortTime.IsEmpty()) {
            header.Format(_T("%s (%s)"), static_cast<LPCTSTR>(sender), static_cast<LPCTSTR>(shortTime));
        }
        themDongChat(header, RGB(204, 0, 0), true, PFA_LEFT);
        themDongChat(content, RGB(20, 20, 20), false, PFA_LEFT);
        themDongChat(_T(""), RGB(0, 0, 0), false, PFA_LEFT);
    }
}

void CChatClientDlg::updateDanhSachBan(const CString& user, bool bOnline)
{
    if (user.IsEmpty() || user.CompareNoCase(m_strCurrentUsername) == 0) {
        return;
    }
    CString newText = user + (bOnline ? _T(" (online)") : _T(" (offline)"));
    for (int i = 0; i < m_listOnline.GetCount(); i++) {
        CString s;
        m_listOnline.GetText(i, s);
        int spacePos = s.Find(_T(' '));
        CString existingName = (spacePos != -1) ? s.Left(spacePos) : s;
        if (existingName.CompareNoCase(user) == 0) {
            m_listOnline.DeleteString(i);
            m_listOnline.InsertString(i, newText);
            return;
        }
    }
    m_listOnline.AddString(newText);
}

void CChatClientDlg::OnDestroy()
{
    InterlockedExchange(&m_bRunning, 0L);
    if (m_hSocket != INVALID_SOCKET) {
        shutdown(m_hSocket, SD_BOTH);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
    if (m_hRecvThread != NULL) {
        DWORD dwWait;
        do {
            dwWait = MsgWaitForMultipleObjects(1, &m_hRecvThread, FALSE, 3000, QS_ALLINPUT);
            if (dwWait == WAIT_OBJECT_0 + 1) {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        } while (dwWait == WAIT_OBJECT_0 + 1);
        CloseHandle(m_hRecvThread);
        m_hRecvThread = NULL;
    }
    CDialogEx::OnDestroy();
}
