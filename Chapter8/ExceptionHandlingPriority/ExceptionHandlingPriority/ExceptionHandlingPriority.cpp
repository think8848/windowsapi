#include <windows.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo);
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPVOID lpHandler;
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        lpHandler = AddVectoredExceptionHandler(1, VectoredHandler);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_OK:
            __try
            {
                PCHAR pChar;
                pChar = NULL;
                *pChar = TEXT('A');
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                MessageBox(NULL, TEXT("SEH�쳣�������"), TEXT("SEH��ʾ"), MB_OK);
            }
            break;

        case IDC_BTN_OK2:
            hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
            if (hThread)
                CloseHandle(hThread);

            break;

        case IDCANCEL:
            RemoveVectoredExceptionHandler(lpHandler);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    MessageBox(NULL, TEXT("VEH�쳣�������"), TEXT("VEH��ʾ"), MB_OK);

    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    __try
    {
        PCHAR pChar;
        pChar = NULL;
        *pChar = TEXT('A');
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        MessageBox(NULL, TEXT("���Ը����̣߳�SEH�쳣�������"), TEXT("SEH��ʾ"), MB_OK);
    }

    return 0;
}