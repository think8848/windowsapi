#pragma once

// ���������ĺ���

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

// ��������
DLL_API BOOL InstallHook(int idHook, DWORD dwThreadId);// �������ֱ��ǹ������ͺ���Դ�������߳�ID
DLL_API BOOL UninstallHook();