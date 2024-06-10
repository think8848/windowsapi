#include <windows.h>
#include <intrin.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 声明引用外部函数
EXTERN_C VOID GetCPUID(int cpuInfo[4], int function_id);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int arrCpuInfo[4] = { 0 };
    TCHAR szBuf[32] = { 0 };

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
            GetCPUID(arrCpuInfo, 1);
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