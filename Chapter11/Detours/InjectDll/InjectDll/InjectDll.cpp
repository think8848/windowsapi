#include <windows.h>
#include <tchar.h>
#include "..\..\..\Detours-master\include\detours.h"

// 编译为x86时需要使用的.lib
#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X86\\detours.lib")
// 编译为x64时需要使用的.lib
//#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X64\\detours.lib")

// 目标函数指针(加static关键字说明仅用于本文件)
static BOOL(WINAPI* OriginalExtTextOutW)(HDC hdc, int x, int y, UINT options,
    const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx) = ExtTextOutW;

// 自定义函数
BOOL WINAPI DetourExtTextOutW(HDC hdc, int x, int y, UINT options,
    RECT* lprect, LPCWSTR lpString, UINT c, INT* lpDx)
{
    TCHAR szText1[] = TEXT("屏幕");
    TCHAR szText2[] = TEXT("用户名");
    TCHAR szText3[] = TEXT("购买者");
    TCHAR szTextReplace[] = TEXT("                                                          ");
    LPCTSTR lpStr;

    if ((lpStr = _tcsstr(lpString, szText1)) ||
        (lpStr = _tcsstr(lpString, szText2)) ||
        (lpStr = _tcsstr(lpString, szText3)))
    {
        memcpy((LPVOID)lpStr, szTextReplace, _tcslen(lpStr) * sizeof(TCHAR));
    }

    OriginalExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    // 如果当前进程是辅助进程则不执行任何处理
    if (DetourIsHelperProcess())
        return TRUE;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        // 恢复当前进程的导入表
        DetourRestoreAfterWith();

        // 开启(开始)事务
        DetourTransactionBegin();
        // 指定更新线程
        DetourUpdateThread(GetCurrentThread());
        // 执行Hook处理
        DetourAttach(&(PVOID&)OriginalExtTextOutW, DetourExtTextOutW);
        // 提交事务
        DetourTransactionCommit();
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        // 执行Unhook处理
        DetourDetach(&(PVOID&)OriginalExtTextOutW, DetourExtTextOutW);
        DetourTransactionCommit();
    }

    return TRUE;
}