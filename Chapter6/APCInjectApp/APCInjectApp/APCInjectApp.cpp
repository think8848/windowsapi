#include <windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParameter);
BOOL InjectDll(DWORD dwProcessId, LPTSTR lpDllPath);
BOOL EjectDll(DWORD dwProcessId, LPTSTR lpDllPath);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread = NULL;
    DWORD dwProcessId;
    TCHAR szDllPath[MAX_PATH] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        SetDlgItemText(hwndDlg, IDC_EDIT_PROCESSID, TEXT("���������ID"));
        SetDlgItemText(hwndDlg, IDC_EDIT_DLLPATH,
            TEXT("F:\\Source\\Windows\\Chapter16\\RemoteDll2\\Debug\\RemoteDll.dll"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INJECT:
            // �������߳���ɶ�Ŀ�������dll��ע��
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread)
                CloseHandle(hThread);
            break;

        case IDC_BTN_EJECT:
            dwProcessId = GetDlgItemInt(hwndDlg, IDC_EDIT_PROCESSID, NULL, FALSE);
            GetDlgItemText(hwndDlg, IDC_EDIT_DLLPATH, szDllPath, _countof(szDllPath));
            EjectDll(dwProcessId, szDllPath);
            break;

        case IDCANCEL:
            SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_EJECT, 0);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    DWORD dwProcessId;
    TCHAR szDllPath[MAX_PATH] = { 0 };

    dwProcessId = GetDlgItemInt(g_hwndDlg, IDC_EDIT_PROCESSID, NULL, FALSE);
    GetDlgItemText(g_hwndDlg, IDC_EDIT_DLLPATH, szDllPath, _countof(szDllPath));
    return InjectDll(dwProcessId, szDllPath);
}

BOOL InjectDll(DWORD dwProcessId, LPTSTR lpDllPath)
{
    HANDLE hProcess = NULL;
    LPTSTR lpDllPathRemote = NULL;

    hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (!hProcess)
        return FALSE;

    // (1)	����VirtualAllocEx������Զ�̽��̵ĵ�ַ�ռ��з���һ���ڴ棻
    SIZE_T cbDllPath = (_tcslen(lpDllPath) + 1) * sizeof(TCHAR);
    lpDllPathRemote = (LPTSTR)VirtualAllocEx(hProcess, NULL, cbDllPath,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!lpDllPathRemote)
        return FALSE;

    // (2)	����WriteProcessMemory������Ҫע���dll��·�����Ƶ���1��������ڴ��У�
    if (!WriteProcessMemory(hProcess, lpDllPathRemote, lpDllPath, cbDllPath, NULL))
        return FALSE;

    // (3)	����GetProcAddress�����õ�LoadLibraryA / LoadLibraryW����(Kernel32.dll)��ʵ�ʵ�ַ��
    HMODULE hModule = GetModuleHandle(TEXT("Kernel32"));
    PAPCFUNC pfnAPC = NULL;
    if (hModule != NULL)
    {
        pfnAPC = (PAPCFUNC)GetProcAddress(hModule, "LoadLibraryW");
        if (!pfnAPC)
            return FALSE;
    }

    // (4)	����Ŀ������е��߳�
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    THREADENTRY32 te = { sizeof(THREADENTRY32) };
    BOOL bRet = FALSE;
    HANDLE hThread = NULL;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    bRet = Thread32First(hSnapshot, &te);
    while (bRet)
    {
        if (te.th32OwnerProcessID == dwProcessId)
        {
            hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
            if (hThread)
            {
                QueueUserAPC(pfnAPC, hThread, (ULONG_PTR)lpDllPathRemote);

                // �ر��߳̾��
                CloseHandle(hThread);
                hThread = NULL;
            }
        }

        bRet = Thread32Next(hSnapshot, &te);
    }

    CloseHandle(hSnapshot);

    //// (5)	����VirtualFreeEx�����ͷŵ�1��������ڴ棻
    //if (!lpDllPathRemote)
    //    VirtualFreeEx(hProcess, lpDllPathRemote, 0, MEM_RELEASE);
    if (hProcess)
        CloseHandle(hProcess);

    return TRUE;
}

// �Զ��庯��EjectDll����֤ж�سɹ�
BOOL EjectDll(DWORD dwProcessId, LPTSTR lpDllPath)
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
    THREADENTRY32 te = { sizeof(THREADENTRY32) };
    BOOL bRet = FALSE;
    BOOL bFound = FALSE;

    // ����GetProcAddress�����õ�FreeLibrary����(Kernel32.dll)��ʵ�ʵ�ַ
    HMODULE hModule = GetModuleHandle(TEXT("Kernel32"));
    PAPCFUNC pfnAPC = NULL;
    if (hModule != NULL)
    {
        pfnAPC = (PAPCFUNC)GetProcAddress(hModule, "FreeLibrary");
        if (!pfnAPC)
            return FALSE;
    }

    // ��ȡĿ�������RemoteDll.dll��ģ����
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    bRet = Module32First(hSnapshot, &me);
    while (bRet)
    {
        if (_tcsicmp(TEXT("RemoteDll.dll"), me.szModule) == 0 ||
            _tcsicmp(lpDllPath, me.szExePath) == 0)
        {
            bFound = TRUE;
            break;
        }

        bRet = Module32Next(hSnapshot, &me);
    }

    CloseHandle(hSnapshot);
    if (!bFound)
        return FALSE;

    // ����Ŀ������е��߳�
    hSnapshot = INVALID_HANDLE_VALUE;
    bRet = FALSE;
    HANDLE hThread = NULL;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    bRet = Thread32First(hSnapshot, &te);
    while (bRet)
    {
        if (te.th32OwnerProcessID == dwProcessId)
        {
            hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
            if (hThread)
            {
                QueueUserAPC(pfnAPC, hThread, (ULONG_PTR)(me.modBaseAddr));

                // �ر��߳̾��
                CloseHandle(hThread);
                hThread = NULL;
            }
        }

        bRet = Thread32Next(hSnapshot, &te);
    }

    CloseHandle(hSnapshot);

    return TRUE;
}