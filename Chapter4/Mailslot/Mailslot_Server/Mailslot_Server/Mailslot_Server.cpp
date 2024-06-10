#include <windows.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szMailslotName[] = TEXT("\\\\.\\mailslot\\99F1755D-31FD-4CE5-8183-3F438316E8D7");
    static HANDLE hMailslot;
    DWORD dwNextSize, dwMessageCount, dwNumOfBytesRead;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // �����ʼ���
        hMailslot = CreateMailslot(szMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
        if (hMailslot == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
                MessageBox(hwndDlg, TEXT("ָ�����Ƶ��ʼ����Ѿ�����"), TEXT("������ʾ"), MB_OK);
            else
                MessageBox(hwndDlg, TEXT("CreateMailslot��������ʧ��"), TEXT("������ʾ"), MB_OK);

            ExitProcess(0);
        }

        // ������ʱ��
        SetTimer(hwndDlg, 1, 1000, NULL);
        return TRUE;

    case WM_TIMER:
        // �ڵ���ReadFile������ȡ������ǰ���ȵ���GetMailslotInfo������ȷ���ʼ������Ƿ�����Ϣ
        GetMailslotInfo(hMailslot, NULL, &dwNextSize, &dwMessageCount, NULL);
        for (DWORD i = 0; i < dwMessageCount; i++)
        {
            LPBYTE lpBuf = new BYTE[dwNextSize];
            ZeroMemory(lpBuf, dwNextSize);
            ReadFile(hMailslot, lpBuf, dwNextSize, &dwNumOfBytesRead, NULL);
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_MSG), LB_ADDSTRING, 0, (LPARAM)(LPTSTR)lpBuf);
            delete[]lpBuf;
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            CloseHandle(hMailslot);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}