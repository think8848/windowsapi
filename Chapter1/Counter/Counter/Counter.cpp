#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ��������
#define F_START     1       // ��ʼ����
#define F_STOP      2       // ֹͣ����

// ȫ�ֱ���
HWND g_hwndDlg;
int g_nOption;              // ��־

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID Counter();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ����ģ̬�Ի���
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

        // ����ֹͣ����ͣ��������ť
        EnableWindow(hwndBtnStop, FALSE);
        EnableWindow(hwndBtnPause, FALSE);
        EnableWindow(hwndBtnContinue, FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            g_nOption = 0;              // ������¿�ʼ��Ȼ��ֹͣ��Ȼ���ٿ�ʼg_nOption��ֵΪ3
            g_nOption |= F_START;
            Counter();                  // ��ʼ����

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