#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ����CF_TCHAR�����ܳ�����Unicode����ANSI�汾���������ı���ʽ���ݶ��ܱ���ȷ��ȡ
#ifdef UNICODE
    #define CF_TCHAR CF_UNICODETEXT
#else
    #define CF_TCHAR CF_TEXT
#endif

// �Զ�����������ݸ�ʽ
typedef struct _CUSTOM_DATA
{
    // ����洢����һ���˵�����������
    TCHAR szName[128];
    UINT uAge;
}CUSTOM_DATA, * PCUSTOM_DATA;

// ȫ�ֱ���
HINSTANCE g_hInstance;  // ʵ�����
HWND g_hwnd;            // ���ھ��
UINT g_uFormat;         // ע����������ݸ�ʽ

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID OnInit(HWND hwndDlg);
VOID OnBtnText();                   // ��ȡ�ı����ݰ�ť����
VOID OnBtnBitmap();                 // ��ȡλͼ��ť����
VOID OnBtnCustom();                 // ��ȡ�Զ������ݰ�ť����
VOID OnBtnDelay();                  // ��ȡ�ӳ��ύ��ť����
VOID OnBtnMultiple();               // ��ȡ�������ݰ�ť����

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        OnInit(hwndDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_TEXT:
            OnBtnText();
            break;

        case IDC_BTN_BITMAP:
            OnBtnBitmap();
            break;

        case IDC_BTN_CUSTOM:
            OnBtnCustom();
            break;

        case IDC_BTN_DELAY:
            OnBtnDelay();
            break;

        case IDC_BTN_MULTIPLE:
            OnBtnMultiple();
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

/****************************************************************/
VOID OnInit(HWND hwndDlg)
{
    g_hwnd = hwndDlg;
    // ע��һ���Զ���������ʽ����д���ʹ����ͬ�����ݸ�ʽ���ƣ���˷�����ͬ�ĸ�ʽ��ʶֵ
    g_uFormat = RegisterClipboardFormat(TEXT("RegisterFormat"));

    // ��ʾͼƬ�ľ�̬�ؼ�����SS_BITMAP | SS_REALSIZECONTROL��ʽ
    LONG lStyle = GetWindowLongPtr(GetDlgItem(g_hwnd, IDC_STATIC_BITMAP), GWL_STYLE);
    SetWindowLongPtr(GetDlgItem(g_hwnd, IDC_STATIC_BITMAP), GWL_STYLE,
        lStyle | SS_BITMAP | SS_REALSIZECONTROL);
}

VOID OnBtnText()                   // ��ȡ�ı����ݰ�ť����
{
    LPTSTR lpStr;

    // �������Ƿ����CF_TCHAR��ʽ���ı�
    if (IsClipboardFormatAvailable(CF_TCHAR))
    {
        // �򿪼�����
        OpenClipboard(g_hwnd);
        // ��ȡ���������ı���ʽ�����ݣ������Ƶ�szText������
        lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
        // ��ʾ���༭����
        SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, lpStr);

        // �رռ�����
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("������û���ı���ʽ�����ݣ�"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnBitmap()                 // ��ȡλͼ��ť����
{
    // �������Ƿ����CF_BITMAP��ʽ������
    if (IsClipboardFormatAvailable(CF_BITMAP))
    {
        // �򿪼�����
        OpenClipboard(g_hwnd);
        // ��ȡ��������λͼ��ʽ������
        HBITMAP hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
        // ��ʾͼƬ
        SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

        // �رռ�����
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("������û��λͼ��ʽ�����ݣ�"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnCustom()                 // ��ȡ�Զ������ݰ�ť����
{
    TCHAR szBuf[128] = { 0 };
    PCUSTOM_DATA pCustomData;

    // �������Ƿ����g_uFormat��ʽ������
    if (IsClipboardFormatAvailable(g_uFormat))
    {
        // �򿪼�����
        OpenClipboard(g_hwnd);
        // ��ȡ��������g_uFormat��ʽ������
        pCustomData = (PCUSTOM_DATA)GetClipboardData(g_uFormat);
        wsprintf(szBuf, TEXT("%s, %d"), pCustomData->szName, pCustomData->uAge);
        // ��ʾ���༭����
        SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, szBuf);

        // �رռ�����
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("������û���Զ����ʽ�����ݣ�"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnDelay()                  // ��ȡ�ӳ��ύ��ť����
{
    // �������ȡ�ӳ��ύʵ���϶�ȡ��Ҳ��λͼ������ֱ�ӵ���OnBtnBitmap
    OnBtnBitmap();
}

VOID OnBtnMultiple()                    // ��ȡ�������ݰ�ť����
{
    // ����ı����ݱ༭���Զ������ݱ༭���ͼ��̬�ؼ�������
    SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, TEXT(""));
    SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, TEXT(""));
    SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ö�ټ������ϵ�ǰ���õ����ݸ�ʽ
    UINT uFormat = EnumClipboardFormats(0);
    while (uFormat)
    {
        // ����ֻ������3�ָ�ʽ
        if (uFormat == CF_TCHAR)
        {
            LPTSTR lpStr;

            // ��ȡ���������ı���ʽ�����ݣ������Ƶ�szText������
            lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
            // ��ʾ���༭����
            SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, lpStr);
        }
        else if (uFormat == CF_BITMAP)
        {
            // ��ȡ��������λͼ��ʽ������
            HBITMAP hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
            // ��ʾͼƬ
            SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
        }
        else if (uFormat == g_uFormat)
        {
            TCHAR szBuf[128] = { 0 };
            PCUSTOM_DATA pCustomData;

            // ��ȡ��������g_uFormat��ʽ������
            pCustomData = (PCUSTOM_DATA)GetClipboardData(g_uFormat);
            wsprintf(szBuf, TEXT("%s, %d"), pCustomData->szName, pCustomData->uAge);
            // ��ʾ���༭����
            SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, szBuf);
        }

        uFormat = EnumClipboardFormats(uFormat);
    }

    // �رռ�����
    CloseClipboard();
}