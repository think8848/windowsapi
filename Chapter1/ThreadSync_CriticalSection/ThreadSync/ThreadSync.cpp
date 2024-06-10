#include <windows.h>
#include "resource.h"

// ��������
#define NUM 2

// ȫ�ֱ���
int g_n;
CRITICAL_SECTION g_cs;

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
    HANDLE hThread[NUM] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // ��ʼ���ؼ��ζ���CRITICAL_SECTION�ṹ
        InitializeCriticalSection(&g_cs);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), FALSE);
            g_n = 10;       // �����߳�ִ���̺߳�����ǰ��ȫ�ֱ���g_n��ֵΪ10
            for (int i = 0; i < NUM; i++)
                hThread[i] = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

            WaitForMultipleObjects(NUM, hThread, TRUE, INFINITE);
            for (int i = 0; i < NUM; i++)
            {
                if (hThread[i] != NULL)
                    CloseHandle(hThread[i]);
            }

            // �����߳̽����Ժ󣬰�g_n������ֵ��ʾ�ڱ༭�ؼ���
            SetDlgItemInt(hwndDlg, IDC_EDIT_NUM, g_n, TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
            break;

        case IDCANCEL:
            // �ͷŹؼ��ζ���
            DeleteCriticalSection(&g_cs);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    for (int i = 1; i <= 100000000; i++)
    {
        // ����ؼ���
        EnterCriticalSection(&g_cs);
        g_n++;
        g_n--;
        // �뿪�ؼ���
        LeaveCriticalSection(&g_cs);
    }

    return 0;
}