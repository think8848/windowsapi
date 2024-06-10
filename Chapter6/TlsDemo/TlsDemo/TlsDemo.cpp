#include <windows.h>
#include "resource.h"

// �궨��
#define THREADCOUNT 5

// ȫ�ֱ���
DWORD g_dwTlsIndex;
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �̺߳���
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread[THREADCOUNT] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            // �ӽ����з���һ��TLS����
            g_dwTlsIndex = TlsAlloc();
            if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
            {
                MessageBox(hwndDlg, TEXT("TlsAlloc��������ʧ�ܣ�"), TEXT("������ʾ"), MB_OK);
                return FALSE;
            }

            // ����THREADCOUNT���߳�
            SetDlgItemText(g_hwndDlg, IDC_EDIT_TLSSLOTS, TEXT(""));
            for (int i = 0; i < THREADCOUNT; i++)
            {
                if ((hThread[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL)) != NULL)
                    CloseHandle(hThread[i]);
            }

            // �ȴ������߳̽������ͷ�TLS����
            WaitForMultipleObjects(THREADCOUNT, hThread, TRUE, INFINITE);
            TlsFree(g_dwTlsIndex);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    LPVOID lpData = NULL;
    TCHAR szBuf[64] = { 0 };

    lpData = new BYTE[256];
    ZeroMemory(lpData, 256);

    // ���Լ��Ĵ洢����ָ��������д������
    if (!TlsSetValue(g_dwTlsIndex, lpData))
    {
        wsprintf(szBuf, TEXT("�߳�%d����TlsSetValueʧ��"), (INT)lpParameter);
        MessageBox(g_hwndDlg, szBuf, TEXT("������ʾ"), MB_OK);
        delete[]lpData;
        return 0;
    }

    // ��ȡ�Լ��Ĵ洢����ָ��������������
    lpData = TlsGetValue(g_dwTlsIndex);
    if (!lpData && GetLastError() != ERROR_SUCCESS)
    {
        wsprintf(szBuf, TEXT("�߳�%d����TlsGetValueʧ��"), (INT)lpParameter);
        MessageBox(g_hwndDlg, szBuf, TEXT("������ʾ"), MB_OK);
    }
    // ÿ���̴߳洢����ָ����������������ʾ���༭�ؼ���
    wsprintf(szBuf, TEXT("�߳�%d������%d����ֵ��0x%p\r\n"), (INT)lpParameter, g_dwTlsIndex, lpData);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_SETSEL, -1, -1);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    delete[]lpData;
    return 0;
}