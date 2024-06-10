#include <windows.h>
#include <winternl.h>
#include "resource.h"

#pragma comment(lib, "Ntdll.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 进程伪装
BOOL DisguiseProcess(HANDLE hProcess, LPCWSTR lpImagePathName, LPCWSTR lpCommandLine);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            DisguiseProcess(GetCurrentProcess(), L"C:\\Windows\\Explorer.exe", L"Explorer.exe");
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL DisguiseProcess(HANDLE hProcess, LPCWSTR lpImagePathName, LPCWSTR lpCommandLine)
{
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    PEB peb = { 0 };
    RTL_USER_PROCESS_PARAMETERS userProcessParam = { 0 };
    SIZE_T nImagePathName, nImagePathNameMax, nCommandLine, nCommandLineMax;

    // 获取指定进程的PROCESS_BASIC_INFORMATION结构
    NtQueryInformationProcess(hProcess, ProcessBasicInformation,
        &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);

    // 读取指定进程的进程环境块PEB
    ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(PEB), NULL);
    // 读取指定进程的RTL_USER_PROCESS_PARAMETERS结构
    ReadProcessMemory(hProcess, peb.ProcessParameters,
        &userProcessParam, sizeof(RTL_USER_PROCESS_PARAMETERS), NULL);

    // 修改指定进程的文件路径
    nImagePathName = wcslen(lpImagePathName) * 2; nImagePathNameMax = nImagePathName + 2;
    WriteProcessMemory(hProcess, userProcessParam.ImagePathName.Buffer, lpImagePathName, nImagePathNameMax, NULL);
    WriteProcessMemory(hProcess, &userProcessParam.ImagePathName.Length, &nImagePathName, sizeof(nImagePathName), NULL);
    WriteProcessMemory(hProcess, &userProcessParam.ImagePathName.MaximumLength, &nImagePathNameMax, sizeof(nImagePathNameMax), NULL);

    // 修改指定进程的命令行参数
    nCommandLine = wcslen(lpCommandLine) * 2; nCommandLineMax = nCommandLine + 2;
    WriteProcessMemory(hProcess, userProcessParam.CommandLine.Buffer, lpCommandLine, nCommandLineMax, NULL);
    WriteProcessMemory(hProcess, &userProcessParam.CommandLine.Length, &nCommandLine, sizeof(nCommandLine), NULL);
    WriteProcessMemory(hProcess, &userProcessParam.CommandLine.MaximumLength, &nCommandLineMax, sizeof(nCommandLineMax), NULL);

    return TRUE;
}