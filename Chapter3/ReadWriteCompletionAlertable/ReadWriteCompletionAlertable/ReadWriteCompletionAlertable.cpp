#include <windows.h>
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ���ݸ�������̵��Զ������ݽṹ
typedef struct _IOData
{
    HANDLE m_hFileSource; // Դ�ļ����
    HANDLE m_hFileDest;   // Ŀ���ļ����
    HANDLE m_hFileMap;    // Դ�ļ�ӳ�����
    LPVOID m_lpMemory;    // ӳ��������ַ
}IOData, * PIOData;

// ȫ�ֱ���
HINSTANCE g_hInstance;
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �������
VOID WINAPI OverlappedCompletionRoutine(
    DWORD dwErrorCode,              // I/O�����״̬����
    DWORD dwNumberOfBytesTransfered,// �Ѵ�����ֽ���
    LPOVERLAPPED lpOverlapped);     // ��������I/O����ʱָ�����Ǹ�OVERLAPPED�ṹ��ָ��

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR  szFileNameSource[MAX_PATH] = { 0 };
    TCHAR  szFileNameDest[MAX_PATH] = { 0 };

    OPENFILENAME ofnSource = { 0 };
    ofnSource.lStructSize = sizeof(ofnSource);
    ofnSource.hwndOwner = hwndDlg;
    ofnSource.lpstrFilter = TEXT("All(*.*)\0*.*\0");
    ofnSource.nFilterIndex = 1;                          // Ĭ��ѡ���1��������
    ofnSource.lpstrFile = szFileNameSource;              // �����û�ѡ����ļ����Ļ�����
    ofnSource.lpstrFile[0] = NULL;                       // ����Ҫ��ʼ���ļ����༭�ؼ�
    ofnSource.nMaxFile = _countof(szFileNameSource);
    ofnSource.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵��ļ�");   // �Ի������������ʾ���ַ���
    ofnSource.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

    OPENFILENAME ofnDest = { 0 };
    ofnDest.lStructSize = sizeof(ofnDest);
    ofnDest.hwndOwner = hwndDlg;
    ofnDest.lpstrFilter = TEXT("All(*.*)\0*.*\0");
    ofnDest.nFilterIndex = 1;                           // Ĭ��ѡ���1��������
    ofnDest.lpstrFile = szFileNameDest;                 // �����û�ѡ����ļ����Ļ�����
    ofnDest.lpstrFile[0] = NULL;                        // ����Ҫ��ʼ���ļ����༭�ؼ�
    ofnDest.nMaxFile = _countof(szFileNameDest);
    ofnDest.lpstrTitle = TEXT("��ѡ��Ҫ������ļ���");  // �Ի������������ʾ���ַ���
    ofnDest.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

    HANDLE hFileSource = INVALID_HANDLE_VALUE, hFileDest = INVALID_HANDLE_VALUE;
    HANDLE hFileMap = NULL;
    LPVOID lpMemory = NULL;
    DWORD  dwFileSizeLow = 0;
    DWORD  dwFileSizeHigh = 0;
    BOOL   bRet = FALSE;
    DWORD  dwRet = 0;
    PIOData pIOData = NULL;
    LPOVERLAPPED lpOverlapped = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_SOURCE:
            if (GetOpenFileName(&ofnSource))
                SetDlgItemText(hwndDlg, IDC_EDIT_SOURCE, szFileNameSource);
            break;

        case IDC_BTN_DEST:
            if (GetSaveFileName(&ofnDest))
                SetDlgItemText(hwndDlg, IDC_EDIT_DEST, szFileNameDest);
            break;

        case IDC_BTN_COPY:
            // ��Դ�ļ�
            GetDlgItemText(hwndDlg, IDC_EDIT_SOURCE, szFileNameSource, _countof(szFileNameSource));
            hFileSource = CreateFile(szFileNameSource, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (hFileSource == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("��Դ�ļ�ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ��ȡԴ�ļ���С
            dwFileSizeLow = GetFileSize(hFileSource, &dwFileSizeHigh);

            // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
            hFileMap = CreateFileMapping(hFileSource, NULL, PAGE_READWRITE, 0, 0, NULL);
            if (!hFileMap)
            {
                MessageBox(hwndDlg, TEXT("CreateFileMapping����ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
            lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (!lpMemory)
            {
                MessageBox(hwndDlg, TEXT("MapViewOfFile����ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ����Ŀ���ļ�
            GetDlgItemText(hwndDlg, IDC_EDIT_DEST, szFileNameDest, _countof(szFileNameDest));
            hFileDest = CreateFile(szFileNameDest, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (hFileDest == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("����Ŀ���ļ�ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ����һ��IOData�ṹ������ؾ�����ݹ�ȥ���ͷ�
            pIOData = new IOData;
            pIOData->m_hFileSource = hFileSource;
            pIOData->m_hFileDest = hFileDest;
            pIOData->m_hFileMap = hFileMap;
            pIOData->m_lpMemory = lpMemory;

            // �첽д��Ŀ���ļ�
            lpOverlapped = new OVERLAPPED;
            ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));
            lpOverlapped->Offset = 0;
            lpOverlapped->OffsetHigh = 0;
            lpOverlapped->hEvent = pIOData; // ͨ��hEvent�ֶδ��ݸ��������һ���Զ������ݽṹ
            bRet = WriteFileEx(hFileDest, lpMemory, dwFileSizeLow, lpOverlapped, OverlappedCompletionRoutine);

            // ���Խ���ȥ��һЩ�������飬���ﲥ��һ������
            PlaySound(TEXT("����һ�Ƽ�į�ĳ�.wav"), NULL, SND_FILENAME | SND_ASYNC/* | SND_LOOP*/);
            break;

        case IDC_BTN_QUERY:
            dwRet = SleepEx(INFINITE, TRUE);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }

    return FALSE;
}

VOID WINAPI OverlappedCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    PIOData pIOData = (PIOData)(lpOverlapped->hEvent);

    MessageBox(g_hwndDlg, TEXT("д��Ŀ���ļ����"), TEXT("��ʾ"), MB_OK);

    // ������
    UnmapViewOfFile(pIOData->m_lpMemory);
    CloseHandle(pIOData->m_hFileMap);
    CloseHandle(pIOData->m_hFileSource);
    CloseHandle(pIOData->m_hFileDest);

    delete pIOData;
    delete lpOverlapped;

    return;
}