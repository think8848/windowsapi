#include <windows.h>
#include "resource.h"
#include "DIPSHookDll.h"

#pragma comment(lib, "DIPSHookDll.lib")

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndLV;
    HWND hwndDIPSServer;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndLV = GetTopWindow(GetTopWindow(FindWindow(TEXT("ProgMan"), NULL)));
        // ���ñ�������ͼ�ꡢ�ָ�����ͼ���ж����Ϣ���Ӱ�ť
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_INSTALLHOOK:
            InstallHook(WH_GETMESSAGE, GetWindowThreadProcessId(hwndLV, NULL));
            // ���ñ�������ͼ�ꡢ�ָ�����ͼ���ж����Ϣ���Ӱ�ť
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), TRUE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), TRUE);
            MessageBox(hwndDlg, TEXT("��װ��Ϣ���ӳɹ�"), TEXT("�ɹ�"), MB_OK);
            break;

        case IDC_BTN_UNINSTALLHOOK:
            // ��ȡ���������ھ��
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            // ʹ��SendMessage������PostMessage��ȷ������ж����ǰ���������Ի����Ѿ�����
            SendMessage(hwndDIPSServer, WM_CLOSE, 0, 0);
            UninstallHook();
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SAVE), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESTORE), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNINSTALLHOOK), FALSE);
            MessageBox(hwndDlg, TEXT("ж����Ϣ���ӳɹ�"), TEXT("�ɹ�"), MB_OK);
            break;

        case IDC_BTN_SAVE:
            // ��ȡ���������ھ��
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            SendMessage(hwndDIPSServer, WM_APP, (WPARAM)hwndLV, TRUE);
            MessageBox(hwndDlg, TEXT("��������ͼ��ɹ�"), TEXT("�ɹ�"), MB_OK);
            break;

        case IDC_BTN_RESTORE:
            // ��ȡ���������ھ��
            hwndDIPSServer = FindWindow(NULL, TEXT("DIPSServer"));
            SendMessage(hwndDIPSServer, WM_APP, (WPARAM)hwndLV, FALSE);
            MessageBox(hwndDlg, TEXT("�ָ�����ͼ��ɹ�"), TEXT("�ɹ�"), MB_OK);
            break;

        case IDCANCEL:
            if (FindWindow(NULL, TEXT("DIPSServer")))
                SendMessage(hwndDlg, WM_COMMAND, IDC_BTN_UNINSTALLHOOK, 0);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}