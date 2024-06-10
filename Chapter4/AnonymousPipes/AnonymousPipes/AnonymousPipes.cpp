#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// ��������
#define BUF_SIZE    1024

// ȫ�ֱ���
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
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        // ��ʼ���༭�ؼ�
        SetDlgItemText(hwndDlg, IDC_EDIT_URL, TEXT("www.baidu.com"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_PING:
            // �����߳�
            if ((hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL)) != NULL)
                CloseHandle(hThread);
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
    // ���������ܵ�
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(sa) };
    sa.bInheritHandle = TRUE;
    CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);

    // �����ӽ��̣����ӽ���Ping������ض��������ܵ���д����
    STARTUPINFO si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;

    // �����в���ƴ��Ϊ��Ping www.baidu.com����ʽ
    TCHAR szCommandLine[MAX_PATH] = TEXT("Ping ");
    TCHAR szURL[256] = { 0 };
    GetDlgItemText(g_hwndDlg, IDC_EDIT_URL, szURL, _countof(szURL));
    StringCchCat(szCommandLine, _countof(szCommandLine), szURL);

    // ����Ping�ӽ���
    if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        CHAR szBuf[BUF_SIZE + 1] = { 0 };
        CHAR szOutput[BUF_SIZE * 8] = { 0 };
        DWORD dwNumOfBytesRead;

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        while (TRUE)
        {
            // ��ȡ�����ܵ��Ķ�ȡ���
            ZeroMemory(szBuf, sizeof(szBuf));
            ReadFile(hReadPipe, szBuf, BUF_SIZE, &dwNumOfBytesRead, NULL);
            if (dwNumOfBytesRead == 0)
                break;

            // Ping����̨�������ANSI���룬���ʹ��StringCchCatA��SetDlgItemTextA
            // �Ѷ�ȡ��������׷�ӵ�szOutput������
            StringCchCatA(szOutput, _countof(szOutput), szBuf);
            // ��ʾ���༭�ؼ���
            SetDlgItemTextA(g_hwndDlg, IDC_EDIT_CONTENT, szOutput);
        }
    }

    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    return 0;
}