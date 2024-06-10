#include <windows.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CreateProcessAndInjectDll();

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
        case IDC_BTN_CREATE:
            CreateProcessAndInjectDll();
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL CreateProcessAndInjectDll()
{
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    TCHAR szExePath[MAX_PATH] = TEXT("ThreeThousandYears.exe");
    TCHAR szDllPath[MAX_PATH] = TEXT("MessageBoxDll.dll");
    BOOL bRet;

    // 29字节的机器指令和MAX_PATH * sizeof(TCHAR)字节的要注入的dll的名称
    BYTE ShellCode[29 + MAX_PATH * sizeof(TCHAR)] =
    {
        0x60,                           // pushad
        0x9C,                           // pushfd
        0x68,0xAA,0xBB,0xCC,0xDD,       // push [0xDDCCBBAA](0xDDCCBBAA是目标进程中要注入的dll的名称)
        0xFF,0x15,0xDD,0xCC,0xBB,0xAA,  // call [0xDDCCBBAA](0xDDCCBBAA是LoadLibraryW函数的地址)
        0x9D,                           // popfd
        0x61,                           // popad
        0xFF,0x25,0xAA,0xBB,0xCC,0xDD,  // jmp [0xDDCCBBAA](0xDDCCBBAA为目标进程原入口点)
        0xAA,0xAA,0xAA,0xAA,            // 保存loadlibraryW函数地址的4字节数据区域
        0xAA,0xAA,0xAA,0xAA,            // 保存目标进程原入口点地址的4字节数据区域
        0,                              // 往后开始就是存放要注入的dll名称的数据区域
    };

    // 以挂起模式创建一个进程
    bRet = CreateProcess(szExePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!bRet)
        return FALSE;

    // 获取目标进程主线程环境(EIP)
    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(pi.hThread, &context))
        return FALSE;

    // 获得LoadLibraryW函数的地址
    DWORD dwLoadLibraryWAddr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), 
        "LoadLibraryW");
    if (!dwLoadLibraryWAddr)
        return FALSE;

    // 在目标进程中分配内存，存放ShellCode
    LPVOID lpMemoryRemote = VirtualAllocEx(pi.hProcess, NULL, 29 + MAX_PATH * sizeof(TCHAR),
        MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!lpMemoryRemote)
        return FALSE;

    // push [0xDDCCBBAA](0xDDCCBBAA是目标进程中要注入的dll的名称) 偏移ShellCode + 3
    *(DWORD*)(ShellCode + 3) = (DWORD)lpMemoryRemote + 29;

    // call [0xDDCCBBAA](0xDDCCBBAA是LoadLibraryW函数的地址)      偏移ShellCode + 9
    *(DWORD*)(ShellCode + 9) = (DWORD)lpMemoryRemote + 21;

    // jmp [0xDDCCBBAA](0xDDCCBBAA为目标进程原入口点)             偏移ShellCode + 17
    *(DWORD*)(ShellCode + 17) = (DWORD)lpMemoryRemote + 25;

    // 保存loadlibraryW函数地址的4字节数据区域                    偏移ShellCode + 21
    *(DWORD*)(ShellCode + 21) = dwLoadLibraryWAddr;

    // 保存目标进程原入口点地址的4字节数据区域                    偏移ShellCode + 25
    *(DWORD*)(ShellCode + 25) = context.Eip;

    // 往后开始就是存放要注入的dll名称的数据区域                  偏移ShellCode + 29
    memcpy_s(ShellCode + 29, MAX_PATH * sizeof(TCHAR), szDllPath, sizeof(szDllPath));

    // 把shellcode写入目标进程
    if (!WriteProcessMemory(pi.hProcess, lpMemoryRemote, ShellCode, 
        29 + MAX_PATH * sizeof(TCHAR), NULL))
        return FALSE;

    // 修改目标进程的EIP，执行被注入的代码
    context.Eip = (DWORD)lpMemoryRemote;
    if (!SetThreadContext(pi.hThread, &context))
        return FALSE;

    // 恢复目标进程的执行
    ResumeThread(pi.hThread);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}