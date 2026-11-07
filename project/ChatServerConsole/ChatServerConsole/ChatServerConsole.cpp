#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"
#include "Protocol.h"
#include "Database.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;
using json = nlohmann::json;

#define SVCNAME TEXT("ChatServerService")
#define SVC_ERROR ((DWORD)0xC0000001L)

static SERVICE_STATUS gSvcStatus;
static SERVICE_STATUS_HANDLE gSvcStatusHandle;
static HANDLE ghSvcStopEvent = NULL;

static SOCKET g_listenSocket = INVALID_SOCKET;
static unordered_map<string, SOCKET> g_onlineUsers;
static CRITICAL_SECTION g_csOnline;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);

static DWORD WINAPI TcpServerWorkerThread(LPVOID lpParam);
static DWORD WINAPI HandleClientProc(LPVOID lpParam);

struct ClientThreadParam {
    SOCKET clientSocket;
};

static void RegisterOnline(const string& username, SOCKET s) {
    EnterCriticalSection(&g_csOnline);
    g_onlineUsers[username] = s;
    LeaveCriticalSection(&g_csOnline);
}

static void UnregisterOnline(const string& username) {
    EnterCriticalSection(&g_csOnline);
    g_onlineUsers.erase(username);
    LeaveCriticalSection(&g_csOnline);
}

static SOCKET FindOnlineSocket(const string& username) {
    SOCKET result = INVALID_SOCKET;
    EnterCriticalSection(&g_csOnline);
    auto it = g_onlineUsers.find(username);
    if (it != g_onlineUsers.end()) result = it->second;
    LeaveCriticalSection(&g_csOnline);
    return result;
}

static bool receiveData(SOCKET s, char* buf, int size) {
    int received = 0;
    while (received < size) {
        int r = recv(s, buf + received, size - received, 0);
        if (r <= 0) return false;
        received += r;
    }
    return true;
}

static bool sendPacket(SOCKET s, uint32_t action, const string& jsonStr) {
    PacketHeader hdr;
    hdr.action = action;
    hdr.dataSize = (uint32_t)jsonStr.size();

    if (send(s, (const char*)&hdr, sizeof(hdr), 0) == SOCKET_ERROR) return false;

    if (hdr.dataSize > 0) {
        if (send(s, jsonStr.c_str(), (int)jsonStr.size(), 0) == SOCKET_ERROR) return false;
    }
    return true;
}

static void ThongBaoUserStatus(const string& username, const string& status, SOCKET excludeSocket = INVALID_SOCKET) {
    json bcast = {
        {"user", username},
        {"status", status}
    };
    string payload = bcast.dump();

    EnterCriticalSection(&g_csOnline);
    for (const auto& kv : g_onlineUsers) {
        if (kv.second != excludeSocket) {
            sendPacket(kv.second, ACTION_USER_STATUS, payload);
        }
    }
    LeaveCriticalSection(&g_csOnline);
}

