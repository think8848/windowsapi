#include <windows.h>
#include <tchar.h>
#include "resource.h"

// 函数声明
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
    LPCTSTR lpDllPath = TEXT("..\\..\\FWMDll\\Release\\FWMDll_修改.dll");
    //LPCTSTR lpDllPath = TEXT("..\\..\\FWMDll2\\Release\\FWMDll.dll");
    //LPCTSTR lpDllPath = TEXT("..\\..\\..\\Detours\\InjectDll\\Release\\InjectDll.dll");
    LPTSTR lpDllPathRemote = NULL;
    HANDLE hThreadRemote = NULL;

    GetStartupInfo(&si);
    CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    // (1)	调用VirtualAllocEx函数在远程进程的地址空间中分配一块内存；
    int cbDllPath = (_tcslen(lpDllPath) + 1) * sizeof(TCHAR);
    lpDllPathRemote = (LPTSTR)VirtualAllocEx(pi.hProcess, NULL, cbDllPath, MEM_COMMIT, PAGE_READWRITE);
    if (!lpDllPathRemote)
        return FALSE;

    // (2)	调用WriteProcessMemory函数把要注入的dll的路径复制到第1步分配的内存中；
    if (!WriteProcessMemory(pi.hProcess, lpDllPathRemote, lpDllPath, cbDllPath, NULL))
        return FALSE;

    // (3)	调用GetProcAddress函数得到LoadLibraryA / LoadLibraryW函数(Kernel32.dll)的实际地址；
    PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
        GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (!pfnThreadRtn)
        return FALSE;

    // (4)	调用CreateRemoteThread函数在远程进程中创建一个线程
    hThreadRemote = CreateRemoteThread(pi.hProcess, NULL, 0, pfnThreadRtn, lpDllPathRemote, 0, NULL);
    if (!hThreadRemote)
        return FALSE;

    WaitForSingleObject(hThreadRemote, INFINITE);
    ResumeThread(pi.hThread);

    // (5)	调用VirtualFreeEx函数释放第1步分配的内存；
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