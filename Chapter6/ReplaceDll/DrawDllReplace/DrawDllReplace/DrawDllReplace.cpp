// 定义DLL的导出函数

#include <Windows.h>
#include <tchar.h>

#define DLL_EXPORT
#include "DrawDllReplace.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// 导出函数
VOID DrawRectangle(HWND hwnd)
{
    HDC hdc;

    hdc = GetDC(hwnd);
    Ellipse(hdc, 10, 10, 110, 110);
    ReleaseDC(hwnd, hdc);
}

VOID DrawEllipse(HWND hwnd)
{
    HDC hdc;

    hdc = GetDC(hwnd);
    Rectangle(hdc, 10, 10, 110, 110);
    ReleaseDC(hwnd, hdc);
}