#include <windows.h>
#include "HookZwQuerySystemInformation.h"
#include "resource.h"

#pragma comment(lib, "HookZwQuerySystemInformation.lib")

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dwProcessId;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INSTALLHOOK:
            dwProcessId = GetDlgItemInt(hwndDlg, IDC_EDIT_PROCESSID, NULL, FALSE);
            InstallHook(WH_GETMESSAGE, 0, dwProcessId);
            break;

        case IDC_BTN_UNINSTALLHOOK:
            UninstallHook();
            break;

        case IDCANCEL:
            UninstallHook();
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}