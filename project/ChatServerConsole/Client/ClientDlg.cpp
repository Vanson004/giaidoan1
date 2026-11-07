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
}

CChatClientDlg::~CChatClientDlg()
{
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
    ON_NOTIFY(NM_CLICK, IDC_LIST_FRIENDS, &CChatClientDlg::OnNMClickListFriends)
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
    int resIcons[4] = { IDI_ICON_User1, IDI_ICON_User2, IDI_ICON_User3, IDI_ICON_User4 };
    int myIconIdx = layIconUser(m_strCurrentUsername);
    HICON hMyIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(resIcons[myIconIdx]), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (hMyIcon) {
        m_hIcon = hMyIcon;
    }
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    if (!m_strCurrentUsername.IsEmpty()) {
        SetWindowTextW(m_strCurrentUsername);
    }

    m_richChat.SetBackgroundColor(FALSE, RGB(255, 255, 255));
    m_richChat.SetReadOnly(TRUE);
    m_richChat.SetEventMask(m_richChat.GetEventMask() | ENM_SELCHANGE | ENM_MOUSEEVENTS);
    m_richChat.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 15));

    m_imageList.Create(20, 20, ILC_COLOR32 | ILC_MASK, 4, 4);
    for (int i = 0; i < 4; i++) {
        HICON hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(resIcons[i]), IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
        if (hIcon) {
            m_imageList.Add(hIcon);
            DestroyIcon(hIcon);
        } else {
            m_imageList.Add(AfxGetApp()->LoadIcon(resIcons[i]));
        }
    }

    m_listOnline.SetImageList(&m_imageList, LVSIL_SMALL);
    m_listOnline.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    CRect rcList;
    m_listOnline.GetClientRect(&rcList);
    int colWidth = (rcList.Width() > 0) ? (rcList.Width() - 4) : 120;
    m_listOnline.InsertColumn(0, _T("Friends"), LVCFMT_LEFT, colWidth);
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

BOOL CChatClientDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        if (GetFocus() == &m_editMessage) {
            enterSend();
            return TRUE;
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}

void CChatClientDlg::enterSend()
{
    OnBnClickedButtonSend();
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
            string sender = resJson.value("sender", "");
            string receiver = resJson.value("receiver", "");
            string content = resJson.value("content", "");
            string timeStr = resJson.value("time", "");
            int isRead = 0;
            if (resJson.contains("isRead")) {
                isRead = resJson["isRead"].get<int>();
            }
            if (!sender.empty() && !content.empty()) {
                TinNhan* pMsg = new TinNhan();
                pMsg->sender = KetNoi::toCString(sender);
                pMsg->receiver = KetNoi::toCString(receiver);
                pMsg->content = KetNoi::toCString(content);
                pMsg->time = timeStr.empty() ? KetNoi::toCString(KetNoi::getTime()) : KetNoi::toCString(timeStr);
                pMsg->daXem = isRead;
                if (::IsWindow(m_hWnd)) {
                    PostMessage(WM_RECEIVE_MSG, 0, (LPARAM)pMsg);
                } else {
                    delete pMsg;
                }
            }
        }
        else if (action == ACTION_USER_STATUS) {
            string user = resJson.value("user","");
            string status = resJson.value("status","");
            if (!user.empty()) {
                TrangThaiUser* pEvt = new TrangThaiUser();
                pEvt->username = KetNoi::toCString(user);
                pEvt->isOnline = (status == "online");
                if (::IsWindow(m_hWnd)) {
                    PostMessage(WM_USER_STATUS, 0, (LPARAM)pEvt);
                } else {
                    delete pEvt;
                }
            }
        }
        else if (action == ACTION_MSG_READ) {
            string reader = resJson.value("reader", "");
            if (!reader.empty()) {
                ThongBaoDaXem* pEvt = new ThongBaoDaXem();
                pEvt->nguoiXem = KetNoi::toCString(reader);
                if (::IsWindow(m_hWnd)) {
                    PostMessage(WM_MESSAGE_STATUS, 0, (LPARAM)pEvt);
                } else {
                    delete pEvt;
                }
            }
        }
        else if (action == ACTION_GET_HISTORY) {
            string status = resJson.value("status", "");
            string friendName = resJson.value("friend", "");
            if (status == RES_SUCCESS) {
                LichSuChat* pEvt = new LichSuChat();
                pEvt->friendName = KetNoi::toCString(friendName);
                if (resJson.contains("messages") && resJson["messages"].is_array()) {
                    for (const auto& item : resJson["messages"]) {
                        TinNhan m;
                        m.sender = KetNoi::toCString(item.value("sender", ""));
                        m.receiver = KetNoi::toCString(item.value("receiver", ""));
                        m.content = KetNoi::toCString(item.value("content", ""));
                        m.time = KetNoi::toCString(item.value("time", ""));
                        if (item.contains("isRead")) {
                            m.daXem = item["isRead"].get<int>();
                        }
                        pEvt->dsTinNhan.push_back(m);
                    }
                }
                if (::IsWindow(m_hWnd)) {
                    PostMessage(WM_RECEIVE_HISTORY, 0, (LPARAM)pEvt);
                } else {
                    delete pEvt;
                }
            }
        }
    }
}

