// 定义DLL的导出函数

#include <Windows.h>
#include <tchar.h>

#define DLL_EXPORT
#include "HookDll.h"

// 全局变量
HINSTANCE g_hMod;
HHOOK g_hHookKeyboard;
TCHAR g_szBuf[256] = { 0 };

#pragma data_seg("Shared")
    HWND  g_hwnd = NULL;
#pragma data_seg()

#pragma comment(linker, "/SECTION:Shared,RWS")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hMod = hModule;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// 导出函数
BOOL InstallHook(int idHook, DWORD dwThreadId, HWND hwnd)
{
    if (!g_hHookKeyboard)
    {
        g_hwnd = hwnd;

        g_hHookKeyboard = SetWindowsHookEx(idHook, KeyboardProc, g_hMod, dwThreadId);
        if (!g_hHookKeyboard)
            return FALSE;
    }

    return TRUE;
}

BOOL UninstallHook()
{
    if (g_hHookKeyboard)
    {
        if (!UnhookWindowsHookEx(g_hHookKeyboard))
            return FALSE;
    }

    g_hHookKeyboard = NULL;
    return TRUE;
}

// 内部函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    BYTE bKeyState[256];
    COPYDATASTRUCT copyDataStruct = { 0 };

    if (nCode < 0)
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        GetKeyboardState(bKeyState);
        bKeyState[VK_SHIFT] = HIBYTE(GetKeyState(VK_SHIFT));
        ZeroMemory(g_szBuf, sizeof(g_szBuf));
        ToUnicode(wParam, lParam >> 16, bKeyState, g_szBuf, _countof(g_szBuf), 0);
        copyDataStruct.cbData = sizeof(g_szBuf);
        copyDataStruct.lpData = g_szBuf;
        SendMessage(g_hwnd, WM_COPYDATA, (WPARAM)g_hwnd, (LPARAM)&copyDataStruct);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}