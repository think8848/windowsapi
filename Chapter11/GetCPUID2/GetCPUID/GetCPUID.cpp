#include <windows.h>
#include <intrin.h>
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
    int arrCpuInfo[4] = { 0 };
    TCHAR szBuf[32] = { 0 };

    // GetCPUID函数字节码
    BYTE bGetCPUID[] = {
        0x4C, 0x8B, 0xC1,       // mov r8, rcx
        0x8B, 0xC2,             // mov eax, edx
        0x0F, 0xA2,             // cpuid
        0x41, 0x89, 0x00,       // mov dword ptr [r8], eax
        0x41, 0x89, 0x50, 0x0C, // mov dword ptr [r8 + 0xC], edx
        0xC3 };                 // ret
    // 函数指针变量
    VOID(*GetCPUID)(int cpuInfo[4], int function_id) = (VOID(*)(int*, int))(ULONG_PTR)bGetCPUID;
    DWORD dwOldProtect;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_GET:
            // intrinsic函数
            __cpuid(arrCpuInfo, 1);
            wsprintf(szBuf, TEXT("%08X%08X"), arrCpuInfo[3], arrCpuInfo[0]);
            SetDlgItemText(hwndDlg, IDC_EDIT_INTRINSIC, szBuf);

            // 自定义汇编函数
            ZeroMemory(arrCpuInfo, sizeof(arrCpuInfo));

            VirtualProtect(GetCPUID, sizeof(bGetCPUID), PAGE_EXECUTE_READWRITE, &dwOldProtect);
            GetCPUID(arrCpuInfo, 1);
            VirtualProtect(GetCPUID, sizeof(bGetCPUID), dwOldProtect, &dwOldProtect);

            wsprintf(szBuf, TEXT("%08X%08X"), arrCpuInfo[3], arrCpuInfo[0]);
            SetDlgItemText(hwndDlg, IDC_EDIT_ASM, szBuf);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}