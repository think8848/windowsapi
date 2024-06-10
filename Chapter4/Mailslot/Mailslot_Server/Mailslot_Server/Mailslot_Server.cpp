#include <windows.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szMailslotName[] = TEXT("\\\\.\\mailslot\\99F1755D-31FD-4CE5-8183-3F438316E8D7");
    static HANDLE hMailslot;
    DWORD dwNextSize, dwMessageCount, dwNumOfBytesRead;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // 创建邮件槽
        hMailslot = CreateMailslot(szMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
        if (hMailslot == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
                MessageBox(hwndDlg, TEXT("指定名称的邮件槽已经存在"), TEXT("错误提示"), MB_OK);
            else
                MessageBox(hwndDlg, TEXT("CreateMailslot函数调用失败"), TEXT("错误提示"), MB_OK);

            ExitProcess(0);
        }

        // 创建计时器
        SetTimer(hwndDlg, 1, 1000, NULL);
        return TRUE;

    case WM_TIMER:
        // 在调用ReadFile函数读取数据以前，先调用GetMailslotInfo函数来确定邮件槽中是否有消息
        GetMailslotInfo(hMailslot, NULL, &dwNextSize, &dwMessageCount, NULL);
        for (DWORD i = 0; i < dwMessageCount; i++)
        {
            LPBYTE lpBuf = new BYTE[dwNextSize];
            ZeroMemory(lpBuf, dwNextSize);
            ReadFile(hMailslot, lpBuf, dwNextSize, &dwNumOfBytesRead, NULL);
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_MSG), LB_ADDSTRING, 0, (LPARAM)(LPTSTR)lpBuf);
            delete[]lpBuf;
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            CloseHandle(hMailslot);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}