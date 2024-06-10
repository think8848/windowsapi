#include <windows.h>
#include "resource.h"

// 宏定义
#define THREADCOUNT 5

// 全局变量
__declspec(thread) LPVOID gt_lpData = (LPVOID)0x12345678;// 赋个初值是为了分析TLS表的时候方便查看
HWND g_hwndDlg;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 线程函数
DWORD WINAPI ThreadProc(LPVOID lpParameter);

// TLS回调函数
VOID NTAPI TlsCallback(PVOID DllHandle, DWORD Reason, PVOID Reserved);

// 注册TLS回调函数
#pragma data_seg(".CRT$XLB")
    PIMAGE_TLS_CALLBACK pTlsCallback = TlsCallback;
#pragma data_seg()

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
            // 创建THREADCOUNT个线程
            SetDlgItemText(g_hwndDlg, IDC_EDIT_TLSSLOTS, TEXT(""));
            for (int i = 0; i < THREADCOUNT; i++)
            {
                if ((hThread[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL)) != NULL)
                    CloseHandle(hThread[i]);
            }
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
    TCHAR szBuf[64] = { 0 };

    gt_lpData = new BYTE[256];
    ZeroMemory(gt_lpData, 256);

    // 每个线程的静态TLS数据显示到编辑控件中
    wsprintf(szBuf, TEXT("线程%d的gt_lpData值：0x%p\r\n"), (INT)lpParameter, gt_lpData);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_SETSEL, -1, -1);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    delete[]gt_lpData;
    return 0;
}

VOID NTAPI TlsCallback(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        // 启动了一个新进程(包括第一个线程)
        MessageBox(g_hwndDlg, TEXT("我是TLS回调函数"), TEXT("提示"), MB_OK);
        break;

    case DLL_PROCESS_DETACH:
        // 进程将要被终止(包括第一个线程)
    case DLL_THREAD_ATTACH:
        // 创建了一个新线程，创建所有线程时都会发送这个通知，除了第一个线程
    case DLL_THREAD_DETACH:
        // 线程将要被终止，终止所有线程时都会发送这个通知，除了第一个线程
        break;
    }
}