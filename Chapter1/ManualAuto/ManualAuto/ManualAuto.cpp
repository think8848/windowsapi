#include <windows.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;
HANDLE g_hEvent;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc1(LPVOID lpParameter);
DWORD WINAPI ThreadProc2(LPVOID lpParameter);
DWORD WINAPI ThreadProc3(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread[3] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // �����¼�����
        g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CREATETHREAD:
            // �����¼�����
            ResetEvent(g_hEvent);

            hThread[0] = CreateThread(NULL, 0, ThreadProc1, NULL, 0, NULL);
            hThread[1] = CreateThread(NULL, 0, ThreadProc2, NULL, 0, NULL);
            hThread[2] = CreateThread(NULL, 0, ThreadProc3, NULL, 0, NULL);
            for (int i = 0; i < 3; i++)
            {
                if (hThread[i] != NULL)
                    CloseHandle(hThread[i]);
            }

            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), TRUE);
            break;

        case IDC_BTN_SETEVENT:
            // �����¼�����
            SetEvent(g_hEvent);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETEVENT), FALSE);
            break;

        case IDCANCEL:
            // �ر��¼�������
            CloseHandle(g_hEvent);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc1(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("�߳�1�ɹ��ȴ����¼�����"), TEXT("��ʾ"), MB_OK);
    // ��һЩ����

    //SetEvent(g_hEvent);
    return 0;
}

DWORD WINAPI ThreadProc2(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("�߳�2�ɹ��ȴ����¼�����"), TEXT("��ʾ"), MB_OK);
    // ��һЩ����

    //SetEvent(g_hEvent);
    return 0;
}

DWORD WINAPI ThreadProc3(LPVOID lpParameter)
{
    WaitForSingleObject(g_hEvent, INFINITE);
    MessageBox(g_hwndDlg, TEXT("�߳�3�ɹ��ȴ����¼�����"), TEXT("��ʾ"), MB_OK);
    // ��һЩ����

    //SetEvent(g_hEvent);
    return 0;
}