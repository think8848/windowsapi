#include <windows.h>
#include <Shlwapi.h>
#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

// 全局变量
HWND g_hwndDlg;
BOOL g_bCancel = FALSE;     // 复制操作过程中，如果用户点击了“取消”按钮，设置该变量为TRUE

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 线程函数，创建一个新线程负责复制操作
DWORD WINAPI ThreadProc(LPVOID lpParameter);
// CopyFileEx函数的回调函数
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

        // 初始化源文件、目标文件编辑框
        SetDlgItemText(hwndDlg, IDC_EDIT_SOURCE, TEXT("F:\\Test.rar"));
        SetDlgItemText(hwndDlg, IDC_EDIT_TARGET, TEXT("F:\\Downloads\\Test.rar"));

        // 设置多行编辑控件的缓冲区大小为不限制
        SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_PROCESS), EM_SETLIMITTEXT, 0, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_COPY:
            // 创建线程进行复制操作
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
    // 指定的源文件是否有效
    if (!PathFileExists(szSource))
    {
        MessageBox(g_hwndDlg, TEXT("指定的源文件不存在！"), TEXT("提示"), MB_OK);
        return 0;
    }
    // 指定的目标文件是否已存在
    if (PathFileExists(szTarget))
    {
        int nRet = MessageBox(NULL, TEXT("指定的新文件已经存在，是否覆盖目标文件"),
            TEXT("提示"), MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON2);
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
            MessageBox(g_hwndDlg, TEXT("用户取消了复制操作，线程函数返回"), TEXT("已取消"), MB_OK);
    }
    else
    {
        MessageBox(g_hwndDlg, TEXT("已经成功复制了文件，线程函数返回"), TEXT("已成功"), MB_OK);
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

    // 实时显示复制进度
    wsprintf(szBuf, TEXT("文件总大小：%I64X\t已传输：%I64X\t文件流总大小：%I64X\t已传输：%I64X\t流编号：%d\t\n"), TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart, StreamSize.QuadPart, StreamBytesTransferred.QuadPart, dwStreamNumber);
    SendMessage(hwndEdit, EM_SETSEL, -1, -1);
    SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

    // 继续复制操作
    return PROGRESS_CONTINUE;
}