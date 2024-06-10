#include <windows.h>
#include <tchar.h>
#include "resource.h"

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    LPTSTR lpEnvironmentStrings;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_ENV);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_LOOK:
            SetWindowText(hwndEdit, TEXT(""));

            lpEnvironmentStrings = GetEnvironmentStrings();
            while (lpEnvironmentStrings[0] != TEXT('\0'))
            {
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)lpEnvironmentStrings);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\n"));

                lpEnvironmentStrings += _tcslen(lpEnvironmentStrings) + 1;
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