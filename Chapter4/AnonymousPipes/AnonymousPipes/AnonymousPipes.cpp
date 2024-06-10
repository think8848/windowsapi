#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// 常量定义
#define BUF_SIZE    1024

// 全局变量
HWND g_hwndDlg;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 线程函数
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        // 初始化编辑控件
        SetDlgItemText(hwndDlg, IDC_EDIT_URL, TEXT("www.baidu.com"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_PING:
            // 创建线程
            if ((hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL)) != NULL)
                CloseHandle(hThread);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    // 创建匿名管道
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(sa) };
    sa.bInheritHandle = TRUE;
    CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);

    // 创建子进程，把子进程Ping的输出重定向到匿名管道的写入句柄
    STARTUPINFO si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;

    // 命令行参数拼接为：Ping www.baidu.com的形式
    TCHAR szCommandLine[MAX_PATH] = TEXT("Ping ");
    TCHAR szURL[256] = { 0 };
    GetDlgItemText(g_hwndDlg, IDC_EDIT_URL, szURL, _countof(szURL));
    StringCchCat(szCommandLine, _countof(szCommandLine), szURL);

    // 创建Ping子进程
    if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        CHAR szBuf[BUF_SIZE + 1] = { 0 };
        CHAR szOutput[BUF_SIZE * 8] = { 0 };
        DWORD dwNumOfBytesRead;

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        while (TRUE)
        {
            // 读取匿名管道的读取句柄
            ZeroMemory(szBuf, sizeof(szBuf));
            ReadFile(hReadPipe, szBuf, BUF_SIZE, &dwNumOfBytesRead, NULL);
            if (dwNumOfBytesRead == 0)
                break;

            // Ping控制台的输出是ANSI编码，因此使用StringCchCatA和SetDlgItemTextA
            // 把读取到的数据追加到szOutput缓冲区
            StringCchCatA(szOutput, _countof(szOutput), szBuf);
            // 显示到编辑控件中
            SetDlgItemTextA(g_hwndDlg, IDC_EDIT_CONTENT, szOutput);
        }
    }

    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    return 0;
}