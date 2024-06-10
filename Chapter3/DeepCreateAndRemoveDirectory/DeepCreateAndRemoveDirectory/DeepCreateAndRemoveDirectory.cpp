#include <windows.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ����Ŀ¼
BOOL MyCreateDirectory(LPTSTR lpPathName);
// ɾ��Ŀ¼
BOOL MyRemoveDirectory(LPTSTR lpPathName);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szPathName[MAX_PATH] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hwndDlg, IDC_EDIT_CREATE, TEXT("F:\\Downloads\\Web\\JavaWeb"));
        SetDlgItemText(hwndDlg, IDC_EDIT_REMOVE, TEXT("F:\\Downloads\\Web"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CREATE:
            if (GetDlgItemText(hwndDlg, IDC_EDIT_CREATE, szPathName, _countof(szPathName)))
            {
                if (MyCreateDirectory(szPathName))
                    MessageBox(hwndDlg, TEXT("����Ŀ¼�ɹ�"), TEXT("��ʾ"), MB_OK);
                else
                    MessageBox(hwndDlg, TEXT("����·�������Ƿ�����"), TEXT("ʧ��"), MB_OK);
            }
            break;

        case IDC_BTN_REMOVE:
            if (GetDlgItemText(hwndDlg, IDC_EDIT_REMOVE, szPathName, _countof(szPathName)))
            {
                if (MyRemoveDirectory(szPathName))
                    MessageBox(hwndDlg, TEXT("ɾ��Ŀ¼�ɹ�"), TEXT("��ʾ"), MB_OK);
                else
                    MessageBox(hwndDlg, TEXT("����·�������Ƿ�����"), TEXT("ʧ��"), MB_OK);
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

// ����Ŀ¼
BOOL MyCreateDirectory(LPTSTR lpPathName)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    LPTSTR lp;

    // �����ж�Ŀ¼�Ƿ��Ѿ�����
    if (PathFileExists(lpPathName))
        return TRUE;

    // ���������F:\Downloads\Web\JavaWeb\ĩβ��һ��\��б�ܣ���ȥ��
    if (lpPathName[_tcslen(lpPathName) - 1] == TEXT('\\'))
        lpPathName[_tcslen(lpPathName) - 1] = TEXT('\0');

    // �ݹ鴴����һ��Ŀ¼
    lp = _tcsrchr(lpPathName, TEXT('\\'));
    for (int i = 0; i < lp - lpPathName; i++)
        szBuf[i] = *(lpPathName + i);
    MyCreateDirectory(szBuf);

    // ������Ӧ��Ŀ¼
    if (CreateDirectory(lpPathName, NULL))
        return TRUE;

    return FALSE;
}

// ɾ��Ŀ¼
BOOL MyRemoveDirectory(LPTSTR lpPathName)
{
    TCHAR szDirectory[MAX_PATH] = { 0 };
    TCHAR szSearch[MAX_PATH] = { 0 };
    TCHAR szDirFile[MAX_PATH] = { 0 };
    HANDLE hFindFile;
    WIN32_FIND_DATA fd = { 0 };

    // ���·����βû��\�����һ��
    StringCchCopy(szDirectory, _countof(szDirectory), lpPathName);
    if (szDirectory[_tcslen(szDirectory) - 1] != TEXT('\\'))
        StringCchCat(szDirectory, _countof(szDirectory), TEXT("\\"));

    // ƴ�������ַ���
    StringCchCopy(szSearch, _countof(szSearch), szDirectory);
    StringCchCat(szSearch, _countof(szSearch), TEXT("*.*"));
    // �ݹ����Ŀ¼
    hFindFile = FindFirstFile(szSearch, &fd);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            // ����Ǵ���ǰĿ¼��.���ߴ�����һ��Ŀ¼��..������
            if (_tcscmp(fd.cFileName, TEXT(".")) == 0 || _tcscmp(fd.cFileName, TEXT("..")) == 0)
                continue;

            // �ҵ����ļ�����Ŀ¼����
            StringCchCopy(szDirFile, _countof(szDirFile), szDirectory);
            StringCchCat(szDirFile, _countof(szDirFile), fd.cFileName);

            // �������ҵ����ļ�����Ŀ¼
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // �����Ŀ¼���ݹ����
                MyRemoveDirectory(szDirFile);
            }
            else
            {
                // ɾ��ֻ������
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    SetFileAttributes(szDirFile, fd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                // ɾ���ļ�
                DeleteFile(szDirFile);
            }
        } while (FindNextFile(hFindFile, &fd));

        // �رղ��Ҿ��
        FindClose(hFindFile);
    }
    // ɾ����Ӧ��Ŀ¼
    if (RemoveDirectory(lpPathName))
        return TRUE;

    return FALSE;
}