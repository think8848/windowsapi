#include <windows.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LONG CALLBACK VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo);      // VEH异常处理程序
DWORD StructuredExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);             // SEH异常过滤程序
LONG WINAPI TopLevelUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);// UEF异常过滤程序
LONG CALLBACK VectoredContinueHandler(PEXCEPTION_POINTERS ExceptionInfo);       // VCH继续处理程序
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPVOID lpHandler, lpHandlerContinue;
    HANDLE hThread = NULL;
    int n = 10, m = 0, x;
    TCHAR szBuf[32] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // VEH向量化异常处理
        lpHandler = AddVectoredExceptionHandler(1, VectoredExceptionHandler);
        // VCH向量化继续处理
        lpHandlerContinue = AddVectoredContinueHandler(1, VectoredContinueHandler);
        // UEF顶层未处理异常过滤器
        SetUnhandledExceptionFilter(TopLevelUnhandledExceptionFilter);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            __try
            {
                // 会发生EXCEPTION_INT_DIVIDE_BY_ZERO除零异常
                x = n / m;
                wsprintf(szBuf, TEXT("%d / %d = %d"), n, m, x);
                MessageBox(hwndDlg, szBuf, TEXT("已从异常中恢复"), MB_OK);
            }
            __except (StructuredExceptionFilter(GetExceptionInformation()))
            {
                // 除非SEH异常过滤程序返回EXCEPTION_EXECUTE_HANDLER，否则这里不会执行
                MessageBox(hwndDlg, TEXT("SEH异常处理程序"), TEXT("SEH提示"), MB_OK);
            }
            break;

        case IDC_BTN_OK2:
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread)
                CloseHandle(hThread);
            break;

        case IDCANCEL:
            RemoveVectoredExceptionHandler(lpHandler);
            RemoveVectoredContinueHandler(lpHandlerContinue);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

LONG CALLBACK VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("VEH异常处理程序"), TEXT("VEH提示"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // 把除数设置为2
    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD StructuredExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("SEH异常过滤程序"), TEXT("SEH提示"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // 把除数设置为2
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI TopLevelUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("UEF顶层未处理异常过滤器程序"), TEXT("UEF提示"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // 把除数设置为2
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG CALLBACK VectoredContinueHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("VCH继续处理程序"), TEXT("VCH提示"), MB_OK);

    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int n = 10, m = 0, x;
    TCHAR szBuf[32] = { 0 };

    __try
    {
        // 会发生EXCEPTION_INT_DIVIDE_BY_ZERO除零异常
        x = n / m;
        wsprintf(szBuf, TEXT("%d / %d = %d"), n, m, x);
        MessageBox(g_hwndDlg, szBuf, TEXT("已从异常中恢复"), MB_OK);
    }
    __except (StructuredExceptionFilter(GetExceptionInformation()))
    {
        // 除非SEH异常过滤程序返回EXCEPTION_EXECUTE_HANDLER，否则这里不会执行
        MessageBox(g_hwndDlg, TEXT("来自辅助线程：SEH异常处理程序"), TEXT("SEH提示"), MB_OK);
    }

    return 0;
}