#include <windows.h>
#include "resource.h"

// �궨��
#define THREADCOUNT 5

// ȫ�ֱ���
__declspec(thread) LPVOID gt_lpData = NULL;
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
    HANDLE hThread[THREADCOUNT];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            // ����THREADCOUNT���߳�
            SetDlgItemText(g_hwndDlg, IDC_EDIT_TLSSLOTS, TEXT(""));
            for (int i = 0; i < THREADCOUNT; i++)
            {
                if ((hThread[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL)) != NULL)
                    CloseHandle(hThread[i]);
            }
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
    TCHAR szBuf[64] = { 0 };

    gt_lpData = new BYTE[256];
    ZeroMemory(gt_lpData, 256);

    // ÿ���̵߳ľ�̬TLS������ʾ���༭�ؼ���
    wsprintf(szBuf, TEXT("�߳�%d��gt_lpDataֵ��0x%p\r\n"), (INT)lpParameter, gt_lpData);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_SETSEL, -1, -1);
    SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_TLSSLOTS), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    delete[]gt_lpData;
    return 0;
}