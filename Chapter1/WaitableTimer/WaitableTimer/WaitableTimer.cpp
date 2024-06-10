#include <windows.h>
#include <Commctrl.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ȫ�ֱ���
HWND g_hwndDlg;
HANDLE g_hTimer;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SYSTEMTIME st = { 0 };
    FILETIME ftLocal, ftUTC;
    LARGE_INTEGER li;
    HANDLE hThread = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // ����һ���Զ����ÿɵȴ���ʱ������
        g_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

        hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
        if (hThread != NULL)
            CloseHandle(hThread);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_SET:
        case IDC_BTN_RESET:
           // �Զ���һ��ʱ��
            /*st.wYear = 2019;
            st.wMonth = 8;
            st.wDay = 5;
            st.wHour = 17;
            st.wMinute = 45;
            st.wSecond = 0;
            st.wMilliseconds = 0;*/

            // ��ȡ����ʱ��ؼ���ʱ��
            SendDlgItemMessage(hwndDlg, IDC_DATETIMEPICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
            // ϵͳʱ��ת����FILETIMEʱ��
            SystemTimeToFileTime(&st, &ftLocal);
            // ����FILETIMEʱ��ת����UTC��FILETIMEʱ��
            LocalFileTimeToFileTime(&ftLocal, &ftUTC);
            // ��Ҫ��ָ��FILETIME�ṹ��ָ��ǿ��ת��ΪLARGE_INTEGER *��__int64 *���ͣ�
            li.LowPart = ftUTC.dwLowDateTime;
            li.HighPart = ftUTC.dwHighDateTime;
            // ���ÿɵȴ���ʱ��
            SetWaitableTimer(g_hTimer, &li, 10 * 1000, NULL, NULL, FALSE);
            break;

        case IDC_BTN_CANCEL:
            // ȡ���ɵȴ���ʱ��
            CancelWaitableTimer(g_hTimer);
            break;

        case IDCANCEL:
            // �رտɵȴ���ʱ��������
            CloseHandle(g_hTimer);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    while (TRUE)
    {
        // �ȴ��ɵȴ���ʱ��
        WaitForSingleObject(g_hTimer, INFINITE);
        ShellExecute(NULL, TEXT("open"), TEXT("Calc.exe"), NULL, NULL, SW_SHOW);
    }

    return 0;
}