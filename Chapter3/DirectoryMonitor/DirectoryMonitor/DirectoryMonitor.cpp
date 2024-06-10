#include <windows.h>
#include <shlwapi.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Shlwapi.lib")

// 自定义消息
#define WM_DIRECTORYCHANGES (WM_APP + 1)

// 全局变量
HWND  g_hwndDlg;
BOOL  g_bStarting;                  // 工作线程开始、结束标志
TCHAR g_szShowChanges[1024];        // 显示指定目录中文件、子目录变化结果所用的缓冲区

// 函数声明
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
        // 多行编辑控件窗口句柄
        hwndEditChanges = GetDlgItem(hwndDlg, IDC_EDIT_CHANGES);
        // 初始化监视目录编辑框
        SetDlgItemText(hwndDlg, IDC_EDIT_PATH, TEXT("C:\\"));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            // 创建线程，开始监视目录变化
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
        // 处理自定义消息，显示g_szShowChanges中的目录变化结果
        SendMessage(hwndEditChanges, EM_SETSEL, -1, -1);
        SendMessage(hwndEditChanges, EM_REPLACESEL, TRUE, (LPARAM)g_szShowChanges);
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    TCHAR   szPath[MAX_PATH] = { 0 };           // 获取监视目录编辑控件中的路径
    HANDLE  hDirectory = INVALID_HANDLE_VALUE;  // 要监视的目录的句柄
    TCHAR   szBuffer[1024] = { 0 };             // 返回目录变化信息的缓冲区
    DWORD   dwBytesReturned;                    // 实际写入到缓冲区的字节数
    PFILE_NOTIFY_INFORMATION pFNI, pFNINext;
    TCHAR szFileName[MAX_PATH], szFileNameNew[MAX_PATH];

    // 清空多行编辑控件
    SetDlgItemText(g_hwndDlg, IDC_EDIT_CHANGES, TEXT(""));

    // 打开目录
    GetDlgItemText(g_hwndDlg, IDC_EDIT_PATH, szPath, _countof(szPath));
    hDirectory = CreateFile(szPath, /*GENERIC_READ | GENERIC_WRITE | */FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hDirectory == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("Error"), MB_OK);
        return 0;
    }

    while (g_bStarting)
    {
        if (!PathFileExists(szPath))
        {
            MessageBox(g_hwndDlg, TEXT("监视目录文件夹已被删除"), TEXT("Error"), MB_OK);
            return 0;
        }

        // 对于同步操作，直到指定目录中的文件、目录发生变化时，ReadDirectoryChangesW函数才返回
        // 因此使用异步操作比较恰当一些
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

        // 工作线程把目录变化结果写入g_szShowChanges中
        ZeroMemory(g_szShowChanges, sizeof(g_szShowChanges));
        switch (pFNI->Action)
        {
        case FILE_ACTION_ADDED:
            wsprintf(g_szShowChanges, TEXT("新建文件、目录：%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_REMOVED:
            wsprintf(g_szShowChanges, TEXT("删除文件、目录：%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_MODIFIED:
            wsprintf(g_szShowChanges, TEXT("修改文件、目录：%s\n"), szFileName);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;

        case FILE_ACTION_RENAMED_OLD_NAME:
            wsprintf(g_szShowChanges, TEXT("文件目录重命名：%s  -->  %s\n"),
                szFileName, szFileNameNew);
            PostMessage(g_hwndDlg, WM_DIRECTORYCHANGES, 0, 0);
            break;
        }
    }

    return 0;
}