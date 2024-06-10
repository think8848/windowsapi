#include <Windows.h>
#include <tchar.h>
#include <wtsapi32.h>
#include <Userenv.h>

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")

// 服务入口点函数
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
// 服务控制处理函数
DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

// 全局变量
TCHAR g_szServiceName[] = TEXT("MyService");    // 服务名称
SERVICE_STATUS_HANDLE g_hServiceStatus;         // 服务状态句柄
SERVICE_STATUS g_serviceStatus = { 0 };         // 服务状态结构

int _tmain(int argc, TCHAR* argv[])
{
    const SERVICE_TABLE_ENTRY serviceTableEntry[] = { {g_szServiceName, ServiceMain}, {NULL, NULL} };

    // 将调用线程连接到SCM，从而使该线程成为服务控制调度线程
    StartServiceCtrlDispatcher(serviceTableEntry);

    return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // 注册一个服务控制处理函数HandlerEx，该函数返回一个服务状态句柄hServiceStatus
    g_hServiceStatus = RegisterServiceCtrlHandlerEx(g_szServiceName, HandlerEx, NULL);

    g_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
    g_serviceStatus.dwControlsAccepted = 0;
    SetServiceStatus(g_hServiceStatus, &g_serviceStatus);

    // 初始化工作
    Sleep(2000);

    g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
    g_serviceStatus.dwControlsAccepted =
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(g_hServiceStatus, &g_serviceStatus);

    // 执行服务任务，这里可以是你想执行的任何代码
    ShellExecute(NULL, TEXT("open"),
        TEXT("F:\\Source\\Windows\\Chapter6\\HelloWindows7\\Debug\\HelloWindows.exe"),
        NULL, NULL, SW_SHOW);
}

DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);

        // 可以做一些清理工作

        g_serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);
        break;

    case SERVICE_CONTROL_PAUSE:
        g_serviceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);
        g_serviceStatus.dwCurrentState = SERVICE_PAUSED;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);
        break;

    case SERVICE_CONTROL_CONTINUE:
        g_serviceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);
        g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(g_hServiceStatus, &g_serviceStatus);
        break;
    }

    return NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////
BOOL CreateUIProcess(LPCTSTR lpApplicationName, LPTSTR lpCommandLine)
{
    DWORD dwSessionId;                                  // 当前登录用户的Session ID
    HANDLE hUserToken = NULL, hUserTokenDup = NULL;     // 访问令牌句柄
    LPVOID lpEnvironment = NULL;
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // 获取当前登录用户的Session ID
    dwSessionId = WTSGetActiveConsoleSessionId();
    // 查询指定Session ID对应的登录用户的主访问令牌
    WTSQueryUserToken(dwSessionId, &hUserToken);

    // 复制一份用户访问令牌句柄
    DuplicateTokenEx(hUserToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
        TokenPrimary, &hUserTokenDup);
    CloseHandle(hUserToken);

    // 获取当前登录用户的环境变量块
    CreateEnvironmentBlock(&lpEnvironment, hUserTokenDup, FALSE);

    // 创建进程
    CreateProcessAsUser(hUserTokenDup, lpApplicationName, lpCommandLine, NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT, lpEnvironment, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hUserTokenDup);
    DestroyEnvironmentBlock(lpEnvironment);
    return TRUE;
}

BOOL CreateUIProcess2(LPCTSTR lpApplicationName, LPTSTR lpCommandLine)
{
    DWORD dwSessionId;                                      // 当前登录用户的Session ID
    HANDLE hProcessToken = NULL, hProcessTokenDup = NULL;   // 访问令牌句柄
    LPVOID lpEnvironment = NULL;
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // 获取与服务进程关联的访问令牌
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken);

    // 复制一份访问令牌句柄
    DuplicateTokenEx(hProcessToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
        TokenPrimary, &hProcessTokenDup);
    CloseHandle(hProcessToken);

    // 设置访问令牌句柄的Session ID为当前登录用户
    dwSessionId = WTSGetActiveConsoleSessionId();
    SetTokenInformation(hProcessTokenDup, TokenSessionId, &dwSessionId, sizeof(dwSessionId));

    // 获取当前登录用户的环境变量块
    CreateEnvironmentBlock(&lpEnvironment, hProcessTokenDup, FALSE);

    // 创建进程
    CreateProcessAsUser(hProcessTokenDup, lpApplicationName, lpCommandLine, NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT, lpEnvironment, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hProcessTokenDup);
    DestroyEnvironmentBlock(lpEnvironment);
    return TRUE;
}