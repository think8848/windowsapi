#include <windows.h>
#include "HookDll.h"
#include "resource.h"

#pragma comment(lib, "HookDll.lib")

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INSTALLHOOK:
            InstallHook(WH_KEYBOARD, 0, hwndDlg);
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

    case WM_COPYDATA:
        SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_KEYBOARD), EM_SETSEL, -1, -1);
        SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_KEYBOARD), EM_REPLACESEL,
            TRUE, (LPARAM)(LPTSTR)(((PCOPYDATASTRUCT)lParam)->lpData));
        return TRUE;
    }

    return FALSE;
}