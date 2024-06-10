#pragma once

// 声明导出的函数

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

// 导出函数
DLL_API BOOL InstallHook(int idHook, DWORD dwThreadId, HWND hwnd);
DLL_API BOOL UninstallHook();

// 内部函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);