#include <windows.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;
HANDLE g_hEvent;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc1(LPVOID lpParameter);
DWORD WINAPI ThreadProc2(LPVOID lpParameter);
DWORD WINAPI ThreadProc3(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread[3] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // 创建事件对象
        g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CREATETHREAD:
            // 重置事件对象
            ResetEvent(g_hEvent);

            hThread[0] = CreateThread(NULL, 0, ThreadProc1, NULL, 0, NULL);
            hThread[1] = CreateThread(NULL, 0, ThreadProc2, NULL, 0, NULL);
            hThread[2] = CreateThread(NULL, 0, ThreadProc3, NULL, 0, NULL);
            for (int i = 0; i < 3; i++)
            {
                if (hThread[i] != NULL)
                    CloseHandle(hThread[i]);
            }

            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), TRUE);
            break;

        case IDC_BTN_SETEVENT:
            // 设置事件对象
            SetEvent(g_hEvent);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), FALSE);
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

DWORD WINAPI ThreadProc1(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("线程1成功等待到事件对象"), TEXT("提示"), MB_OK);
    // 做一些工作

    //SetEvent(g_hEvent);
    return 0;
}

DWORD WINAPI ThreadProc2(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("线程2成功等待到事件对象"), TEXT("提示"), MB_OK);
    // 做一些工作

    //SetEvent(g_hEvent);
    return 0;
}

DWORD WINAPI ThreadProc3(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("线程3成功等待到事件对象"), TEXT("提示"), MB_OK);
    // 做一些工作

    //SetEvent(g_hEvent);
    return 0;
}