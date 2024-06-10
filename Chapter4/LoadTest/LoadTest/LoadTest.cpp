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
    TCHAR szCommandLine[MAX_PATH] = TEXT("Test.exe");
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    LPVOID lpBaseAddress = (LPVOID)0x009E1009;
    WORD wCodeOld, wCodeNew = 0x9090;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_LOADTEST:
            GetStartupInfo(&si);
            if (CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED,
                NULL, NULL, &si, &pi))
            {
                if (ReadProcessMemory(pi.hProcess, lpBaseAddress, &wCodeOld, sizeof(WORD), NULL))
                {
                    // 目标进程lpBaseAddress地址处的数据内容是否为0x1774，如果是则替换
                    if (wCodeOld == 0x1774)
                    {
                        // 改写机器码
                        WriteProcessMemory(pi.hProcess, lpBaseAddress, &wCodeNew,
                            sizeof(WORD), NULL);
                        ResumeThread(pi.hThread);
                    }
                    else
                    {
                        MessageBox(hwndDlg, TEXT("目标软件版本错误"), TEXT("错误提示"), MB_OK);
                        TerminateProcess(pi.hProcess, 0);
                    }
                }

                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
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