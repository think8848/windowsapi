#include <windows.h>
#include <tchar.h>
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
    TCHAR szPath[MAX_PATH] = { 0 }; // �ļ�·��
    TCHAR szBuf[512] = { 0 };       // ׷������
    LARGE_INTEGER liFileSize;
    HANDLE hFile, hFileMap;
    LPVOID lpMemory;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hwndDlg, IDC_EDIT_PATH, TEXT("D:\\Test.txt"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OPEN:
            // ��һ���ļ�
            GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
            hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }
            else
            {
                GetFileSizeEx(hFile, &liFileSize);
                if (liFileSize.QuadPart == 0)
                {
                    MessageBox(hwndDlg, TEXT("�ļ���СΪ0"), TEXT("��ʾ"), MB_OK);
                    CloseHandle(hFile);
                    return TRUE;
                }
            }

            // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
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

            // ���ļ�������ʾ���༭�ؼ���
            SetDlgItemText(hwndDlg, IDC_EDIT_TEXT, (LPTSTR)lpMemory);

            // ������
            UnmapViewOfFile(lpMemory);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            break;

        case IDC_BTN_APPEND:
            if (!GetDlgItemText(hwndDlg, IDC_EDIT_APPEND, szBuf, _countof(szBuf)))
            {
                MessageBox(hwndDlg, TEXT("������׷������"), TEXT("��ʾ"), MB_OK);
                break;
            }

            // ��һ���ļ�
            GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
            hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
            // ��չ�ļ���ָ���Ĵ�С
            GetFileSizeEx(hFile, &liFileSize);
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, liFileSize.HighPart, 
                liFileSize.LowPart + _tcslen(szBuf) * sizeof(TCHAR), NULL);
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

            // д��׷������
            memcpy_s((LPBYTE)lpMemory + liFileSize.QuadPart, _tcslen(szBuf) * sizeof(TCHAR), 
                szBuf, _tcslen(szBuf) * sizeof(TCHAR));
            FlushViewOfFile(lpMemory, 0);
            // �����ļ�������ʾ���༭�ؼ���
            SetDlgItemText(hwndDlg, IDC_EDIT_TEXT, (LPTSTR)lpMemory);

            // ������
            UnmapViewOfFile(lpMemory);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}