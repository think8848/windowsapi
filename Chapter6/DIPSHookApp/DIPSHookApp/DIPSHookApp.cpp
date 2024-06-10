#include <windows.h>
#include "resource.h"
#include "DIPSHookDll.h"

#pragma comment(lib, "DIPSHookDll.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndLV;
    HWND hwndDIPSServer;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndLV = GetTopWindow(GetTopWindow(FindWindow(TEXT("ProgMan"), NULL)));
        // 禁用保存桌面图标、恢复桌面图标和卸载消息钩子按钮
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INSTALLHOOK:
            InstallHook(WH_GETMESSAGE, GetWindowThreadProcessId(hwndLV, NULL));
            // 启用保存桌面图标、恢复桌面图标和卸载消息钩子按钮
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), TRUE);
            MessageBox(hwndDlg, TEXT("安装消息钩子成功"), TEXT("成功"), MB_OK);
            break;

        case IDC_BTN_UNINSTALLHOOK:
            // 获取服务器窗口句柄
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            // 使用SendMessage而不是PostMessage，确保钩子卸载以前，服务器对话框已经销毁
            SendMessage(hwndDIPSServer, WM_CLOSE, 0, 0);
            UninstallHook();
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), FALSE);
            MessageBox(hwndDlg, TEXT("卸载消息钩子成功"), TEXT("成功"), MB_OK);
            break;

        case IDC_BTN_SAVE:
            // 获取服务器窗口句柄
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            SendMessage(hwndDIPSServer, WM_APP, (WPARAM)hwndLV, TRUE);
            MessageBox(hwndDlg, TEXT("保存桌面图标成功"), TEXT("成功"), MB_OK);
            break;

        case IDC_BTN_RESTORE:
            // 获取服务器窗口句柄
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            SendMessage(hwndDIPSServer, WM_APP, (WPARAM)hwndLV, FALSE);
            MessageBox(hwndDlg, TEXT("恢复桌面图标成功"), TEXT("成功"), MB_OK);
            break;

        case IDCANCEL:
            if (FindWindow(NULL, TEXT("DIPSServer")))
                SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_UNINSTALLHOOK, 0);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}