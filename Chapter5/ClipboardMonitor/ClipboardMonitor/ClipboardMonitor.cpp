#include <Windows.h>
#include <Richedit.h>
#include "resource.h"

#ifdef UNICODE
    #define CF_TCHAR CF_UNICODETEXT
#else
    #define CF_TCHAR CF_TEXT
#endif

// ȫ�ֱ���
HWND g_hwndNextViewer;      // ������鿴��������һ��������鿴���Ĵ��ھ��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ����Riched20.dll��̬���ӿ�
    LoadLibrary(TEXT("Riched20.dll"));

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    LONG lStyle;
    LPTSTR lpStr;
    HBITMAP hBmp;
    UINT uFormat;
    TCHAR szSeparator[] = TEXT("\n--------------------------------------------------------\n");

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // ��ʾͼƬ�ľ�̬�ؼ�����SS_BITMAP | SS_REALSIZECONTROL���
        lStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATIC_BITMAP), GWL_STYLE);
        SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATIC_BITMAP), GWL_STYLE,
            lStyle | SS_BITMAP | SS_REALSIZECONTROL);

        // ����ǰ���򴰿���ӵ�������鿴��������
        g_hwndNextViewer = SetClipboardViewer(hwndDlg);

        hwndEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            // �Ӽ�����鿴������ɾ������
            ChangeClipboardChain(hwndDlg, g_hwndNextViewer);
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        return TRUE;

    case WM_DRAWCLIPBOARD:
        // ����������ݷ����仯
        // ���������鿴�����д�����һ������
        if (g_hwndNextViewer)
            SendMessage(g_hwndNextViewer, uMsg, wParam, lParam);

        // ������ʾ
        OpenClipboard(hwndDlg);
        uFormat = EnumClipboardFormats(0);
        while (uFormat)
        {
            // ����ֻ������2�ָ�ʽ
            if (uFormat == CF_TCHAR)
            {
                lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)lpStr);
                SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szSeparator);
            }
            else if (uFormat == CF_BITMAP)
            {
                hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
                SendDlgItemMessage(hwndDlg, IDC_STATIC_BITMAP, STM_SETIMAGE,
                    IMAGE_BITMAP, (LPARAM)hBmp);
            }

            uFormat = EnumClipboardFormats(uFormat);
        }
        CloseClipboard();
        return TRUE;

    case WM_CHANGECBCHAIN:
        // ����WM_CHANGECBCHAIN��Ϣ��ά���ü�����鿴����
        if ((HWND)wParam == g_hwndNextViewer)
            g_hwndNextViewer = (HWND)lParam;
        else if (g_hwndNextViewer)
            SendMessage(g_hwndNextViewer, uMsg, wParam, lParam);
        return TRUE;
    }

    return FALSE;
}