static DWORD WINAPI HandleClientProc(LPVOID lpParam)
{
    ClientThreadParam* pParam = reinterpret_cast<ClientThreadParam*>(lpParam);
    SOCKET clientSocket = pParam->clientSocket;
    delete pParam;

    string loggedInUser = "";

    while (true) {
        if (ghSvcStopEvent != NULL && WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) {
            break;
        }

        PacketHeader hdr{};
        if (!receiveData(clientSocket, (char*)&hdr, sizeof(hdr))) {
            break;
        }

        if (hdr.dataSize == 0 || hdr.dataSize > 1024 * 1024) {
            break;
        }

        vector<char> buf(hdr.dataSize + 1, 0);
        if (!receiveData(clientSocket, buf.data(), (int)hdr.dataSize)) {
            break;
        }
        buf[hdr.dataSize] = '\0';
        string jsonStr(buf.data(), hdr.dataSize);

        json reqJson;
        try {
            reqJson = json::parse(jsonStr);
        } catch (...) {
            continue;
        }

        if (hdr.action == ACTION_REGISTER) {
            string username = reqJson.value("username", "");
            string password = reqJson.value("password", "");

            if (username.empty() || password.empty()) {
                json res = {
                    {"status", RES_FAIL},
                    {"action", "REGISTER"},
                    {"reason", "missing_fields"}
                };
                sendPacket(clientSocket, ACTION_REGISTER, res.dump());
                continue;
            }

            bool ok = Database::registerUser(username, password);
            string statusStr = ok ? RES_SUCCESS : RES_FAIL;

            json res = {
                {"status", statusStr},
                {"action", "REGISTER"}
            };
            sendPacket(clientSocket, ACTION_REGISTER, res.dump());

            if (ok) {
                ThongBaoUserStatus(username, "offline");
            }
        }
        else if (hdr.action == ACTION_LOGIN) {
            string username = reqJson.value("username", "");
            string password = reqJson.value("password", "");

            if (username.empty() || password.empty()) {
                json res = {
                    {"status", RES_FAIL},
                    {"action", "LOGIN"},
                    {"reason", "missing_fields"}
                };
                sendPacket(clientSocket, ACTION_LOGIN, res.dump());
                continue;
            }

            bool ok = Database::loginUser(username, password);
            if (ok) {
                loggedInUser = username;
                RegisterOnline(username, clientSocket);

                json res = {
                    {"status", RES_SUCCESS},
                    {"action", "LOGIN"},
                    {"username", username}
                };
                sendPacket(clientSocket, ACTION_LOGIN, res.dump());

                vector<string> allUsers = Database::getAllUsers();
                for (const string& u : allUsers) {
                    if (u == username) continue;
                    bool isOnline = (FindOnlineSocket(u) != INVALID_SOCKET);
                    json statusJson = {
                        {"user", u},
                        {"status", isOnline ? "online" : "offline"}
                    };
                    sendPacket(clientSocket, ACTION_USER_STATUS, statusJson.dump());
                }

                ThongBaoUserStatus(username, "online", clientSocket);
            } else {
                json res = {
                    {"status", RES_FAIL},
                    {"action", "LOGIN"},
                    {"reason", "wrong_credentials"}
                };
                sendPacket(clientSocket, ACTION_LOGIN, res.dump());
            }
        }
        else if (hdr.action == ACTION_CHAT) {
            string sender = reqJson.value("sender", "");
            string receiver = reqJson.value("receiver", "");
            string content = reqJson.value("content", "");
            string timeStr = reqJson.value("time", "");
            if (sender.empty() || receiver.empty() || content.empty()) {
                continue;
            }
            SOCKET recvSocket = FindOnlineSocket(receiver);
            bool isOnline = (recvSocket != INVALID_SOCKET);
            Database::saveMessage(sender, receiver, content, 0);

            if (isOnline) {
                json msgJson = {
                    {"sender", sender},
                    {"receiver", receiver},
                    {"content", content},
                    {"time", timeStr},
                    {"isRead", 0}
                };
                sendPacket(recvSocket, ACTION_CHAT, msgJson.dump());
            }
        }
        else if (hdr.action == ACTION_GET_HISTORY) {
            string user1 = reqJson.value("user1", "");
            string user2 = reqJson.value("user2", "");

            if (user1.empty() || user2.empty()) {
                json res = {
                    {"status", RES_FAIL},
                    {"action", "GET_HISTORY"},
                    {"reason", "missing_fields"}
                };
                sendPacket(clientSocket, ACTION_GET_HISTORY, res.dump());
                continue;
            }

            int updatedCount = Database::updateMessagesIsRead(user1, user2);
            if (updatedCount > 0) {
                SOCKET senderSocket = FindOnlineSocket(user2);
                if (senderSocket != INVALID_SOCKET) {
                    json readNotif = {
                        {"reader", user1},
                        {"sender", user2}
                    };
                    sendPacket(senderSocket, ACTION_MSG_READ, readNotif.dump());
                }
            }

            vector<HistoryMessage> msgs = Database::getChatHistory(user1, user2);
            json msgList = json::array();
            for (const auto& m : msgs) {
                msgList.push_back({
                    {"sender", m.sender},
                    {"receiver", m.receiver},
                    {"content", m.content},
                    {"time", m.timestamp},
                    {"isRead", m.isRead}
                });
            }
            json res = {
                {"status", RES_SUCCESS},
                {"action", "GET_HISTORY"},
                {"friend", user2},
                {"messages", msgList}
            };
            sendPacket(clientSocket, ACTION_GET_HISTORY, res.dump());
        }
    }

    if (!loggedInUser.empty()) {
        UnregisterOnline(loggedInUser);
        ThongBaoUserStatus(loggedInUser, "offline");
    }

    closesocket(clientSocket);
    return 0;
}

