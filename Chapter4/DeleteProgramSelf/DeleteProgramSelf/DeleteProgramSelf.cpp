#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
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
    CHAR szApplicationName[MAX_PATH] = { 0 };               // 本程序文件路径
    TCHAR szCmdPath[MAX_PATH] = { 0 };                      // cmd.exe文件路径
    TCHAR szBatFilePath[MAX_PATH];                          // .bat文件路径
    TCHAR szBatFileName[MAX_PATH] = TEXT("删除程序.bat");   // .bat文件名称

    CHAR szBatFileContent[MAX_PATH * 3] = { 0 };            // .bat文件内容
    CHAR szBatFileContentFormat[MAX_PATH] = 
            { "@ping 127.0.0.1 -n 5 >nul\r\ndel \"%s\"\r\ndel %%0" };
    HANDLE hFile;

    TCHAR szCommandLine[MAX_PATH] = TEXT("/c ");            // CreateProcess的lpCommandLine参数
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = { 0 };

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            // 本程序文件路径
            GetModuleFileNameA(NULL, szApplicationName, _countof(szApplicationName));

            // cmd.exe文件路径
            GetEnvironmentVariable(TEXT("ComSpec"), szCmdPath, _countof(szCmdPath));

            // .bat文件路径，放到系统临时目录
            GetTempPath(_countof(szBatFilePath), szBatFilePath);
            if (szBatFilePath[_tcslen(szBatFilePath) - 1] != TEXT('\\'))
               StringCchCat(szBatFilePath, _countof(szBatFilePath), TEXT("\\"));
            StringCchCat(szBatFilePath, _countof(szBatFilePath), szBatFileName);

            // 创建.bat文件，写入内容
            hFile = CreateFile(szBatFilePath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            wsprintfA(szBatFileContent, szBatFileContentFormat, szApplicationName);
            WriteFile(hFile, szBatFileContent, strlen(szBatFileContent), NULL, NULL);
            CloseHandle(hFile);

            // 创建进程，执行.bat批处理文件
            StringCchCat(szCommandLine, _countof(szCommandLine), szBatFilePath);
            if (CreateProcess(szCmdPath, szCommandLine, NULL, NULL, FALSE,
                CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }

            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}