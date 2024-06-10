#include <windows.h>
#include "resource.h"

// 常量定义
#define BUFFER_SIZE 2048

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

    LPCTSTR lpSrc[] = {
        TEXT("SystemDrive\t= %SystemDrive%"),
        TEXT("windir\t\t= %windir%"),
        TEXT("TEMP\t\t= %TEMP%"),
        TEXT("ProgramFiles\t= %ProgramFiles%"),
        TEXT("USERNAME\t= %USERNAME%"),
        TEXT("USERPROFILE\t= %USERPROFILE%"),
        TEXT("ALLUSERSPROFILE\t= %ALLUSERSPROFILE%"),
        TEXT("APPDATA\t\t= %APPDATA%"),
        TEXT("LOCALAPPDATA\t= %LOCALAPPDATA%") };
    TCHAR szDst[BUFFER_SIZE] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_ENV);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_LOOK:
            for (int i = 0; i < _countof(lpSrc); i++)
            {
                ExpandEnvironmentStrings(lpSrc[i], szDst, BUFFER_SIZE);

                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szDst);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\n"));
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