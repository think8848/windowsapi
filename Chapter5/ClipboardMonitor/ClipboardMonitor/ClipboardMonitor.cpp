#include <Windows.h>
#include <Richedit.h>
#include "resource.h"

#ifdef UNICODE
    #define CF_TCHAR CF_UNICODETEXT
#else
    #define CF_TCHAR CF_TEXT
#endif

// 全局变量
HWND g_hwndNextViewer;      // 剪贴板查看器链中下一个剪贴板查看器的窗口句柄

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

        // 将当前程序窗口添加到剪贴板查看器的链中
        g_hwndNextViewer = SetClipboardViewer(hwndDlg);

        hwndEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            // 从剪贴板查看器链中删除自身
            ChangeClipboardChain(hwndDlg, g_hwndNextViewer);
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        return TRUE;

    case WM_DRAWCLIPBOARD:
        // 剪贴板的内容发生变化
        // 如果剪贴板查看器链中存在下一个窗口
        if (g_hwndNextViewer)
            SendMessage(g_hwndNextViewer, uMsg, wParam, lParam);

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

    case WM_CHANGECBCHAIN:
        // 处理WM_CHANGECBCHAIN消息，维护好剪贴板查看器链
        if ((HWND)wParam == g_hwndNextViewer)
            g_hwndNextViewer = (HWND)lParam;
        else if (g_hwndNextViewer)
            SendMessage(g_hwndNextViewer, uMsg, wParam, lParam);
        return TRUE;
    }

    return FALSE;
}