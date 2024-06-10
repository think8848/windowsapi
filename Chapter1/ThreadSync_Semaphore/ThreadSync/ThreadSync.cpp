#include <windows.h>
#include "resource.h"

// 常量定义
#define NUM 2

// 全局变量
int g_n;
HANDLE g_hSemaphore;

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
        // 创建信号量对象
        g_hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
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
            // 关闭信号量对象句柄
            CloseHandle(g_hSemaphore);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    for (int i = 1; i <= 1000000; i++)
    {
        // 等待信号量
        WaitForSingleObject(g_hSemaphore, INFINITE);
        g_n++;
        g_n--;
        // 释放信号量
        ReleaseSemaphore(g_hSemaphore, 1, NULL);
    }

    return 0;
}