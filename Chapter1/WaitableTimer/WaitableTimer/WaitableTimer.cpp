#include <windows.h>
#include <Commctrl.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HWND g_hwndDlg;
HANDLE g_hTimer;

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
    SYSTEMTIME st = { 0 };
    FILETIME ftLocal, ftUTC;
    LARGE_INTEGER li;
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // 创建一个自动重置可等待计时器对象
        g_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

        hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
        if (hThread != NULL)
            CloseHandle(hThread);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_SET:
        case IDC_BTN_RESET:
           // 自定义一个时间
            /*st.wYear = 2019;
            st.wMonth = 8;
            st.wDay = 5;
            st.wHour = 17;
            st.wMinute = 45;
            st.wSecond = 0;
            st.wMilliseconds = 0;*/

            // 获取日期时间控件的时间
            SendDlgItemMessage(hwndDlg, IDC_DATETIMEPICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
            // 系统时间转换成FILETIME时间
            SystemTimeToFileTime(&st, &ftLocal);
            // 本地FILETIME时间转换成UTC的FILETIME时间
            LocalFileTimeToFileTime(&ftLocal, &ftUTC);
            // 不要将指向FILETIME结构的指针强制转换为LARGE_INTEGER *或__int64 *类型，
            li.LowPart = ftUTC.dwLowDateTime;
            li.HighPart = ftUTC.dwHighDateTime;
            // 设置可等待计时器
            SetWaitableTimer(g_hTimer, &li, 10 * 1000, NULL, NULL, FALSE);
            break;

        case IDC_BTN_CANCEL:
            // 取消可等待计时器
            CancelWaitableTimer(g_hTimer);
            break;

        case IDCANCEL:
            // 关闭可等待计时器对象句柄
            CloseHandle(g_hTimer);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    while (TRUE)
    {
        // 等待可等待计时器
        WaitForSingleObject(g_hTimer, INFINITE);
        ShellExecute(NULL, TEXT("open"), TEXT("Calc.exe"), NULL, NULL, SW_SHOW);
    }

    return 0;
}