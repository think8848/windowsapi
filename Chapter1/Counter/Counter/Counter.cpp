#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 常量定义
#define F_START     1       // 开始计数
#define F_STOP      2       // 停止计数

// 全局变量
HWND g_hwndDlg;
int g_nOption;              // 标志

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID Counter();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 创建模态对话框
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndBtnStart, hwndBtnStop, hwndBtnPause, hwndBtnContinue;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        hwndBtnStart = GetDlgItem(hwndDlg, IDC_BTN_START);
        hwndBtnStop = GetDlgItem(hwndDlg, IDC_BTN_STOP);
        hwndBtnPause = GetDlgItem(hwndDlg, IDC_BTN_PAUSE);
        hwndBtnContinue = GetDlgItem(hwndDlg, IDC_BTN_CONTINUE);

        // 禁用停止、暂停、继续按钮
        EnableWindow(hwndBtnStop, FALSE);
        EnableWindow(hwndBtnPause, FALSE);
        EnableWindow(hwndBtnContinue, FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            g_nOption = 0;              // 如果按下开始，然后停止，然后再开始g_nOption的值为3
            g_nOption |= F_START;
            Counter();                  // 开始计数

            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            break;

        case IDC_BTN_STOP:
            g_nOption |= F_STOP;
            EnableWindow(hwndBtnStart, TRUE);
            EnableWindow(hwndBtnStop, FALSE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDC_BTN_PAUSE:
            g_nOption &= ~F_START;
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, TRUE);
            break;

        case IDC_BTN_CONTINUE:
            g_nOption |= F_START;
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

VOID Counter()
{
    int n = 0;

    while (!(g_nOption & F_STOP))
    {
        if (g_nOption & F_START)
            SetDlgItemInt(g_hwndDlg, IDC_EDIT_COUNT, n++, FALSE);
    }
}