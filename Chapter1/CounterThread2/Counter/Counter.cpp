#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ȫ�ֱ���
HWND g_hwndDlg;
HANDLE g_hEventStart;       // �¼�����������Ϊ��ʼ��־
HANDLE g_hEventStop;        // �¼�����������Ϊֹͣ��־

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndBtnStart, hwndBtnStop, hwndBtnPause, hwndBtnContinue;
    HANDLE hThread = NULL;

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

        // �����¼�����
        g_hEventStart = CreateEvent(NULL, TRUE, FALSE, NULL);
        g_hEventStop = CreateEvent(NULL, TRUE, FALSE, NULL);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread != NULL)
            {
                CloseHandle(hThread);
                hThread = NULL;
            }

            SetEvent(g_hEventStart);    // ���ÿ�ʼ��־
            ResetEvent(g_hEventStop);   // ���ֹͣ��־

            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            break;

        case IDC_BTN_STOP:
            SetEvent(g_hEventStop);     // ����ֹͣ��־
            EnableWindow(hwndBtnStart, TRUE);
            EnableWindow(hwndBtnStop, FALSE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDC_BTN_PAUSE:
            ResetEvent(g_hEventStart);  // �����ʼ��־
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, FALSE);
            EnableWindow(hwndBtnContinue, TRUE);
            break;

        case IDC_BTN_CONTINUE:
            SetEvent(g_hEventStart);    // ���ÿ�ʼ��־
            EnableWindow(hwndBtnStart, FALSE);
            EnableWindow(hwndBtnStop, TRUE);
            EnableWindow(hwndBtnPause, TRUE);
            EnableWindow(hwndBtnContinue, FALSE);
            break;

        case IDCANCEL:
            // �ر��¼�������
            CloseHandle(g_hEventStart);
            CloseHandle(g_hEventStop);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int n = 0;

    while (WaitForSingleObject(g_hEventStop, 0) != WAIT_OBJECT_0)       // �Ƿ�������ֹͣ��־
    {
        if (WaitForSingleObject(g_hEventStart, 100) == WAIT_OBJECT_0)   // �Ƿ������˿�ʼ��־
            SetDlgItemInt(g_hwndDlg, IDC_EDIT_COUNT, n++, FALSE);
    }

    return 0;
}