static DWORD WINAPI TcpServerWorkerThread(LPVOID lpParam)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        SvcReportEvent((LPTSTR)TEXT("WSAStartup"));
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return 1;
    }

    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET) {
        SvcReportEvent((LPTSTR)TEXT("socket"));
        WSACleanup();
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return 1;
    }

    int opt = 1;
    setsockopt(g_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9999);

    if (::bind(g_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        SvcReportEvent((LPTSTR)TEXT("bind"));
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        WSACleanup();
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return 1;
    }

    if (listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        SvcReportEvent((LPTSTR)TEXT("listen"));
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        WSACleanup();
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return 1;
    }

    while (true) {
        if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) {
            break;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(g_listenSocket, &readfds);

        timeval timeout = { 1, 0 };
        int activity = select(0, &readfds, NULL, NULL, &timeout);

        if (activity == SOCKET_ERROR) {
            if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) break;
            continue;
        }

        if (activity > 0 && FD_ISSET(g_listenSocket, &readfds)) {
            sockaddr_in clientAddr{};
            int clientAddrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

            if (clientSocket == INVALID_SOCKET) {
                if (WaitForSingleObject(ghSvcStopEvent, 0) == WAIT_OBJECT_0) break;
                continue;
            }

            ClientThreadParam* pParam = new ClientThreadParam();
            pParam->clientSocket = clientSocket;

            HANDLE hThread = CreateThread(NULL, 0, HandleClientProc, (LPVOID)pParam, 0, NULL);
            if (hThread == NULL) {
                closesocket(clientSocket);
                delete pParam;
            } else {
                CloseHandle(hThread);
            }
        }
    }

    if (g_listenSocket != INVALID_SOCKET) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
    return 0;
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    ghSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghSvcStopEvent == NULL) {
        SvcReportEvent((LPTSTR)TEXT("CreateEvent"));
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    InitializeCriticalSection(&g_csOnline);

    if (!Database::initDB()) {
        SvcReportEvent((LPTSTR)TEXT("Database::initDB"));
        ReportSvcStatus(SERVICE_STOPPED, ERROR_DATABASE_FAILURE, 0);
        DeleteCriticalSection(&g_csOnline);
        CloseHandle(ghSvcStopEvent);
        ghSvcStopEvent = NULL;
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    HANDLE hWorker = CreateThread(NULL, 0, TcpServerWorkerThread, NULL, 0, NULL);
    if (hWorker != NULL) {
        CloseHandle(hWorker);
    }

    while (1) {
        WaitForSingleObject(ghSvcStopEvent, INFINITE);
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);

        DeleteCriticalSection(&g_csOnline);
        CloseHandle(ghSvcStopEvent);
        ghSvcStopEvent = NULL;
        return;
    }
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

int __cdecl _tmain(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY DispatchTable[] = {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(DispatchTable)) {
        SvcReportEvent((LPTSTR)TEXT("StartServiceCtrlDispatcher"));
    }
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
    else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 5000);

        if (g_listenSocket != INVALID_SOCKET) {
            closesocket(g_listenSocket);
            g_listenSocket = INVALID_SOCKET;
        }

        if (ghSvcStopEvent != NULL) {
            SetEvent(ghSvcStopEvent);
        }
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
    if (NULL != hEventSource) {
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
