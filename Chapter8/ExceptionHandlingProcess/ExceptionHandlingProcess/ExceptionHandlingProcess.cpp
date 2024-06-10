#include <windows.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LONG CALLBACK VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo);      // VEH�쳣�������
DWORD StructuredExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);             // SEH�쳣���˳���
LONG WINAPI TopLevelUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);// UEF�쳣���˳���
LONG CALLBACK VectoredContinueHandler(PEXCEPTION_POINTERS ExceptionInfo);       // VCH�����������
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

        // VEH�������쳣����
        lpHandler = AddVectoredExceptionHandler(1, VectoredExceptionHandler);
        // VCH��������������
        lpHandlerContinue = AddVectoredContinueHandler(1, VectoredContinueHandler);
        // UEF����δ�����쳣������
        SetUnhandledExceptionFilter(TopLevelUnhandledExceptionFilter);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            __try
            {
                // �ᷢ��EXCEPTION_INT_DIVIDE_BY_ZERO�����쳣
                x = n / m;
                wsprintf(szBuf, TEXT("%d / %d = %d"), n, m, x);
                MessageBox(hwndDlg, szBuf, TEXT("�Ѵ��쳣�лָ�"), MB_OK);
            }
            __except (StructuredExceptionFilter(GetExceptionInformation()))
            {
                // ����SEH�쳣���˳��򷵻�EXCEPTION_EXECUTE_HANDLER���������ﲻ��ִ��
                MessageBox(hwndDlg, TEXT("SEH�쳣�������"), TEXT("SEH��ʾ"), MB_OK);
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
    MessageBox(g_hwndDlg, TEXT("VEH�쳣�������"), TEXT("VEH��ʾ"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // �ѳ�������Ϊ2
    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD StructuredExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("SEH�쳣���˳���"), TEXT("SEH��ʾ"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // �ѳ�������Ϊ2
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI TopLevelUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("UEF����δ�����쳣����������"), TEXT("UEF��ʾ"), MB_OK);

    //ExceptionInfo->ContextRecord->Ecx = 2;  // �ѳ�������Ϊ2
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG CALLBACK VectoredContinueHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(g_hwndDlg, TEXT("VCH�����������"), TEXT("VCH��ʾ"), MB_OK);

    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int n = 10, m = 0, x;
    TCHAR szBuf[32] = { 0 };

    __try
    {
        // �ᷢ��EXCEPTION_INT_DIVIDE_BY_ZERO�����쳣
        x = n / m;
        wsprintf(szBuf, TEXT("%d / %d = %d"), n, m, x);
        MessageBox(g_hwndDlg, szBuf, TEXT("�Ѵ��쳣�лָ�"), MB_OK);
    }
    __except (StructuredExceptionFilter(GetExceptionInformation()))
    {
        // ����SEH�쳣���˳��򷵻�EXCEPTION_EXECUTE_HANDLER���������ﲻ��ִ��
        MessageBox(g_hwndDlg, TEXT("���Ը����̣߳�SEH�쳣�������"), TEXT("SEH��ʾ"), MB_OK);
    }

    return 0;
}