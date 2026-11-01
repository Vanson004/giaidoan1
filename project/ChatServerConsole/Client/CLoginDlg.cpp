#include "pch.h"
#include "Client.h"
#include "CLoginDlg.h"
#include "afxdialogex.h"
#include "CRegisterDlg.h"
#include "KetNoi.h"
#include "ClientDlg.h"
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
      m_bConnected(false)
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
    DDX_Control(pDX, IDC_EDIT_IP, m_editIP);
    DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
    DDX_Control(pDX, IDC_EDIT_LOGIN_USER, m_editLoginUser);
    DDX_Control(pDX, IDC_EDIT_LOGIN_PASS, m_editLoginPass);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CLoginDlg::OnBnClickedButtonConnect)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
    ON_BN_CLICKED(IDC_BUTTON_GO_REGISTER, &CLoginDlg::OnBnClickedButtonGoRegister)
END_MESSAGE_MAP()

BOOL CLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    m_editIP.SetWindowTextW(_T("127.0.0.1"));
    m_editPort.SetWindowTextW(_T("9999"));

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        AfxMessageBox(_T("khoi tao winsock that bai"));
        return FALSE;
    }

    return TRUE;
}

void CLoginDlg::OnBnClickedButtonConnect()
{
    if (m_hSocket != INVALID_SOCKET) {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        m_bConnected = false;
    }
    CString strIP, strPort;
    m_editIP.GetWindowTextW(strIP);
    m_editPort.GetWindowTextW(strPort);
    if (strIP.IsEmpty() || strPort.IsEmpty()) {
        AfxMessageBox(_T("thieu dia chi IP hoac Port"), MB_ICONWARNING);
        return;
    }
    int port = _ttoi(strPort);
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        AfxMessageBox(_T("Tao socket that bai"), MB_ICONERROR);
        return;
    }
    DWORD timeout = 5000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    string ip = KetNoi::toUtf8(strIP);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1) {
        AfxMessageBox(_T("Dia chi IP khong hop le"), MB_ICONWARNING);
        closesocket(s);
        return;
    }
    if (connect(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        CString msg;
        msg.Format(_T("Khong the ket noi toi Server\n(Loi: %d)"), WSAGetLastError());
        AfxMessageBox(msg, MB_ICONERROR);
        closesocket(s);
        return;
    }
    m_hSocket = s;
    m_bConnected = true;
    AfxMessageBox(_T("Ket noi thanh cong"), MB_ICONINFORMATION);
}

void CLoginDlg::OnBnClickedButtonLogin()
{
    if (!m_bConnected || m_hSocket == INVALID_SOCKET) {
        if (m_hSocket != INVALID_SOCKET) {
            closesocket(m_hSocket);
            m_hSocket = INVALID_SOCKET;
            m_bConnected = false;
        }
        CString strIP, strPort;
        m_editIP.GetWindowTextW(strIP);
        m_editPort.GetWindowTextW(strPort);
        if (strIP.IsEmpty() || strPort.IsEmpty()) {
            AfxMessageBox(_T("thieu IP hoac port"), MB_ICONWARNING);
            return;
        }
        int port = _ttoi(strPort);
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            AfxMessageBox(_T("Tao socket that bai!"), MB_ICONERROR);
            return;
        }
        DWORD timeout = 5000;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

        string ip = KetNoi::toUtf8(strIP);
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons((u_short)port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1) {
            AfxMessageBox(_T("Dia chi IP khong hop le"), MB_ICONWARNING);
            closesocket(s);
            return;
        }
        if (connect(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            CString msg;
            msg.Format(_T("Khong the ket noi toi Server\n(Loi: %d)"), WSAGetLastError());
            AfxMessageBox(msg, MB_ICONERROR);
            closesocket(s);
            return;
        }
        m_hSocket = s;
        m_bConnected = true;
    }

    CString strUser, strPass;
    m_editLoginUser.GetWindowTextW(strUser);
    m_editLoginPass.GetWindowTextW(strPass);
    if (strUser.IsEmpty() || strPass.IsEmpty()) {
        AfxMessageBox(_T("thieu Username hoac Password"), MB_ICONWARNING);
        return;
    }
    string userStr = KetNoi::toUtf8(strUser);
    regex UserRegex("^[a-zA-Z0-9_]{1,100}$");
    if (!regex_match(userStr, UserRegex)) {
        AfxMessageBox(_T("Username khong hop le\nChi duoc dung chu cai,so,gach duoi\n"), MB_ICONWARNING);
        return;
    }
    string passStr = KetNoi::toUtf8(strPass);
    json reqJson = {{"username", userStr}, {"password", passStr}};
    string dataGui = reqJson.dump();
    if (!KetNoi::sendPacket(m_hSocket, ACTION_LOGIN, dataGui)) {
        AfxMessageBox(_T("Gui du lieu len Server that bai"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        m_bConnected = false;
        return;
    }
    DWORD timeout = 10000;
    setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    uint32_t actionNhan = 0;
    string jsonNhan;
    if (!KetNoi::receivePacket(m_hSocket, actionNhan, jsonNhan)) {
        AfxMessageBox(_T("Khong nhan duoc phan hoi tu Server"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        m_bConnected = false;
        return;
    }
    json ketQua;
    try {
        ketQua = json::parse(jsonNhan);
    } catch (...) {
        AfxMessageBox(_T("phan hoi tu Server khong hop le"), MB_ICONERROR);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        m_bConnected = false;
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
        m_bConnected = false;
        AfxMessageBox(_T("dang nhap that bai"), MB_ICONERROR);
    }
}

void CLoginDlg::OnBnClickedButtonGoRegister()
{
    CString strIP, strPort;
    m_editIP.GetWindowTextW(strIP);
    m_editPort.GetWindowTextW(strPort);
    CRegisterDlg registerDlg;
    registerDlg.m_strIP = strIP;
    registerDlg.m_strPort = strPort;
    registerDlg.DoModal();
}
