#include <windows.h>
#include "resource.h"

#define BUF_SIZE 4096

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hFileMap;
    static LPVOID lpMemory;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // 创建或打开一个 命名文件映射内核对象，BUF_SIZE字节
        hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, BUF_SIZE, TEXT("2F4368E6-09A1-4D5E-ACC9-C1BDBB041BF7"));
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

        // 创建一个计时器，每一秒钟在静态控件中刷新显示一次共享数据
        SetTimer(hwndDlg, 1, 1000, NULL);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_EDIT_SHARE:
            // 编辑控件中的内容改变
            if (HIWORD(wParam) == EN_UPDATE)
                GetDlgItemText(hwndDlg, IDC_EDIT_SHARE, (LPTSTR)lpMemory, BUF_SIZE);
            break;

        case IDCANCEL:
            // 清理工作
            KillTimer(hwndDlg, 1);
            UnmapViewOfFile(lpMemory);
            CloseHandle(hFileMap);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_TIMER:
        SetDlgItemText(hwndDlg, IDC_STATIC_SHARE, (LPTSTR)lpMemory);
        return TRUE;
    }

    return FALSE;
}

/*
GUID guid;
TCHAR szGUID[64] = { 0 };

// 生成一个GUID
CoCreateGuid(&guid);
// 转换为字符串
wsprintf(szGUID, TEXT("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"),
    guid.Data1, guid.Data2, guid.Data3,
    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
*/