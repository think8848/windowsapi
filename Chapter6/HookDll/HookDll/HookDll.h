#pragma once

// ���������ĺ���

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

// ��������
DLL_API BOOL InstallHook(int idHook, DWORD dwThreadId, HWND hwnd);
DLL_API BOOL UninstallHook();

// �ڲ�����
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);