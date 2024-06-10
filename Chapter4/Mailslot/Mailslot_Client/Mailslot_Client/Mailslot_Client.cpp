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
    HANDLE hMailslot;
    TCHAR szBuf[1024] = { 0 };
    int n;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_SEND:
            // 打开邮槽
            hMailslot = CreateFile(szMailslotName, GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (hMailslot != INVALID_HANDLE_VALUE)
            {
                // 写入数据
                n = GetDlgItemText(hwndDlg, IDC_EDIT_MSG, szBuf, _countof(szBuf));
                WriteFile(hMailslot, szBuf, (n + 1) * sizeof(TCHAR), NULL, NULL);

                CloseHandle(hMailslot);
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