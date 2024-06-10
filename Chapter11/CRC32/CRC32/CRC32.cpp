#include <windows.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 常量定义
//#define Poly 0xEDB88320L                // CRC-32标准
// 生成CRC-32查询表
VOID GenerateCRC32Table(PUINT pCRC32Table);
// 计算CRC-32
UINT CRC32(LPBYTE lpData, UINT nSize);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 打开文件所用变量
    TCHAR szFile[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndDlg;
    ofn.lpstrFilter =
        TEXT("exe文件(*.exe)\0*.exe\0dll文件(*.dll)\0*.dll\0All(*.*)\0*.*\0");
    ofn.nFilterIndex = 3;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = NULL;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrTitle = TEXT("请选择要打开的PE文件");
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // 内存映射文件所用变量
    HANDLE hFile, hFileMap;
    LPVOID lpMemory;
    LARGE_INTEGER liFileSize;

    UINT nCRC;
    TCHAR szBuf[16] = { 0 };

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_BROWSE:
            if (GetOpenFileName(&ofn))
                SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile);
            break;

        case IDC_BTN_CALC:
            // 打开文件
            GetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile, _countof(szFile));
            hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
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
                    return TRUE;
                }
            }

            // 为hFile文件对象创建一个文件映射内核对象
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (!hFileMap)
            {
                MessageBox(hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
            lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
            if (!lpMemory)
            {
                MessageBox(hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
                return TRUE;
            }

            // 自定义函数计算CRC-32
            nCRC = CRC32((LPBYTE)lpMemory, liFileSize.LowPart);
            wsprintf(szBuf, TEXT("0x%08X"), nCRC);
            SetDlgItemText(hwndDlg, IDC_EDIT_RESULT, szBuf);

            // RtlComputeCrc32计算CRC-32
            typedef UINT(WINAPI* pfnRtlComputeCrc32)(INT dwInitial, LPVOID lpData, INT nLen);
            pfnRtlComputeCrc32 fnRtlComputeCrc32;
            fnRtlComputeCrc32 = (pfnRtlComputeCrc32)
                GetProcAddress(GetModuleHandle(TEXT("NtDll.dll")), "RtlComputeCrc32");
            nCRC = fnRtlComputeCrc32(0, lpMemory, liFileSize.LowPart);  // 第一个参数指定为0
            wsprintf(szBuf, TEXT("0x%08X"), nCRC);
            SetDlgItemText(hwndDlg, IDC_EDIT_RESULT2, szBuf);

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

#define Poly 0xEDB88320                 // CRC-32标准

VOID GenerateCRC32Table(PUINT pCRC32Table)
{
    UINT nCrc;

    for (UINT i = 0; i < 256; i++)
    {
        nCrc = i;
        for (int j = 0; j < 8; j++)
        {
            if (nCrc & 0x00000001)
                nCrc = (nCrc >> 1) ^ Poly;
            else
                nCrc = nCrc >> 1;
        }

        pCRC32Table[i] = nCrc;
    }
}

UINT CRC32(LPBYTE lpData, UINT nSize)
{
    UINT CRC32Table[256] = { 0 };       // CRC-32查询表
    UINT nCrc = 0xFFFFFFFF;

    // 生成CRC-32查询表
    GenerateCRC32Table(CRC32Table);

    // 计算CRC-32
    for (UINT i = 0; i < nSize; i++)
        nCrc = CRC32Table[(nCrc ^ lpData[i]) & 0xFF] ^ (nCrc >> 8);

    return nCrc ^ 0xFFFFFFFF;           // 按位取反
}