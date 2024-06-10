#include <windows.h>
#include "resource.h"
#include "DrawDll.h"

#pragma comment(lib, "DrawDll.lib")

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL bFirst = TRUE;
    static BOOL bRect;
    HDC hdc;
    PAINTSTRUCT ps;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_DRAWRECT:
            bFirst = FALSE;
            bRect = TRUE;
            InvalidateRect(hwndDlg, NULL, TRUE);
            break;

        case IDC_BTN_DRAWELLIPSE:
            bFirst = FALSE;
            bRect = FALSE;
            InvalidateRect(hwndDlg, NULL, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_PAINT:
        hdc = BeginPaint(hwndDlg, &ps);
        if (!bFirst)
        {
            if (bRect)
                DrawRectangle(hwndDlg);
            else
                DrawEllipse(hwndDlg);
        }
        EndPaint(hwndDlg, &ps);
        return TRUE;
    }

    return FALSE;
}