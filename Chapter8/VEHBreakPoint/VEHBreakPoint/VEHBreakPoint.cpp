#include <Windows.h>

// 全局变量
LPVOID g_pfnLoadLibraryExWAddress;  // LoadLibraryExW函数地址
BYTE g_bOriginalCodeByte;           // 保存LoadLibraryExW函数的第一个指令码
HWND g_hwndDlg;                     // CreateProcessInjectDll程序窗口句柄

// 函数声明
// 设置int 3断点(返回原指令码)
BYTE SetBreakPoint(LPVOID lpCodeAddr);
// 移除int 3断点
VOID RemoveBreakPoint(LPVOID lpCodeAddr, BYTE bOriginalCodeByte);

// 为LoadLibraryExW函数的int 3断点注册一个向量化异常处理程序
LONG CALLBACK LoadLibraryExWBPHandler(PEXCEPTION_POINTERS ExceptionInfo);

// LoadLibraryExW函数int 3中断以后执行用户所需的自定义操作
VOID LoadLibraryExWCustomActions(LPVOID lpCodeAddr, LPVOID lpStackAddr);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hwndDlg = FindWindow(TEXT("#32770"), TEXT("CreateProcessInjectDll"));

        // 获取KernelBase.LoadLibraryExW函数的地址
        g_pfnLoadLibraryExWAddress = (LPVOID)GetProcAddress(
            GetModuleHandle(TEXT("KernelBase.dll")), "LoadLibraryExW");

        // 为LoadLibraryExW函数的int 3断点注册一个向量化异常处理程序
        AddVectoredExceptionHandler(1, LoadLibraryExWBPHandler);
        // 在LoadLibraryExW函数上设置一个int 3断点
        g_bOriginalCodeByte = SetBreakPoint(g_pfnLoadLibraryExWAddress);
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// 内部函数
LONG CALLBACK LoadLibraryExWBPHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    DWORD dwExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

    if (dwExceptionCode == EXCEPTION_BREAKPOINT)
    {
        // 检查是否是我们设置的int 3断点，如果不是，将它传递给其它异常处理程序
        if (ExceptionInfo->ExceptionRecord->ExceptionAddress != g_pfnLoadLibraryExWAddress)
            return EXCEPTION_CONTINUE_SEARCH;

        // 对LoadLibraryExW函数执行用户所需的自定义操作
        LoadLibraryExWCustomActions(ExceptionInfo->ExceptionRecord->ExceptionAddress,
            (LPVOID)(ExceptionInfo->ContextRecord->Esp));

        // 临时移除int 3断点
        RemoveBreakPoint(g_pfnLoadLibraryExWAddress, g_bOriginalCodeByte);
        // 设置单步中断
        ExceptionInfo->ContextRecord->EFlags |= 0x100;

        // 重新执行发生int 3异常的指令，因为设置了单步中断，接下来会单步执行完第一条指令
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else if (dwExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        if (ExceptionInfo->ExceptionRecord->ExceptionAddress !=
            (LPBYTE)g_pfnLoadLibraryExWAddress + 2)
            return EXCEPTION_CONTINUE_SEARCH;

        // 已经执行完用户的自定义操作，也已经单步执行完LoadLibraryExW函数的第一条语句，
        // 重新设置int 3断点，以等待下一次LoadLibraryExW函数调用
        SetBreakPoint(g_pfnLoadLibraryExWAddress);

        // 继续运行
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    // 非int 3断点和单步中断都不处理
    return EXCEPTION_CONTINUE_SEARCH;
}

BYTE SetBreakPoint(LPVOID lpCodeAddr)
{
    BYTE bOriginalCodeByte;
    BYTE bInt3 = 0xCC;
    DWORD dwOldProtect;

    // 读取LoadLibraryExW函数的第一个指令码
    ReadProcessMemory(GetCurrentProcess(), lpCodeAddr, &bOriginalCodeByte,
        sizeof(bOriginalCodeByte), NULL);

    // 设置int 3断点
    VirtualProtect(lpCodeAddr, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    WriteProcessMemory(GetCurrentProcess(), lpCodeAddr, &bInt3, sizeof(bInt3), NULL);
    VirtualProtect(lpCodeAddr, 1, dwOldProtect, &dwOldProtect);

    return bOriginalCodeByte;
}

VOID RemoveBreakPoint(LPVOID lpCodeAddr, BYTE bOriginalCodeByte)
{
    DWORD dwOldProtect;

    VirtualProtect(lpCodeAddr, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    WriteProcessMemory(GetCurrentProcess(), lpCodeAddr, &bOriginalCodeByte,
        sizeof(bOriginalCodeByte), NULL);
    VirtualProtect(lpCodeAddr, 1, dwOldProtect, &dwOldProtect);
}

VOID LoadLibraryExWCustomActions(LPVOID lpCodeAddr, LPVOID lpStackAddr)
{
    TCHAR szDllName[MAX_PATH] = { 0 };

    ReadProcessMemory(GetCurrentProcess(), (LPVOID)(*(LPDWORD)((LPBYTE)lpStackAddr + 4)),
        szDllName, sizeof(szDllName), NULL);

    // dll名称显示到CreateProcessInjectDll程序的编辑控件中
    SendDlgItemMessage(g_hwndDlg, 1002, EM_SETSEL, -1, -1);
    SendDlgItemMessage(g_hwndDlg, 1002, EM_REPLACESEL, TRUE, (LPARAM)szDllName);
    SendDlgItemMessage(g_hwndDlg, 1002, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
}