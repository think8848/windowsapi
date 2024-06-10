#include <Windows.h>
#include <tchar.h>
#include <wtsapi32.h>
#include <Userenv.h>

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")

// ������ڵ㺯��
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
// ������ƴ�����
DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

// ȫ�ֱ���
TCHAR g_szServiceName[] = TEXT("MyService");    // ��������
SERVICE_STATUS_HANDLE g_hServiceStatus;         // ����״̬���
SERVICE_STATUS g_serviceStatus = { 0 };         // ����״̬�ṹ

int _tmain(int argc, TCHAR* argv[])
{
    const SERVICE_TABLE_ENTRY serviceTableEntry[] = { {g_szServiceName, ServiceMain}, {NULL, NULL} };

    // �������߳����ӵ�SCM���Ӷ�ʹ���̳߳�Ϊ������Ƶ����߳�
    StartServiceCtrlDispatcher(serviceTableEntry);

    return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // ע��һ��������ƴ�����HandlerEx���ú�������һ������״̬���hServiceStatus
    g_hServiceStatus = RegisterServiceCtrlHandlerEx(g_szServiceName, HandlerEx, NULL);

    g_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
    g_serviceStatus.dwControlsAccepted = 0;
    SetServiceStatus(g_hServiceStatus, &g_serviceStatus);

    // ��ʼ������
    Sleep(2000);

    g_serviceStatus.dwCurrentState = SERVICE_RUNNING;
    g_serviceStatus.dwControlsAccepted =
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(g_hServiceStatus, &g_serviceStatus);

    // ִ�з��������������������ִ�е��κδ���
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

        // ������һЩ������

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
    DWORD dwSessionId;                                  // ��ǰ��¼�û���Session ID
    HANDLE hUserToken = NULL, hUserTokenDup = NULL;     // �������ƾ��
    LPVOID lpEnvironment = NULL;
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // ��ȡ��ǰ��¼�û���Session ID
    dwSessionId = WTSGetActiveConsoleSessionId();
    // ��ѯָ��Session ID��Ӧ�ĵ�¼�û�������������
    WTSQueryUserToken(dwSessionId, &hUserToken);

    // ����һ���û��������ƾ��
    DuplicateTokenEx(hUserToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
        TokenPrimary, &hUserTokenDup);
    CloseHandle(hUserToken);

    // ��ȡ��ǰ��¼�û��Ļ���������
    CreateEnvironmentBlock(&lpEnvironment, hUserTokenDup, FALSE);

    // ��������
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
    DWORD dwSessionId;                                      // ��ǰ��¼�û���Session ID
    HANDLE hProcessToken = NULL, hProcessTokenDup = NULL;   // �������ƾ��
    LPVOID lpEnvironment = NULL;
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // ��ȡ�������̹����ķ�������
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken);

    // ����һ�ݷ������ƾ��
    DuplicateTokenEx(hProcessToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
        TokenPrimary, &hProcessTokenDup);
    CloseHandle(hProcessToken);

    // ���÷������ƾ����Session IDΪ��ǰ��¼�û�
    dwSessionId = WTSGetActiveConsoleSessionId();
    SetTokenInformation(hProcessTokenDup, TokenSessionId, &dwSessionId, sizeof(dwSessionId));

    // ��ȡ��ǰ��¼�û��Ļ���������
    CreateEnvironmentBlock(&lpEnvironment, hProcessTokenDup, FALSE);

    // ��������
    CreateProcessAsUser(hProcessTokenDup, lpApplicationName, lpCommandLine, NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT, lpEnvironment, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hProcessTokenDup);
    DestroyEnvironmentBlock(lpEnvironment);
    return TRUE;
}