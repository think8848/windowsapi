#pragma once

// 声明导出的函数

#ifdef DLL_EXPORT
#define DLL_API       extern "C" __declspec(dllexport)
#else
#define DLL_API       extern "C" __declspec(dllimport)
#endif

// 导出函数
DLL_API VOID DrawRectangle(HWND hwnd);
DLL_API VOID DrawEllipse(HWND hwnd);