LRESULT CChatClientDlg::OnReceiveMessage(WPARAM, LPARAM lParam)
{
    TinNhan* pMsg = reinterpret_cast<TinNhan*>(lParam);
    if (!pMsg) return 0;
    if (m_strSelectedFriend.CompareNoCase(pMsg->sender) == 0) {
        loadLichSu(m_strSelectedFriend);
    }

    delete pMsg;
    return 0;
}

LRESULT CChatClientDlg::OnReceiveHistory(WPARAM, LPARAM lParam)
{
    LichSuChat* pEvt = reinterpret_cast<LichSuChat*>(lParam);
    if (!pEvt) return 0;

    if (pEvt->friendName.CompareNoCase(m_strSelectedFriend) == 0) {
        m_richChat.SetWindowTextW(_T(""));
        for (const auto& m : pEvt->dsTinNhan) {
            hienThiTin(m.sender, m.content, m.time, m.daXem);
        }
    }
    delete pEvt;
    return 0;
}

LRESULT CChatClientDlg::OnMessageStatus(WPARAM, LPARAM lParam)
{
    ThongBaoDaXem* pEvt = reinterpret_cast<ThongBaoDaXem*>(lParam);
    if (!pEvt) return 0;

    if (!m_strSelectedFriend.IsEmpty() && pEvt->nguoiXem.CompareNoCase(m_strSelectedFriend) == 0) {
        loadLichSu(m_strSelectedFriend);
    }
    delete pEvt;
    return 0;
}

