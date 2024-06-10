#include <windows.h>
#include "resource.h"

// 常量定义
#define NUM 2

// 全局变量
int g_n;
HANDLE g_hEvent;

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
        // 创建一个自动重置匿名事件对象
        g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            // 设置事件对象为有信号
            SetEvent(g_hEvent);

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
            // 关闭事件对象句柄
            CloseHandle(g_hEvent);
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
        // 等待事件对象
        WaitForSingleObject(g_hEvent, INFINITE);
        g_n++;
        g_n--;
        // 设置事件对象为有信号
        SetEvent(g_hEvent);
    }

    return 0;
}