#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 自定义消息，用于计数线程向显示线程发送消息报告工作进度(这两个都是工作线程)
#define WM_WORKPROGRESS (WM_APP + 1)
// 自定义消息，计数线程发送消息给主线程告知工作已完成
#define WM_CALCOVER     (WM_APP + 2)

// 全局变量
HWND g_hwndDlg;
BOOL g_bRuning;         // 计数线程没有消息循环，主线程通过把这个标志设为FALSE通知其终止线程

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 线程函数声明
DWORD WINAPI ThreadProcShow(LPVOID lpParameter);    // 把数值显示到编辑控件中
DWORD WINAPI ThreadProcCalc(LPVOID lpParameter);    // 模拟执行一项任务，定时把一个数加1

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hThreadShow, hThreadCalc;
    static DWORD dwThreadIdShow;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        // 禁用停止按钮
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            g_bRuning = TRUE;
            // 创建显示线程和计数线程
            hThreadShow = CreateThread(NULL, 0, ThreadProcShow, NULL, 0, &dwThreadIdShow);
            hThreadCalc = CreateThread(NULL, 0, ThreadProcCalc, (LPVOID)dwThreadIdShow, 0, NULL);

            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), TRUE);
            break;

        case IDC_BTN_STOP:
            // 通知计数线程退出
            g_bRuning = FALSE;
            // 通知显示线程退出
            PostThreadMessage(dwThreadIdShow, WM_QUIT, 0, 0);

            if (hThreadShow != NULL)
            {
                CloseHandle(hThreadShow);
                hThreadShow = NULL;
            }
            if (hThreadCalc != NULL)
            {
                CloseHandle(hThreadCalc);
                hThreadCalc = NULL;
            }
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CALCOVER:
        if (hThreadShow != NULL)
        {
            CloseHandle(hThreadShow);
            hThreadShow = NULL;
        }
        if (hThreadCalc != NULL)
        {
            CloseHandle(hThreadCalc);
            hThreadCalc = NULL;
        }
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);

        MessageBox(hwndDlg, TEXT("计数线程工作已完成"), TEXT("提示"), MB_OK);
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProcShow(LPVOID lpParameter)
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        switch (msg.message)
        {
        case WM_WORKPROGRESS:
            SetDlgItemInt(g_hwndDlg, IDC_EDIT_COUNT, (UINT)msg.wParam, FALSE);
            break;
        }
    }

    return msg.wParam;
}

DWORD WINAPI ThreadProcCalc(LPVOID lpParameter)
{
    // lpParameter参数是传递过来的显示线程ID
    DWORD dwThreadIdShow = (DWORD)lpParameter;
    int nCount = 0;

    while (g_bRuning)
    {
        PostThreadMessage(dwThreadIdShow, WM_WORKPROGRESS, nCount++, NULL);
        Sleep(50);

        // nCount到达100，说明工作完成
        if (nCount > 100)
        {
            // 通知显示线程退出
            PostThreadMessage(dwThreadIdShow, WM_QUIT, 0, 0);

            // 发送消息给主线程告知工作已完成
            PostMessage(g_hwndDlg, WM_CALCOVER, 0, 0);

            // 本计数线程也退出
            g_bRuning = FALSE;
            break;
        }
    }

    return 0;
}