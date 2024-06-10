#include <windows.h>
#include <Shlwapi.h>
#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

// ȫ�ֱ���
HWND g_hwndDlg;
BOOL g_bCancel = FALSE;     // ���Ʋ��������У�����û�����ˡ�ȡ������ť�����øñ���ΪTRUE

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �̺߳���������һ�����̸߳����Ʋ���
DWORD WINAPI ThreadProc(LPVOID lpParameter);
// CopyFileEx�����Ļص�����
DWORD CALLBACK CopyProgressRoutine(
    LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
    DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData);

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

        // ��ʼ��Դ�ļ���Ŀ���ļ��༭��
        SetDlgItemText(hwndDlg, IDC_EDIT_SOURCE, TEXT("F:\\Test.rar"));
        SetDlgItemText(hwndDlg, IDC_EDIT_TARGET, TEXT("F:\\Downloads\\Test.rar"));

        // ���ö��б༭�ؼ��Ļ�������СΪ������
        SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_PROCESS), EM_SETLIMITTEXT, 0, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_COPY:
            // �����߳̽��и��Ʋ���
            if ((hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL)) != NULL)
                CloseHandle(hThread);
            break;

        case IDC_BTN_CANCEL:
            g_bCancel = TRUE;
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
    TCHAR szSource[MAX_PATH] = { 0 };
    TCHAR szTarget[MAX_PATH] = { 0 };
    BOOL bRet = FALSE;

    GetDlgItemText(g_hwndDlg, IDC_EDIT_SOURCE, szSource, _countof(szSource));
    GetDlgItemText(g_hwndDlg, IDC_EDIT_TARGET, szTarget, _countof(szTarget));
    // ָ����Դ�ļ��Ƿ���Ч
    if (!PathFileExists(szSource))
    {
        MessageBox(g_hwndDlg, TEXT("ָ����Դ�ļ������ڣ�"), TEXT("��ʾ"), MB_OK);
        return 0;
    }
    // ָ����Ŀ���ļ��Ƿ��Ѵ���
    if (PathFileExists(szTarget))
    {
        int nRet = MessageBox(NULL, TEXT("ָ�������ļ��Ѿ����ڣ��Ƿ񸲸�Ŀ���ļ�"),
            TEXT("��ʾ"), MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON2);
        switch (nRet)
        {
        case IDOK:
            break;
        case IDCANCEL:
            return 0;
        }
    }

    bRet = CopyFileEx(szSource, szTarget, CopyProgressRoutine, NULL, &g_bCancel, 0);

    if (!bRet)
    {
        if (GetLastError() == ERROR_REQUEST_ABORTED)
            MessageBox(g_hwndDlg, TEXT("�û�ȡ���˸��Ʋ������̺߳�������"), TEXT("��ȡ��"), MB_OK);
    }
    else
    {
        MessageBox(g_hwndDlg, TEXT("�Ѿ��ɹ��������ļ����̺߳�������"), TEXT("�ѳɹ�"), MB_OK);
    }

    g_bCancel = FALSE;
    return  0;
}

DWORD CALLBACK CopyProgressRoutine(
    LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
    DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
    HWND hwndEdit = GetDlgItem(g_hwndDlg, IDC_EDIT_PROCESS);
    TCHAR szBuf[256] = { 0 };

    // ʵʱ��ʾ���ƽ���
    wsprintf(szBuf, TEXT("�ļ��ܴ�С��%I64X\t�Ѵ��䣺%I64X\t�ļ����ܴ�С��%I64X\t�Ѵ��䣺%I64X\t����ţ�%d\t\n"), TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart, StreamSize.QuadPart, StreamBytesTransferred.QuadPart, dwStreamNumber);
    SendMessage(hwndEdit, EM_SETSEL, -1, -1);
    SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    // �������Ʋ���
    return PROGRESS_CONTINUE;
}