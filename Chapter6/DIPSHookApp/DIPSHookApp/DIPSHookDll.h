#pragma once

// 声明导出的函数

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

// 导出函数
DLL_API BOOL InstallHook(int idHook, DWORD dwThreadId);// 两参数分别是钩子类型和资源管理器线程ID
DLL_API BOOL UninstallHook();