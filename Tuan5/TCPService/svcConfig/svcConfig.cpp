#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

TCHAR szCommand[10];
TCHAR szSvcName[80];

VOID __stdcall DisplayUsage(void);
VOID __stdcall DoInstallSvc(void);
VOID __stdcall DoQuerySvc(void);
VOID __stdcall DoUpdateSvcDesc(void);
VOID __stdcall DoDisableSvc(void);
VOID __stdcall DoEnableSvc(void);
VOID __stdcall DoDeleteSvc(void);

int __cdecl _tmain(int argc, TCHAR* argv[])
{
    printf("\n");
    if (argc != 3)
    {
        printf("ERROR:\tIncorrect number of arguments\n\n");
        DisplayUsage();
        return 0;
    }

    StringCchCopy(szCommand, 10, argv[1]);
    StringCchCopy(szSvcName, 80, argv[2]);

    if (lstrcmpi(szCommand, TEXT("install")) == 0)
        DoInstallSvc();
    else if (lstrcmpi(szCommand, TEXT("query")) == 0)
        DoQuerySvc();
    else if (lstrcmpi(szCommand, TEXT("describe")) == 0)
        DoUpdateSvcDesc();
    else if (lstrcmpi(szCommand, TEXT("disable")) == 0)
        DoDisableSvc();
    else if (lstrcmpi(szCommand, TEXT("enable")) == 0)
        DoEnableSvc();
    else if (lstrcmpi(szCommand, TEXT("delete")) == 0)
        DoDeleteSvc();
    else
    {
        _tprintf(TEXT("Unknown command (%s)\n\n"), szCommand);
        DisplayUsage();
    }
    return 0;
}

VOID __stdcall DisplayUsage()
{
    printf("Description:\n");
    printf("\tCommand-line tool that configures a service.\n\n");
    printf("Usage:\n");
    printf("\tsvcconfig [command] [service_name]\n\n");
    printf("\t[command]\n");
    printf("\t  install\n");
    printf("\t  query\n");
    printf("\t  describe\n");
    printf("\t  disable\n");
    printf("\t  enable\n");
    printf("\t  delete\n");
}

VOID __stdcall DoInstallSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    TCHAR* lastSlash = _tcsrchr(szUnquotedPath, '\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = '\0';
        _tcscat_s(szUnquotedPath, MAX_PATH, TEXT("svc.exe"));
    }

    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = CreateService(
        schSCManager,
        szSvcName,
        szSvcName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        szPath, 
        NULL, NULL, NULL, NULL, NULL);

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else
    {
        _tprintf(TEXT("Service installed successfully (Target: %s)\n"), szPath);
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID __stdcall DoQuerySvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    LPQUERY_SERVICE_CONFIG lpsc = NULL;
    LPSERVICE_DESCRIPTION lpsd = NULL;
    DWORD dwBytesNeeded = 0, cbBufSize = 0, dwError;

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,
        szSvcName,
        SERVICE_QUERY_CONFIG);

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!QueryServiceConfig(
        schService,
        NULL,
        0,
        &dwBytesNeeded))
    {
        dwError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            cbBufSize = dwBytesNeeded;
            lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, cbBufSize);
        }
        else
        {
            printf("QueryServiceConfig failed (%d)", dwError);
            goto cleanup;
        }
    }

    if (!QueryServiceConfig(
        schService,
        lpsc,
        cbBufSize,
        &dwBytesNeeded))
    {
        printf("QueryServiceConfig failed (%d)", GetLastError());
        goto cleanup;
    }

    if (!QueryServiceConfig2(
        schService,
        SERVICE_CONFIG_DESCRIPTION,
        NULL,
        0,
        &dwBytesNeeded))
    {
        dwError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            cbBufSize = dwBytesNeeded;
            lpsd = (LPSERVICE_DESCRIPTION)LocalAlloc(LMEM_FIXED, cbBufSize);
        }
        else
        {
            printf("QueryServiceConfig2 failed (%d)", dwError);
            goto cleanup;
        }
    }

    if (!QueryServiceConfig2(
        schService,
        SERVICE_CONFIG_DESCRIPTION,
        (LPBYTE)lpsd,
        cbBufSize,
        &dwBytesNeeded))
    {
        printf("QueryServiceConfig2 failed (%d)", GetLastError());
        goto cleanup;
    }

    // Print the configuration information.
    _tprintf(TEXT("%s configuration: \n"), szSvcName);
    _tprintf(TEXT("  Type: 0x%x\n"), lpsc->dwServiceType);
    _tprintf(TEXT("  Start Type: 0x%x\n"), lpsc->dwStartType);
    _tprintf(TEXT("  Error Control: 0x%x\n"), lpsc->dwErrorControl);
    _tprintf(TEXT("  Binary path: %s\n"), lpsc->lpBinaryPathName);
    _tprintf(TEXT("  Account: %s\n"), lpsc->lpServiceStartName);

    if (lpsd->lpDescription != NULL && lstrcmp(lpsd->lpDescription, TEXT("")) != 0)
        _tprintf(TEXT("  Description: %s\n"), lpsd->lpDescription);
    if (lpsc->lpLoadOrderGroup != NULL && lstrcmp(lpsc->lpLoadOrderGroup, TEXT("")) != 0)
        _tprintf(TEXT("  Load order group: %s\n"), lpsc->lpLoadOrderGroup);
    if (lpsc->dwTagId != 0)
        _tprintf(TEXT("  Tag ID: %d\n"), lpsc->dwTagId);
    if (lpsc->lpDependencies != NULL && lstrcmp(lpsc->lpDependencies, TEXT("")) != 0)
        _tprintf(TEXT("  Dependencies: %s\n"), lpsc->lpDependencies);

    if (lpsc != NULL) LocalFree(lpsc);
    if (lpsd != NULL) LocalFree(lpsd);

cleanup:
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID __stdcall DoDisableSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,
        szSvcName,
        SERVICE_CHANGE_CONFIG);

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!ChangeServiceConfig(
        schService,
        SERVICE_NO_CHANGE,
        SERVICE_DISABLED,
        SERVICE_NO_CHANGE,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL))
    {
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
    }
    else printf("Service disabled successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID __stdcall DoEnableSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,
        szSvcName,
        SERVICE_CHANGE_CONFIG);

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!ChangeServiceConfig(
        schService,
        SERVICE_NO_CHANGE,
        SERVICE_DEMAND_START,
        SERVICE_NO_CHANGE,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL))
    {
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
    }
    else printf("Service enabled successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID __stdcall DoUpdateSvcDesc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_DESCRIPTION sd;
    LPTSTR szDesc = (LPWSTR)TEXT("TCP Echo Server Service listening on port 9999");

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,
        szSvcName,
        SERVICE_CHANGE_CONFIG);

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    sd.lpDescription = szDesc;

    if (!ChangeServiceConfig2(
        schService,
        SERVICE_CONFIG_DESCRIPTION,
        &sd))
    {
        printf("ChangeServiceConfig2 failed\n");
    }
    else printf("Service description updated successfully.\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID __stdcall DoDeleteSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,
        szSvcName,
        DELETE);

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!DeleteService(schService))
    {
        printf("DeleteService failed (%d)\n", GetLastError());
    }
    else printf("Service deleted successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}