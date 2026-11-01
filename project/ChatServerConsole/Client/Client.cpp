// Client.cpp - Dieu phoi luong chinh cua ung dung
//
// Thu tu hien thi:
//   1. Hien thi CLoginDlg
//   2. Neu Login thanh cong (IDOK):
//      - Lay socket + username tu LoginDlg
//      - Truyen vao CChatClientDlg
//      - Hien thi CChatClientDlg
//   3. Thoat khi ChatClientDlg dong

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "CLoginDlg.h"
#include "CRegisterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ============================================================
// CClientApp
// ============================================================
BEGIN_MESSAGE_MAP(CClientApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CClientApp::CClientApp()
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CClientApp theApp;

BOOL CClientApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC  = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();
    AfxEnableControlContainer();
    AfxInitRichEdit2();

    CShellManager* pShellManager = new CShellManager;

    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    SetRegistryKey(_T("CMC_ChatClient"));

    // =========================================================
    // Buoc 1: Hien thi man hinh Login
    // LUU Y: KHONG dat m_pMainWnd = &loginDlg tai day!
    // Neu dat m_pMainWnd = &loginDlg, khi loginDlg dong lai (EndDialog),
    // MFC se mac dinh hieu la Main Window bi huy va goi PostQuitMessage(0).
    // Dieu do khien chatDlg vua mo ra thi gap ngay WM_QUIT va tat luon!
    // =========================================================
    CLoginDlg loginDlg;
    INT_PTR nLoginResult = loginDlg.DoModal();

    if (nLoginResult == IDOK) {
        // Xoa sach thong diep WM_QUIT (neu co) trong hang doi truoc khi chay chatDlg
        MSG msg;
        while (::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {}

        // =========================================================
        // Buoc 2: Login thanh cong -> Mo man hinh Chat
        //         Truyen socket va username tu LoginDlg sang ChatDlg
        // =========================================================
        CChatClientDlg chatDlg;
        chatDlg.m_hSocket            = loginDlg.m_hSocket;
        chatDlg.m_strCurrentUsername = loginDlg.m_strUsername;

        // QUAN TRONG: Dat socket cua loginDlg = INVALID_SOCKET de tranh
        // destructor cua loginDlg dong socket ma ta vua truyen sang chatDlg
        loginDlg.m_hSocket = INVALID_SOCKET;

        m_pMainWnd = &chatDlg;
        chatDlg.DoModal();
    }
    // Neu IDCANCEL hoac loi: thoat app binh thuong

    if (pShellManager != nullptr) {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}
