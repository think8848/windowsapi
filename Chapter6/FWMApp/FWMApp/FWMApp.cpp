#include <windows.h>
#include <tchar.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL InjectDll();
BOOL EjectDll();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INJECT:
            InjectDll();
            break;

        case IDC_BTN_EJECT:
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL InjectDll()
{
    TCHAR szCommandLine[MAX_PATH] = TEXT("FloatingWaterMark.exe");
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    //LPCTSTR lpDllPath = TEXT("..\\..\\FWMDll\\Release\\FWMDll.dll");
    LPCTSTR lpDllPath = TEXT("..\\..\\FWMDll\\Release\\FWMDll_�޸�.dll");
    //LPCTSTR lpDllPath = TEXT("..\\..\\FWMDll2\\Release\\FWMDll.dll");
    //LPCTSTR lpDllPath = TEXT("..\\..\\..\\Detours\\InjectDll\\Release\\InjectDll.dll");
    LPTSTR lpDllPathRemote = NULL;
    HANDLE hThreadRemote = NULL;

    GetStartupInfo(&si);
    CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    // (1)	����VirtualAllocEx������Զ�̽��̵ĵ�ַ�ռ��з���һ���ڴ棻
    int cbDllPath = (_tcslen(lpDllPath) + 1) * sizeof(TCHAR);
    lpDllPathRemote = (LPTSTR)VirtualAllocEx(pi.hProcess, NULL, cbDllPath, MEM_COMMIT, PAGE_READWRITE);
    if (!lpDllPathRemote)
        return FALSE;

    // (2)	����WriteProcessMemory������Ҫע���dll��·�����Ƶ���1��������ڴ��У�
    if (!WriteProcessMemory(pi.hProcess, lpDllPathRemote, lpDllPath, cbDllPath, NULL))
        return FALSE;

    // (3)	����GetProcAddress�����õ�LoadLibraryA / LoadLibraryW����(Kernel32.dll)��ʵ�ʵ�ַ��
    PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
        GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (!pfnThreadRtn)
        return FALSE;

    // (4)	����CreateRemoteThread������Զ�̽����д���һ���߳�
    hThreadRemote = CreateRemoteThread(pi.hProcess, NULL, 0, pfnThreadRtn, lpDllPathRemote, 0, NULL);
    if (!hThreadRemote)
        return FALSE;

    WaitForSingleObject(hThreadRemote, INFINITE);
    ResumeThread(pi.hThread);

    // (5)	����VirtualFreeEx�����ͷŵ�1��������ڴ棻
    if (!lpDllPathRemote)
        VirtualFreeEx(pi.hProcess, lpDllPathRemote, 0, MEM_RELEASE);
    if (!pi.hThread)
        CloseHandle(pi.hThread);
    if (!pi.hProcess)
        CloseHandle(pi.hProcess);

    return TRUE;
}

BOOL EjectDll()
{
    return TRUE;
}