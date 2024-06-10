#include <Windows.h>

#define DLL_EXPORT
#include "DllTest.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL, TEXT("正在执行DllMain入口点函数"), TEXT("提示"), MB_OK);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// 导出函数
VOID ShowMessage()
{
    MessageBox(NULL, TEXT("我是导出函数"), TEXT("提示"), MB_OK);
}