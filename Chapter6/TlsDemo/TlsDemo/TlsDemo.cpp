#include <windows.h>
#include "resource.h"

// 宏定义
#define THREADCOUNT 5

// 全局变量
DWORD g_dwTlsIndex;
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
    HANDLE hThread[THREADCOUNT] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            // 从进程中分配一个TLS索引
            g_dwTlsIndex = TlsAlloc();
            if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
            {
                MessageBox(hwndDlg, TEXT("TlsAlloc函数调用失败！"), TEXT("错误提示"), MB_OK);
                return FALSE;
            }

            // 创建THREADCOUNT个线程
            SetDlgItemText(g_hwndDlg, IDC_EDIT_TLSSLOTS, TEXT(""));
            for (int i = 0; i < THREADCOUNT; i++)
            {
                if ((hThread[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL)) != NULL)
                    CloseHandle(hThread[i]);
            }

            // 等待所有线程结束，释放TLS索引
            WaitForMultipleObjects(THREADCOUNT, hThread, TRUE, INFINITE);
            TlsFree(g_dwTlsIndex);
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
    LPVOID lpData = NULL;
    TCHAR szBuf[64] = { 0 };

    lpData = new BYTE[256];
    ZeroMemory(lpData, 256);

    // 在自己的存储槽中指定索引处写入数据
    if (!TlsSetValue(g_dwTlsIndex, lpData))
    {
        wsprintf(szBuf, TEXT("线程%d调用TlsSetValue失败"), (INT)lpParameter);
        MessageBox(g_hwndDlg, szBuf, TEXT("错误提示"), MB_OK);
        delete[]lpData;
        return 0;
    }

    // 获取自己的存储槽中指定索引处的数据
    lpData = TlsGetValue(g_dwTlsIndex);
    if (!lpData && GetLastError() != ERROR_SUCCESS)
    {
        wsprintf(szBuf, TEXT("线程%d调用TlsGetValue失败"), (INT)lpParameter);
        MessageBox(g_hwndDlg, szBuf, TEXT("错误提示"), MB_OK);
    }
    // 每个线程存储槽中指定索引处的数据显示到编辑控件中
    wsprintf(szBuf, TEXT("线程%d的索引%d处的值：0x%p\r\n"), (INT)lpParameter, g_dwTlsIndex, lpData);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_SETSEL, -1, -1);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    delete[]lpData;
    return 0;
}