LRESULT CChatClientDlg::OnUserStatus(WPARAM, LPARAM lParam)
{
    TrangThaiUser* pEvt = reinterpret_cast<TrangThaiUser*>(lParam);
    if (!pEvt) return 0;

    updateDanhSachBan(pEvt->username, pEvt->isOnline);
    delete pEvt;
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

void CChatClientDlg::OnNMClickListFriends(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int nItem = (pNMItemActivate != nullptr) ? pNMItemActivate->iItem : -1;
    if (nItem == -1) {
        POSITION pos = m_listOnline.GetFirstSelectedItemPosition();
        if (pos != NULL) {
            nItem = m_listOnline.GetNextSelectedItem(pos);
        }
    }
    if (nItem != -1) {
        chonBan(nItem);
    }
    *pResult = 0;
}

void CChatClientDlg::chonBan(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_listOnline.GetItemCount()) return;
    CString strItem = m_listOnline.GetItemText(nIndex, 0);

    int spacePos = strItem.Find(_T(' '));
    CString friendName = (spacePos != -1) ? strItem.Left(spacePos) : strItem;
    friendName.Trim();
    if (friendName.IsEmpty()) {
        return;
    }
    if (m_strSelectedFriend.CompareNoCase(friendName) == 0) {
        m_editMessage.SetFocus();
        return;
    }
    m_strSelectedFriend = friendName;
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

void CChatClientDlg::themDongChat(const CString& text, COLORREF color, bool bBold, WORD alignment, int fontSize)
{
    long nStart = m_richChat.GetTextLength();
    m_richChat.SetSel(nStart, nStart);

    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = alignment;
    if (alignment == PFA_RIGHT) {
        pf.dwMask |= PFM_RIGHTINDENT;
        pf.dxRightIndent = 250;
    } else {
        pf.dwMask |= PFM_STARTINDENT;
        pf.dxStartIndent = 150;
    }
    m_richChat.SendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);

    CHARFORMAT2 cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_FACE;
    cf.crTextColor = color;
    cf.dwEffects = bBold ? CFE_BOLD : 0;
    cf.yHeight = fontSize;
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
        // Tin nhắn của mình gửi đi (Căn phải phong cách hiện đại)
        themDongChat(content, RGB(25, 25, 25), false, PFA_RIGHT, 210);

        CString subInfo = shortTime;
        if (isRead == 1) {
            subInfo = shortTime.IsEmpty() ? _T("Da xem") : shortTime + _T("  Da xem");
            themDongChat(subInfo, RGB(0, 120, 215), false, PFA_RIGHT, 150);
        } else {
            subInfo = shortTime.IsEmpty() ? _T("Da nhan") : shortTime + _T("  Da nhan");
            themDongChat(subInfo, RGB(130, 130, 130), false, PFA_RIGHT, 150);
        }
        themDongChat(_T(""), RGB(0, 0, 0), false, PFA_RIGHT, 80);
    } else {
        // Tin nhắn của bạn bè gửi đến (Căn trái phong cách hiện đại)
        themDongChat(sender, RGB(0, 102, 204), true, PFA_LEFT, 180);
        themDongChat(content, RGB(25, 25, 25), false, PFA_LEFT, 210);
        if (!shortTime.IsEmpty()) {
            themDongChat(shortTime, RGB(130, 130, 130), false, PFA_LEFT, 150);
        }
        themDongChat(_T(""), RGB(0, 0, 0), false, PFA_LEFT, 80);
    }
}

int CChatClientDlg::layIconUser(const CString& username)
{
    if (username.IsEmpty()) return 0;
    int sum = 0;
    for (int i = 0; i < username.GetLength(); i++) {
        sum += static_cast<int>(towlower(username[i]));
    }
    return abs(sum) % 4;
}

void CChatClientDlg::updateDanhSachBan(const CString& user, bool bOnline)
{
    if (user.IsEmpty() || user.CompareNoCase(m_strCurrentUsername) == 0) {
        return;
    }
    CString newText = user + (bOnline ? _T(" (online)") : _T(" (offline)"));
    int imgIdx = layIconUser(user);

    for (int i = 0; i < m_listOnline.GetItemCount(); i++) {
        CString s = m_listOnline.GetItemText(i, 0);
        int spacePos = s.Find(_T(' '));
        CString existingName = (spacePos != -1) ? s.Left(spacePos) : s;
        if (existingName.CompareNoCase(user) == 0) {
            LVITEM item{};
            item.iItem = i;
            item.iSubItem = 0;
            item.mask = LVIF_TEXT | LVIF_IMAGE;
            item.pszText = (LPTSTR)(LPCTSTR)newText;
            item.iImage = imgIdx;
            m_listOnline.SetItem(&item);
            return;
        }
    }
    int nItem = m_listOnline.GetItemCount();
    m_listOnline.InsertItem(nItem, newText, imgIdx);
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
