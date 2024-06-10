#include <windows.h>
#include <tchar.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szPath[MAX_PATH] = { 0 }; // 文件路径
    TCHAR szBuf[512] = { 0 };       // 追加数据
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
            // 打开一个文件
            GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
            hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }
            else
            {
                GetFileSizeEx(hFile, &liFileSize);
                if (liFileSize.QuadPart == 0)
                {
                    MessageBox(hwndDlg, TEXT("文件大小为0"), TEXT("提示"), MB_OK);
                    CloseHandle(hFile);
                    return TRUE;
                }
            }

            // 为hFile文件对象创建一个文件映射内核对象
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
            if (!hFileMap)
            {
                MessageBox(hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
            lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (!lpMemory)
            {
                MessageBox(hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 把文件内容显示到编辑控件中
            SetDlgItemText(hwndDlg, IDC_EDIT_TEXT, (LPTSTR)lpMemory);

            // 清理工作
            UnmapViewOfFile(lpMemory);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            break;

        case IDC_BTN_APPEND:
            if (!GetDlgItemText(hwndDlg, IDC_EDIT_APPEND, szBuf, _countof(szBuf)))
            {
                MessageBox(hwndDlg, TEXT("请输入追加内容"), TEXT("提示"), MB_OK);
                break;
            }

            // 打开一个文件
            GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
            hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 为hFile文件对象创建一个文件映射内核对象
            // 扩展文件到指定的大小
            GetFileSizeEx(hFile, &liFileSize);
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, liFileSize.HighPart, 
                liFileSize.LowPart + _tcslen(szBuf) * sizeof(TCHAR), NULL);
            if (!hFileMap)
            {
                MessageBox(hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
            lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (!lpMemory)
            {
                MessageBox(hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 写入追加数据
            memcpy_s((LPBYTE)lpMemory + liFileSize.QuadPart, _tcslen(szBuf) * sizeof(TCHAR), 
                szBuf, _tcslen(szBuf) * sizeof(TCHAR));
            FlushViewOfFile(lpMemory, 0);
            // 把新文件内容显示到编辑控件中
            SetDlgItemText(hwndDlg, IDC_EDIT_TEXT, (LPTSTR)lpMemory);

            // 清理工作
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