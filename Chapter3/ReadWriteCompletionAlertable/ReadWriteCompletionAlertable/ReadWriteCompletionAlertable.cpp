#include <windows.h>
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 传递给完成例程的自定义数据结构
typedef struct _IOData
{
    HANDLE m_hFileSource; // 源文件句柄
    HANDLE m_hFileDest;   // 目标文件句柄
    HANDLE m_hFileMap;    // 源文件映射对象
    LPVOID m_lpMemory;    // 映射的虚拟地址
}IOData, * PIOData;

// 全局变量
HINSTANCE g_hInstance;
HWND g_hwndDlg;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 完成例程
VOID WINAPI OverlappedCompletionRoutine(
    DWORD dwErrorCode,              // I/O请求的状态代码
    DWORD dwNumberOfBytesTransfered,// 已传输的字节数
    LPOVERLAPPED lpOverlapped);     // 当初调用I/O函数时指定的那个OVERLAPPED结构的指针

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
    ofnSource.nFilterIndex = 1;                          // 默认选择第1个过滤器
    ofnSource.lpstrFile = szFileNameSource;              // 返回用户选择的文件名的缓冲区
    ofnSource.lpstrFile[0] = NULL;                       // 不需要初始化文件名编辑控件
    ofnSource.nMaxFile = _countof(szFileNameSource);
    ofnSource.lpstrTitle = TEXT("请选择要打开的文件");   // 对话框标题栏中显示的字符串
    ofnSource.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

    OPENFILENAME ofnDest = { 0 };
    ofnDest.lStructSize = sizeof(ofnDest);
    ofnDest.hwndOwner = hwndDlg;
    ofnDest.lpstrFilter = TEXT("All(*.*)\0*.*\0");
    ofnDest.nFilterIndex = 1;                           // 默认选择第1个过滤器
    ofnDest.lpstrFile = szFileNameDest;                 // 返回用户选择的文件名的缓冲区
    ofnDest.lpstrFile[0] = NULL;                        // 不需要初始化文件名编辑控件
    ofnDest.nMaxFile = _countof(szFileNameDest);
    ofnDest.lpstrTitle = TEXT("请选择要保存的文件名");  // 对话框标题栏中显示的字符串
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
            // 打开源文件
            GetDlgItemText(hwndDlg, IDC_EDIT_SOURCE, szFileNameSource, _countof(szFileNameSource));
            hFileSource = CreateFile(szFileNameSource, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (hFileSource == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("打开源文件失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 获取源文件大小
            dwFileSizeLow = GetFileSize(hFileSource, &dwFileSizeHigh);

            // 为hFile文件对象创建一个文件映射内核对象
            hFileMap = CreateFileMapping(hFileSource, NULL, PAGE_READWRITE, 0, 0, NULL);
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

            // 创建目标文件
            GetDlgItemText(hwndDlg, IDC_EDIT_DEST, szFileNameDest, _countof(szFileNameDest));
            hFileDest = CreateFile(szFileNameDest, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (hFileDest == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("创建目标文件失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 分配一个IOData结构，把相关句柄传递过去以释放
            pIOData = new IOData;
            pIOData->m_hFileSource = hFileSource;
            pIOData->m_hFileDest = hFileDest;
            pIOData->m_hFileMap = hFileMap;
            pIOData->m_lpMemory = lpMemory;

            // 异步写入目标文件
            lpOverlapped = new OVERLAPPED;
            ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));
            lpOverlapped->Offset = 0;
            lpOverlapped->OffsetHigh = 0;
            lpOverlapped->hEvent = pIOData; // 通过hEvent字段传递给完成例程一个自定义数据结构
            bRet = WriteFileEx(hFileDest, lpMemory, dwFileSizeLow, lpOverlapped, OverlappedCompletionRoutine);

            // 可以接着去做一些其他事情，这里播放一首音乐
            PlaySound(TEXT("爱是一缕寂寞的愁.wav"), NULL, SND_FILENAME | SND_ASYNC/* | SND_LOOP*/);
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

    MessageBox(g_hwndDlg, TEXT("写入目标文件完成"), TEXT("提示"), MB_OK);

    // 清理工作
    UnmapViewOfFile(pIOData->m_lpMemory);
    CloseHandle(pIOData->m_hFileMap);
    CloseHandle(pIOData->m_hFileSource);
    CloseHandle(pIOData->m_hFileDest);

    delete pIOData;
    delete lpOverlapped;

    return;
}