#include "pch.h"
#include "Client.h"
#include "CRegisterDlg.h"
#include "afxdialogex.h"
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

IMPLEMENT_DYNAMIC(CRegisterDlg, CDialogEx)

CRegisterDlg::CRegisterDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_REGISTER, pParent)
{
}

CRegisterDlg::~CRegisterDlg()
{
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_REG_USER, m_editRegisterUser);
    DDX_Control(pDX, IDC_EDIT_REG_PASS, m_editRegisterPass);
    DDX_Control(pDX, IDC_EDIT_REG_CONFIRM, m_editRegisterConfirm);
}

BEGIN_MESSAGE_MAP(CRegisterDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_DO_REGISTER, &CRegisterDlg::OnBnClickedButtonDoRegister)
END_MESSAGE_MAP()

BOOL CRegisterDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CRegisterDlg::OnBnClickedButtonDoRegister()
{
    CString strUser, strPass, strConfirm;
    m_editRegisterUser.GetWindowTextW(strUser);
    m_editRegisterPass.GetWindowTextW(strPass);
    m_editRegisterConfirm.GetWindowTextW(strConfirm);
    if (strUser.IsEmpty() || strPass.IsEmpty() || strConfirm.IsEmpty()) {
        AfxMessageBox(_T("Dien thieu thong tin"));
        return;
    }
    string userStr = KetNoi::toUtf8(strUser);
    string passStr = KetNoi::toUtf8(strPass);
    regex userRegex("^[a-zA-Z0-9_]{1,100}$");
    if (!regex_match(userStr, userRegex)) {
        AfxMessageBox(_T("username sai\nChi duoc dung chu cai (a-z, A-Z), so (0-9), gach duoi (_)\n"));
        m_editRegisterUser.SetFocus();
        return;
    }
    regex passRegex("^.{8,}$");
    if (!regex_match(passStr, passRegex)) {
        AfxMessageBox(_T("Password toi thieu 8 ky tu"));
        m_editRegisterPass.SetFocus();
        return;
    }
    if (strPass != strConfirm) {
        AfxMessageBox(_T("Confirm k khop mat khau"));
        m_editRegisterConfirm.SetFocus();
        return;
    }
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        AfxMessageBox(_T("Tao socket that bai"), MB_ICONERROR);
        return;
    }
    DWORD timeout = 5000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)SERVER_DEFAULT_PORT);
    if (inet_pton(AF_INET, SERVER_DEFAULT_IP, &serverAddr.sin_addr) != 1) {
        AfxMessageBox(_T("Dia chi IP khong hop le"), MB_ICONERROR);
        closesocket(s);
        return;
    }
    if (connect(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        CString msg;
        msg.Format(_T("Loi khong ket noi toi duoc server\n(Loi: %d)"), WSAGetLastError());
        AfxMessageBox(msg, MB_ICONERROR);
        closesocket(s);
        return;
    }
    json reqJson = {{"username", userStr}, {"password", passStr}};
    string dataGui = reqJson.dump();
    if (!KetNoi::sendPacket(s, ACTION_REGISTER, dataGui)) {
        AfxMessageBox(_T("Gui yeu cau dang ky that bai"), MB_ICONERROR);
        closesocket(s);
        return;
    }
    uint32_t actionNhan = 0;
    string jsonNhan;
    if (!KetNoi::receivePacket(s, actionNhan, jsonNhan)) {
        AfxMessageBox(_T("Khong nhan duoc phan hoi tu Server"), MB_ICONERROR);
        closesocket(s);
        return;
    }
    closesocket(s);
    json ketQua;
    try {
        ketQua = json::parse(jsonNhan);
    } catch (...) {
        AfxMessageBox(_T("phan hoi tu Server khong hop le"), MB_ICONERROR);
        return;
    }
    string status = ketQua.value("status", "");
    if (status == RES_SUCCESS) {
        AfxMessageBox(_T("Dang ky thanh cong"), MB_ICONINFORMATION);
        EndDialog(IDOK);
    } else {
        AfxMessageBox(_T("Dang ky that bai\nTen dang nhap co the da ton tai"), MB_ICONERROR);
        m_editRegisterUser.SetFocus();
    }
}
