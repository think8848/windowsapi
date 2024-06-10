#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// �Զ�����Ϣ�����ڼ����߳�����ʾ�̷߳�����Ϣ���湤������(���������ǹ����߳�)
#define WM_WORKPROGRESS (WM_APP + 1)
// �Զ�����Ϣ�������̷߳�����Ϣ�����̸߳�֪���������
#define WM_CALCOVER     (WM_APP + 2)

// ȫ�ֱ���
HWND g_hwndDlg;
BOOL g_bRuning;         // �����߳�û����Ϣѭ�������߳�ͨ���������־��ΪFALSE֪ͨ����ֹ�߳�

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �̺߳�������
DWORD WINAPI ThreadProcShow(LPVOID lpParameter);    // ����ֵ��ʾ���༭�ؼ���
DWORD WINAPI ThreadProcCalc(LPVOID lpParameter);    // ģ��ִ��һ�����񣬶�ʱ��һ������1

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hThreadShow, hThreadCalc;
    static DWORD dwThreadIdShow;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        // ����ֹͣ��ť
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START:
            g_bRuning = TRUE;
            // ������ʾ�̺߳ͼ����߳�
            hThreadShow = CreateThread(NULL, 0, ThreadProcShow, NULL, 0, &dwThreadIdShow);
            hThreadCalc = CreateThread(NULL, 0, ThreadProcCalc, (LPVOID)dwThreadIdShow, 0, NULL);

            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), TRUE);
            break;

        case IDC_BTN_STOP:
            // ֪ͨ�����߳��˳�
            g_bRuning = FALSE;
            // ֪ͨ��ʾ�߳��˳�
            PostThreadMessage(dwThreadIdShow, WM_QUIT, 0, 0);

            if (hThreadShow != NULL)
            {
                CloseHandle(hThreadShow);
                hThreadShow = NULL;
            }
            if (hThreadCalc != NULL)
            {
                CloseHandle(hThreadCalc);
                hThreadCalc = NULL;
            }
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CALCOVER:
        if (hThreadShow != NULL)
        {
            CloseHandle(hThreadShow);
            hThreadShow = NULL;
        }
        if (hThreadCalc != NULL)
        {
            CloseHandle(hThreadCalc);
            hThreadCalc = NULL;
        }
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), TRUE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);

        MessageBox(hwndDlg, TEXT("�����̹߳��������"), TEXT("��ʾ"), MB_OK);
        return TRUE;
    }

    return FALSE;
}

DWORD WINAPI ThreadProcShow(LPVOID lpParameter)
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        switch (msg.message)
        {
        case WM_WORKPROGRESS:
            SetDlgItemInt(g_hwndDlg, IDC_EDIT_COUNT, (UINT)msg.wParam, FALSE);
            break;
        }
    }

    return msg.wParam;
}

DWORD WINAPI ThreadProcCalc(LPVOID lpParameter)
{
    // lpParameter�����Ǵ��ݹ�������ʾ�߳�ID
    DWORD dwThreadIdShow = (DWORD)lpParameter;
    int nCount = 0;

    while (g_bRuning)
    {
        PostThreadMessage(dwThreadIdShow, WM_WORKPROGRESS, nCount++, NULL);
        Sleep(50);

        // nCount����100��˵���������
        if (nCount > 100)
        {
            // ֪ͨ��ʾ�߳��˳�
            PostThreadMessage(dwThreadIdShow, WM_QUIT, 0, 0);

            // ������Ϣ�����̸߳�֪���������
            PostMessage(g_hwndDlg, WM_CALCOVER, 0, 0);

            // �������߳�Ҳ�˳�
            g_bRuning = FALSE;
            break;
        }
    }

    return 0;
}