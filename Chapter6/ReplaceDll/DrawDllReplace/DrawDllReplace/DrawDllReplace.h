#pragma once

// ���������ĺ���

#ifdef DLL_EXPORT
#define DLL_API       extern "C" __declspec(dllexport)
#else
#define DLL_API       extern "C" __declspec(dllimport)
#endif

// ��������
DLL_API VOID DrawRectangle(HWND hwnd);
DLL_API VOID DrawEllipse(HWND hwnd);