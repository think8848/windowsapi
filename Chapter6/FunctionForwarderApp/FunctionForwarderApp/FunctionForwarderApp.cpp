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
    HMODULE hFunctionForwarder = NULL;
    typedef BOOL(WINAPI* pfnMyMessageBox)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
    pfnMyMessageBox pMyMessageBox = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_MYMESSAGEBOX:
            hFunctionForwarder = LoadLibrary(TEXT("FunctionForwarderDll.dll"));
            if (hFunctionForwarder)
            {
                pMyMessageBox = (pfnMyMessageBox)GetProcAddress(hFunctionForwarder, "MyMessageBox");
                if (pMyMessageBox)
                    pMyMessageBox(hwndDlg, TEXT("MyMessageBox"), TEXT("提示"), MB_OK);

                FreeLibrary(hFunctionForwarder);
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