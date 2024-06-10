#include <Windows.h>
#include <Richedit.h>
#include "resource.h"

#ifdef UNICODE
    #define CF_TCHAR CF_UNICODETEXT
#else
    #define CF_TCHAR CF_TEXT
#endif

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 加载Riched20.dll动态链接库
    LoadLibrary(TEXT("Riched20.dll"));

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    LONG lStyle;
    LPTSTR lpStr;
    HBITMAP hBmp;
    UINT uFormat;
    TCHAR szSeparator[] = TEXT("\n--------------------------------------------------------\n");

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // 显示图片的静态控件设置SS_BITMAP | SS_REALSIZECONTROL风格
        lStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATIC_BITMAP), GWL_STYLE);
        SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATIC_BITMAP), GWL_STYLE,
            lStyle | SS_BITMAP | SS_REALSIZECONTROL);

        // 注册成为剪贴板格式监听器
        if (!AddClipboardFormatListener(hwndDlg))
            MessageBox(hwndDlg, TEXT("注册成为剪贴板格式监听器失败"), TEXT("Error"), MB_OK);

        hwndEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            // 移除剪贴板格式监听器
            RemoveClipboardFormatListener(hwndDlg);
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        return TRUE;

    case WM_CLIPBOARDUPDATE:
        // 更新显示
        OpenClipboard(hwndDlg);
        uFormat = EnumClipboardFormats(0);
        while (uFormat)
        {
            // 这里只处理这2种格式
            if (uFormat == CF_TCHAR)
            {
                lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)lpStr);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szSeparator);
            }
            else if (uFormat == CF_BITMAP)
            {
                hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
                SendDlgItemMessage(hwndDlg, IDC_STATIC_BITMAP, STM_SETIMAGE,
                    IMAGE_BITMAP, (LPARAM)hBmp);
            }

            uFormat = EnumClipboardFormats(uFormat);
        }
        CloseClipboard();
        return TRUE;
    }

    return FALSE;
}