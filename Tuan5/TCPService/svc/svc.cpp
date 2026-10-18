#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")

#define SVCNAME TEXT("TCPService")
#define SVC_ERROR ((DWORD)0xC0000001L)
SERVICE_STATUS        gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;
HANDLE                ghSvcStopEvent = NULL;
SOCKET                g_ServerSock = INVALID_SOCKET;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);
DWORD WINAPI ClientSessionThread(LPVOID lpParam);
DWORD WINAPI TcpServerWorkerThread(LPVOID lpParam);

int __cdecl _tmain(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent((LPTSTR)TEXT("StartServiceCtrlDispatcher"));
    }
    return 0;
}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
    if (!gSvcStatusHandle) {
        SvcReportEvent((LPTSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
    SvcInit(dwArgc, lpszArgv);
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    ghSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TcpServerWorkerThread, NULL, 0, NULL);
    while (1) {
        WaitForSingleObject(ghSvcStopEvent, INFINITE);
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }
}

DWORD WINAPI ClientSessionThread(LPVOID lpParam)
{
    SOCKET clientSock = (SOCKET)(LPVOID)lpParam;
    char buffer[1024];

    while (true)
    {
        int bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
        {
            break;
        }

        buffer[bytesReceived] = '\0';
        send(clientSock, buffer, bytesReceived, 0);
    }

    closesocket(clientSock);
    return 0;
}

DWORD WINAPI TcpServerWorkerThread(LPVOID lpParam)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    g_ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_ServerSock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9999);

    if (bind(g_ServerSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(g_ServerSock);
        WSACleanup();
        return 1;
    }

    if (listen(g_ServerSock, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(g_ServerSock);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) {
            break;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(g_ServerSock, &readfds);

        timeval timeout = { 1, 0 };
        int activity = select(0, &readfds, NULL, NULL, &timeout);

        if (activity > 0 && FD_ISSET(g_ServerSock, &readfds))
        {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            SOCKET clientSock = accept(g_ServerSock, (SOCKADDR*)&clientAddr, &clientAddrLen);
            if (clientSock != INVALID_SOCKET)
            {
                CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientSessionThread, (LPVOID)clientSock, 0, NULL);
            }
        }
    }

    if (g_ServerSock != INVALID_SOCKET) {
        closesocket(g_ServerSock);
        g_ServerSock = INVALID_SOCKET;
    }
    WSACleanup();
    return 0;
}

VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        if (g_ServerSock != INVALID_SOCKET) {
            closesocket(g_ServerSock);
            g_ServerSock = INVALID_SOCKET;
        }
        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        break;
    default:
        break;
    }
}

VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,       
            EVENTLOG_ERROR_TYPE, 
            0,             
            SVC_ERROR,        
            NULL,              
            2,             
            0,               
            lpszStrings,     
            NULL);             

        DeregisterEventSource(hEventSource);
    }
}