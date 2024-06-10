#pragma once

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

// µ¼³öº¯Êý
DLL_API BOOL InstallHook(int idHook, DWORD dwThreadId, DWORD dwProcessId);
DLL_API BOOL UninstallHook();