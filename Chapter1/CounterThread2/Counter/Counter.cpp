#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HWND g_hwndDlg;
HANDLE g_hEventStart;       // 事件对象句柄，作为开始标志
HANDLE g_hEventStop;        // 事件对象句柄，作为停止标志

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
    static HWND hwndBtnStart, hwndBtnStop, hwndBtnPause, hwndBtnContinue;
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        hwndBtnStart = GetDlgItem(hwndDlg, IDC_BTN_START);
        hwndBtnStop = GetDlgItem(hwndDlg, IDC_BTN_STOP);
        hwndBtnPause = GetDlgItem(hwndDlg, IDC_BTN_PAUSE);
        hwndBtnContinue = GetDlgItem(hwndDlg, IDC_BTN_CONTINUE);

        // 禁用停止、暂停、继续按钮
        EnableWindow(hwndBtnStop, FALSE);
        EnableWindow(hwndBtnPause, FALSE);
        EnableWindow(hwndBtnContinue, FALSE);

        // 创建事件对象
        g_hEventStart = CreateEvent(NULL, TRUE, FALSE, NULL);
        g_hEventStop = CreateEvent(NULL, TRUE, FALSE, NULL);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread != NULL)
            {
                CloseHandle(hThread);
                hThread = NULL;
            }

            SetEvent(g_hEventStart);    // 设置开始标志
            ResetEvent(g_hEventStop);   // 清除停止标志

            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            break;

        case IDC_BTN_STOP:
            SetEvent(g_hEventStop);     // 设置停止标志
            EnableWindow(hwndBtnStart, TRUE);
            EnableWindow(hwndBtnStop, FALSE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDC_BTN_PAUSE:
            ResetEvent(g_hEventStart);  // 清除开始标志
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, TRUE);
            break;

        case IDC_BTN_CONTINUE:
            SetEvent(g_hEventStart);    // 设置开始标志
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDCANCEL:
            // 关闭事件对象句柄
            CloseHandle(g_hEventStart);
            CloseHandle(g_hEventStop);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int n = 0;

    while (WaitForSingleObject(g_hEventStop, 0) != WAIT_OBJECT_0)       // 是否设置了停止标志
    {
        if (WaitForSingleObject(g_hEventStart, 100) == WAIT_OBJECT_0)   // 是否设置了开始标志
            SetDlgItemInt(g_hwndDlg, IDC_EDIT_COUNT, n++, FALSE);
    }

    return 0;
}