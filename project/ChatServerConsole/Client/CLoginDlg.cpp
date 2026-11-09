#include "pch.h"
#include "Client.h"
#include "CLoginDlg.h"
#include "afxdialogex.h"
#include "CRegisterDlg.h"
#include "KetNoi.h"
#include <regex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using json = nlohmann::json;

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_LOGIN, pParent),
      m_hSocket(INVALID_SOCKET),
      m_bLoginSuccess(false)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CLoginDlg::~CLoginDlg()
{
    if (!m_bLoginSuccess && m_hSocket != INVALID_SOCKET) {
        closesocket(m_hSocket);
    }
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_LOGIN_USER, m_editLoginUser);
    DDX_Control(pDX, IDC_EDIT_LOGIN_PASS, m_editLoginPass);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
    ON_BN_CLICKED(IDC_BUTTON_GO_REGISTER, &CLoginDlg::OnBnClickedButtonGoRegister)
END_MESSAGE_MAP()

BOOL CLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        AfxMessageBox(_T("Khoi tao Winsock that bai"), MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}

void CLoginDlg::OnBnClickedButtonLogin()
{
    CString strUser, strPass;
    m_editLoginUser.GetWindowTextW(strUser);
    m_editLoginPass.GetWindowTextW(strPass);
    strUser.Trim();
    strPass.Trim();
    if (strUser.IsEmpty() || strPass.IsEmpty()) {
        AfxMessageBox(_T("Vui long nhap day du Username va Password"), MB_ICONWARNING);
        if (strUser.IsEmpty()) m_editLoginUser.SetFocus();
        else m_editLoginPass.SetFocus();
        return;
    }
    string userStr = KetNoi::toUtf8(strUser);
    regex userRegex("^[a-zA-Z0-9_]{1,100}$");
    if (!regex_match(userStr, userRegex)) {
        AfxMessageBox(_T("username khong hop le\nChi duoc dung chu cai (a-z, A-Z), chu so (0-9) va dau gach duoi (_)\n"), MB_ICONWARNING);
        m_editLoginUser.SetFocus();
        return;
    }
    if (m_hSocket != INVALID_SOCKET) {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        AfxMessageBox(_T("Tao socket that bai"), MB_ICONERROR);
        return;
    }
    DWORD timeout = 5000;
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)SERVER_DEFAULT_PORT);
    if (inet_pton(AF_INET, SERVER_DEFAULT_IP, &serverAddr.sin_addr) != 1) {
        AfxMessageBox(_T("Dia chi IP khong hop le"), MB_ICONWARNING);
        closesocket(s);
        return;
    }
    if (connect(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        AfxMessageBox(_T("Khong the ket noi toi Server"), MB_ICONERROR);
        closesocket(s);
        return;
    }
    m_hSocket = s;
    string passStr = KetNoi::toUtf8(strPass);
    json reqJson = {{"username", userStr}, {"password", passStr}};
    string dataGui = reqJson.dump();
    if (!KetNoi::sendPacket(m_hSocket, ACTION_LOGIN, dataGui)) {
        AfxMessageBox(_T("Gui du lieu len Server that bai"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        return;
    }

    DWORD timeoutWait = 10000;
    setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutWait, sizeof(timeoutWait));
    uint32_t actionNhan = 0;
    string jsonNhan;
    if (!KetNoi::receivePacket(m_hSocket, actionNhan, jsonNhan)) {
        AfxMessageBox(_T("Khong nhan duoc phan hoi tu Server"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        return;
    }
    json ketQua;
    try {
        ketQua = json::parse(jsonNhan);
    } catch (...) {
        AfxMessageBox(_T("Phan hoi tu Server khong hop le"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        return;
    }
    string status = ketQua.value("status", "");
    if (status == RES_SUCCESS) {
        DWORD noTimeout = 0;
        setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&noTimeout, sizeof(noTimeout));
        m_bLoginSuccess = true;
        m_strUsername = strUser;

        EndDialog(IDOK);
    } else {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        AfxMessageBox(_T("Dang nhap that bai\nSai ten dang nhap hoac mat khau."), MB_ICONERROR);
        m_editLoginPass.SetWindowTextW(_T(""));
        m_editLoginPass.SetFocus();
    }
}

void CLoginDlg::OnBnClickedButtonGoRegister()
{
    CRegisterDlg registerDlg;
    registerDlg.DoModal();
}
