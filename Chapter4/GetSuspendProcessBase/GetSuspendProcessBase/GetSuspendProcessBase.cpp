#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szCommandLine[MAX_PATH] = TEXT("ThreeThousandYears.exe"); // 目标程序
    MEMORY_BASIC_INFORMATION mbi = { 0 };                           // VirtualQueryEx参数
    SIZE_T nBufSize;                                                // VirtualQueryEx返回值
    TCHAR szImageFile[MAX_PATH] = { 0 };                            // 目标程序完整路径
    TCHAR szBuf[MAX_PATH * 2] = { 0 };                              // 缓冲区

    // 方法1
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    CONTEXT context = { 0 };

    // 创建一个挂起的进程
    GetStartupInfo(&si);
    CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    // 获取目标进程主线程环境
    context.ContextFlags = CONTEXT_ALL;
    GetThreadContext(pi.hThread, &context);

    // context.Eax是程序入口点地址
    nBufSize = VirtualQueryEx(pi.hProcess, (LPVOID)context.Eax, &mbi, sizeof(mbi));
    if (nBufSize > 0)
    {
        GetMappedFileName(pi.hProcess, (LPVOID)context.Eax, szImageFile, _countof(szImageFile));
        wsprintf(szBuf, TEXT("%s 基地址：0x%p"), szImageFile, mbi.AllocationBase);
        MessageBox(NULL, szBuf, TEXT("提示"), MB_OK);
    }

    // 方法2
    SYSTEM_INFO systemInfo = { 0 };
    LPVOID lpMinAppAddress = NULL;

    // 获取进程地址空间的最小、最大内存地址，和页面大小
    GetSystemInfo(&systemInfo);
    lpMinAppAddress = systemInfo.lpMinimumApplicationAddress;

    // 从最小内存地址开始查找第一个具有MEM_IMAGE存储类型的页面(页面状态为已提交)
    while (lpMinAppAddress < systemInfo.lpMaximumApplicationAddress)
    {
        ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
        nBufSize = VirtualQueryEx(pi.hProcess, lpMinAppAddress, &mbi, sizeof(mbi));
        if (nBufSize == 0)
        {
            lpMinAppAddress = (LPBYTE)lpMinAppAddress + systemInfo.dwPageSize;
            continue;
        }

        switch (mbi.State)
        {
        case MEM_RESERVE:
        case MEM_FREE:
            lpMinAppAddress = (LPBYTE)(mbi.BaseAddress) + mbi.RegionSize;
            break;

        case MEM_COMMIT:
            if (mbi.Type == MEM_IMAGE)
            {
                GetMappedFileName(pi.hProcess, lpMinAppAddress, szImageFile, _countof(szImageFile));
                wsprintf(szBuf, TEXT("%s 基地址：0x%p"), szImageFile, mbi.AllocationBase);
                MessageBox(NULL, szBuf, TEXT("提示"), MB_OK);
                break;
            }

            lpMinAppAddress = (LPBYTE)(mbi.BaseAddress) + mbi.RegionSize;
            break;
        }

        // 找到了就退出循环
        if (mbi.Type == MEM_IMAGE)
            break;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}