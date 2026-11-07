#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "CLoginDlg.h"
#include "CRegisterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

    CLoginDlg loginDlg;
    INT_PTR nLoginResult = loginDlg.DoModal();

    if (nLoginResult == IDOK) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {}
        CChatClientDlg chatDlg;
        chatDlg.m_hSocket            = loginDlg.m_hSocket;
        chatDlg.m_strCurrentUsername = loginDlg.m_strUsername;
        loginDlg.m_hSocket = INVALID_SOCKET;
        m_pMainWnd = &chatDlg;
        chatDlg.DoModal();
    }
    if (pShellManager != nullptr) {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}
