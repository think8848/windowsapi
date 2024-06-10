#include <windows.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 创建目录
BOOL MyCreateDirectory(LPTSTR lpPathName);
// 删除目录
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
                    MessageBox(hwndDlg, TEXT("创建目录成功"), TEXT("提示"), MB_OK);
                else
                    MessageBox(hwndDlg, TEXT("请检查路径输入是否有误"), TEXT("失败"), MB_OK);
            }
            break;

        case IDC_BTN_REMOVE:
            if (GetDlgItemText(hwndDlg, IDC_EDIT_REMOVE, szPathName, _countof(szPathName)))
            {
                if (MyRemoveDirectory(szPathName))
                    MessageBox(hwndDlg, TEXT("删除目录成功"), TEXT("提示"), MB_OK);
                else
                    MessageBox(hwndDlg, TEXT("请检查路径输入是否有误"), TEXT("失败"), MB_OK);
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

// 创建目录
BOOL MyCreateDirectory(LPTSTR lpPathName)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    LPTSTR lp;

    // 首先判断目录是否已经存在
    if (PathFileExists(lpPathName))
        return TRUE;

    // 如果是这样F:\Downloads\Web\JavaWeb\末尾有一个\反斜杠，则去掉
    if (lpPathName[_tcslen(lpPathName) - 1] == TEXT('\\'))
        lpPathName[_tcslen(lpPathName) - 1] = TEXT('\0');

    // 递归创建上一级目录
    lp = _tcsrchr(lpPathName, TEXT('\\'));
    for (int i = 0; i < lp - lpPathName; i++)
        szBuf[i] = *(lpPathName + i);
    MyCreateDirectory(szBuf);

    // 创建相应的目录
    if (CreateDirectory(lpPathName, NULL))
        return TRUE;

    return FALSE;
}

// 删除目录
BOOL MyRemoveDirectory(LPTSTR lpPathName)
{
    TCHAR szDirectory[MAX_PATH] = { 0 };
    TCHAR szSearch[MAX_PATH] = { 0 };
    TCHAR szDirFile[MAX_PATH] = { 0 };
    HANDLE hFindFile;
    WIN32_FIND_DATA fd = { 0 };

    // 如果路径结尾没有\则添加一个
    StringCchCopy(szDirectory, _countof(szDirectory), lpPathName);
    if (szDirectory[_tcslen(szDirectory) - 1] != TEXT('\\'))
        StringCchCat(szDirectory, _countof(szDirectory), TEXT("\\"));

    // 拼接搜索字符串
    StringCchCopy(szSearch, _countof(szSearch), szDirectory);
    StringCchCat(szSearch, _countof(szSearch), TEXT("*.*"));
    // 递归遍历目录
    hFindFile = FindFirstFile(szSearch, &fd);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            // 如果是代表当前目录的.或者代表上一级目录的..则跳过
            if (_tcscmp(fd.cFileName, TEXT(".")) == 0 || _tcscmp(fd.cFileName, TEXT("..")) == 0)
                continue;

            // 找到的文件或子目录名称
            StringCchCopy(szDirFile, _countof(szDirFile), szDirectory);
            StringCchCat(szDirFile, _countof(szDirFile), fd.cFileName);

            // 处理本次找到的文件或子目录
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 如果是目录，递归调用
                MyRemoveDirectory(szDirFile);
            }
            else
            {
                // 删除只读属性
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    SetFileAttributes(szDirFile, fd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                // 删除文件
                DeleteFile(szDirFile);
            }
        } while (FindNextFile(hFindFile, &fd));

        // 关闭查找句柄
        FindClose(hFindFile);
    }
    // 删除相应的目录
    if (RemoveDirectory(lpPathName))
        return TRUE;

    return FALSE;
}