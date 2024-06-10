#include <windows.h>
#include "resource.h"

// 常量定义
#define NUM 2

// 全局变量
int g_n;
CRITICAL_SECTION g_cs;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread[NUM] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // 初始化关键段对象CRITICAL_SECTION结构
        InitializeCriticalSection(&g_cs);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), FALSE);
            g_n = 10;       // 创建线程执行线程函数以前把全局变量g_n赋值为10
            for (int i = 0; i < NUM; i++)
                hThread[i] = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

            WaitForMultipleObjects(NUM, hThread, TRUE, INFINITE);
            for (int i = 0; i < NUM; i++)
            {
                if (hThread[i] != NULL)
                    CloseHandle(hThread[i]);
            }

            // 所有线程结束以后，把g_n的最终值显示在编辑控件中
            SetDlgItemInt(hwndDlg, IDC_EDIT_NUM, g_n, TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
            break;

        case IDCANCEL:
            // 释放关键段对象
            DeleteCriticalSection(&g_cs);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    for (int i = 1; i <= 100000000; i++)
    {
        // 进入关键段
        EnterCriticalSection(&g_cs);
        g_n++;
        g_n--;
        // 离开关键段
        LeaveCriticalSection(&g_cs);
    }

    return 0;
}