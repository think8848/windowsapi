#include <windows.h>
#include "resource.h"

#define BUF_SIZE    16

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    HANDLE hFile1, hFile2;
    TCHAR szBuf[BUF_SIZE + 1] = { 0 };
    DWORD dwNumberOfBytesRead;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_TEXT);
        // 设置多行编辑控件的缓冲区大小为不限制
        SendMessage(hwndEdit, EM_SETLIMITTEXT, 0, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OPEN:
            hFile1 = CreateFile(TEXT("Test.txt"), GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            hFile2 = CreateFile(TEXT("Test2.txt"), GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile1 != INVALID_HANDLE_VALUE && hFile2 != INVALID_HANDLE_VALUE)
            {
                while (TRUE)
                {
                    // 从Test文件读取数据
                    ZeroMemory(szBuf, BUF_SIZE * sizeof(TCHAR));
                    ReadFile(hFile1, szBuf, BUF_SIZE * sizeof(TCHAR), &dwNumberOfBytesRead, NULL);
                    if (dwNumberOfBytesRead == 0)
                        break;

                    // 把读取到的数据显示到编辑控件中
                    SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                    SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                    // 把读取到的数据写入到新文件Test2.txt中
                    // 第3个参数不能写为BUF_SIZE * sizeof(TCHAR)，否则最后一次可能会出现问题
                    WriteFile(hFile2, szBuf, dwNumberOfBytesRead, NULL, NULL);
                }

                CloseHandle(hFile1);
                CloseHandle(hFile2);
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