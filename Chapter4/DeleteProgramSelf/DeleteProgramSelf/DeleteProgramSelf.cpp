#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
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
    CHAR szApplicationName[MAX_PATH] = { 0 };               // �������ļ�·��
    TCHAR szCmdPath[MAX_PATH] = { 0 };                      // cmd.exe�ļ�·��
    TCHAR szBatFilePath[MAX_PATH];                          // .bat�ļ�·��
    TCHAR szBatFileName[MAX_PATH] = TEXT("ɾ������.bat");   // .bat�ļ�����

    CHAR szBatFileContent[MAX_PATH * 3] = { 0 };            // .bat�ļ�����
    CHAR szBatFileContentFormat[MAX_PATH] = 
            { "@ping 127.0.0.1 -n 5 >nul\r\ndel \"%s\"\r\ndel %%0" };
    HANDLE hFile;

    TCHAR szCommandLine[MAX_PATH] = TEXT("/c ");            // CreateProcess��lpCommandLine����
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
            // �������ļ�·��
            GetModuleFileNameA(NULL, szApplicationName, _countof(szApplicationName));

            // cmd.exe�ļ�·��
            GetEnvironmentVariable(TEXT("ComSpec"), szCmdPath, _countof(szCmdPath));

            // .bat�ļ�·�����ŵ�ϵͳ��ʱĿ¼
            GetTempPath(_countof(szBatFilePath), szBatFilePath);
            if (szBatFilePath[_tcslen(szBatFilePath) - 1] != TEXT('\\'))
               StringCchCat(szBatFilePath, _countof(szBatFilePath), TEXT("\\"));
            StringCchCat(szBatFilePath, _countof(szBatFilePath), szBatFileName);

            // ����.bat�ļ���д������
            hFile = CreateFile(szBatFilePath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            wsprintfA(szBatFileContent, szBatFileContentFormat, szApplicationName);
            WriteFile(hFile, szBatFileContent, strlen(szBatFileContent), NULL, NULL);
            CloseHandle(hFile);

            // �������̣�ִ��.bat�������ļ�
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