#include <windows.h>
#include <shlwapi.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Shlwapi.lib")

// �Զ�����Ϣ
#define WM_DIRECTORYCHANGES (WM_APP + 1)

// ȫ�ֱ���
HWND  g_hwndDlg;
BOOL  g_bStarting;                  // �����߳̿�ʼ��������־
TCHAR g_szShowChanges[1024];        // ��ʾָ��Ŀ¼���ļ�����Ŀ¼�仯������õĻ�����

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
    static HWND hwndEditChanges;
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        // ���б༭�ؼ����ھ��
        hwndEditChanges = GetDlgItem(hwndDlg, IDC_EDIT_CHANGES);
        // ��ʼ������Ŀ¼�༭��
        SetDlgItemText(hwndDlg, IDC_EDIT_PATH, TEXT("C:\\"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            // �����̣߳���ʼ����Ŀ¼�仯
            g_bStarting = TRUE;
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread)
                CloseHandle(hThread);
            break;

        case IDC_BTN_STOP:
            g_bStarting = FALSE;
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_DIRECTORYCHANGES:
        // �����Զ�����Ϣ����ʾg_szShowChanges�е�Ŀ¼�仯���
        SendMessage(hwndEditChanges, EM_SETSEL, -1, -1);
        SendMessage(hwndEditChanges, EM_REPLACESEL, TRUE, (LPARAM)g_szShowChanges);
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    TCHAR   szPath[MAX_PATH] = { 0 };           // ��ȡ����Ŀ¼�༭�ؼ��е�·��
    HANDLE  hDirectory = INVALID_HANDLE_VALUE;  // Ҫ���ӵ�Ŀ¼�ľ��
    TCHAR   szBuffer[1024] = { 0 };             // ����Ŀ¼�仯��Ϣ�Ļ�����
    DWORD   dwBytesReturned;                    // ʵ��д�뵽���������ֽ���
    PFILE_NOTIFY_INFORMATION pFNI, pFNINext;
    TCHAR szFileName[MAX_PATH], szFileNameNew[MAX_PATH];

    // ��ն��б༭�ؼ�
    SetDlgItemText(g_hwndDlg, IDC_EDIT_CHANGES, TEXT(""));

    // ��Ŀ¼
    GetDlgItemText(g_hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
    hDirectory = CreateFile(szPath, /*GENERIC_READ | GENERIC_WRITE | */FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hDirectory == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("Error"), MB_OK);
        return 0;
    }

    while (g_bStarting)
    {
        if (!PathFileExists(szPath))
        {
            MessageBox(g_hwndDlg, TEXT("����Ŀ¼�ļ����ѱ�ɾ��"), TEXT("Error"), MB_OK);
            return 0;
        }

        // ����ͬ��������ֱ��ָ��Ŀ¼�е��ļ���Ŀ¼�����仯ʱ��ReadDirectoryChangesW�����ŷ���
        // ���ʹ���첽�����Ƚ�ǡ��һЩ
        ZeroMemory(szBuffer, sizeof(szBuffer));
        ReadDirectoryChangesW(hDirectory, szBuffer, sizeof(szBuffer), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS |
            FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY,
            &dwBytesReturned, NULL, NULL);

        pFNI = (PFILE_NOTIFY_INFORMATION)szBuffer;
        ZeroMemory(szFileName, sizeof(szFileName));
        ZeroMemory(szFileNameNew, sizeof(szFileNameNew));
        memcpy_s(szFileName, sizeof(szFileName), pFNI->FileName, pFNI->FileNameLength);
        if (pFNI->NextEntryOffset)
        {
            pFNINext = (PFILE_NOTIFY_INFORMATION)((LPBYTE)pFNI + pFNI->NextEntryOffset);
            memcpy_s(szFileNameNew, sizeof(szFileNameNew),
                pFNINext->FileName, pFNINext->FileNameLength);
        }

        // �����̰߳�Ŀ¼�仯���д��g_szShowChanges��
        ZeroMemory(g_szShowChanges, sizeof(g_szShowChanges));
        switch (pFNI->Action)
        {
        case FILE_ACTION_ADDED:
            wsprintf(g_szShowChanges, TEXT("�½��ļ���Ŀ¼��%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_REMOVED:
            wsprintf(g_szShowChanges, TEXT("ɾ���ļ���Ŀ¼��%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_MODIFIED:
            wsprintf(g_szShowChanges, TEXT("�޸��ļ���Ŀ¼��%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_RENAMED_OLD_NAME:
            wsprintf(g_szShowChanges, TEXT("�ļ�Ŀ¼��������%s  -->  %s\n"),
                szFileName, szFileNameNew);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;
        }
    }

    return 0;
}