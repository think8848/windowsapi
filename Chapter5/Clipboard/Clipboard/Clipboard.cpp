#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ����CF_TCHAR�����ܳ�����Unicode����ANSI�汾���������ı����ݸ�ʽ���ᱻ��ȷ����
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
HWND g_hwndList;        // �б�򴰿ھ��
UINT g_uFormat;         // ע����������ݸ�ʽ

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID OnInit(HWND hwndDlg);          // WM_INITDIALOG

VOID OnBtnText();                   // д���ı���ť����
VOID OnBtnBitmap();                 // д��λͼ��ť����
VOID OnBtnCustom();                 // д���Զ������ݰ�ť����
VOID OnBtnDelay();                  // �ӳ��ύ��ť����
VOID OnBtnMultiple();               // д��������ݰ�ť����

VOID OnRenderFormat(UINT uFormat);  // WM_RENDERFORMAT
VOID OnDestroyClipbord();           // WM_DESTROYCLIPBOARD
VOID OnRenderAllFormats();          // WM_RENDERALLFORMATS


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

    case WM_RENDERFORMAT:
        OnRenderFormat(wParam);
        return FALSE;

    case WM_DESTROYCLIPBOARD:
        OnDestroyClipbord();
        return FALSE;

    case WM_RENDERALLFORMATS:
        OnRenderAllFormats();
        return FALSE;
    }

    return FALSE;
}

/****************************************************************/
VOID OnInit(HWND hwndDlg)          // WM_INITDIALOG
{
    g_hwnd = hwndDlg;
    g_hwndList = GetDlgItem(hwndDlg, IDC_LIST_MSG);

    // ע��һ���Զ���������ʽ
    g_uFormat = RegisterClipboardFormat(TEXT("RegisterFormat"));
}

VOID OnBtnText()                   // д���ı���ť����
{
    TCHAR szText[128] = { 0 };
    LPTSTR lpStr;

    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ��ռ�����
    EmptyClipboard();
    // ��ȡ�ı����ݱ༭����ı�
    GetDlgItemText(g_hwnd, IDC_EDIT_TEXT, szText, _countof(szText));
    // �����ڴ�
    lpStr = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        (_tcslen(szText) + 1) * sizeof(TCHAR));
    if (lpStr)
    {
        // �����ı����ݵ��ոշ�����ڴ���
        StringCchCopy(lpStr, _tcslen(szText) + 1, szText);
        // ���ڴ��е����ݷ��õ�������
        //SetClipboardData(CF_TEXT, lpStr);
        //SetClipboardData(CF_UNICODETEXT, lpStr);
        SetClipboardData(CF_TCHAR, lpStr);
    }

    // �رռ�����
    CloseClipboard();
}

VOID OnBtnBitmap()                 // д��λͼ��ť����
{
    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ��ռ�����
    EmptyClipboard();

    // ���ü�����λͼ����
    HDC hdcDesk, hdcMem;
    HBITMAP hBmp;
    int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);

    hdcDesk = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hdcMem = CreateCompatibleDC(hdcDesk);
    hBmp = CreateCompatibleBitmap(hdcDesk, nWidth, nHeight);
    SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcDesk, 0, 0, SRCCOPY);
    SetClipboardData(CF_BITMAP, hBmp);

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    DeleteDC(hdcDesk);

    // �رռ�����
    CloseClipboard();
}

VOID OnBtnCustom()                 // д���Զ������ݰ�ť����
{
    LPVOID lpMem;
    // �Զ�������
    CUSTOM_DATA customData = { TEXT("����"), 40 };

    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ��ռ�����
    EmptyClipboard();
    // �����ڴ�
    lpMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CUSTOM_DATA));
    if (lpMem)
    {
        // �����Զ������ݵ��ոշ�����ڴ���
        memcpy_s(lpMem, sizeof(CUSTOM_DATA), &customData, sizeof(CUSTOM_DATA));
        // ���ü������Զ�������
        SetClipboardData(g_uFormat, lpMem);
    }

    // �رռ�����
    CloseClipboard();
}

VOID OnBtnDelay()                  // �ӳ��ύ��ť����
{
    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ��ռ�����
    EmptyClipboard();

    // ���ü������ӳ��ύλͼ����
    SetClipboardData(CF_BITMAP, NULL);

    // �رռ�����
    CloseClipboard();
}

VOID OnBtnMultiple()               // д��������ݰ�ť����
{
    // �򿪼�����
    OpenClipboard(g_hwnd);
    // ��ռ�����
    EmptyClipboard();

    // �ı�����
    TCHAR szText[128] = { 0 };
    LPTSTR lpStr;
    GetDlgItemText(g_hwnd, IDC_EDIT_TEXT, szText, _countof(szText));
    lpStr = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        (_tcslen(szText) + 1) * sizeof(TCHAR));
    if (lpStr)
    {
        StringCchCopy(lpStr, _tcslen(szText) + 1, szText);
        SetClipboardData(CF_TCHAR, lpStr);
    }

    // λͼ����
    HDC hdcDesk, hdcMem;
    HBITMAP hBmp;
    int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);
    hdcDesk = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hdcMem = CreateCompatibleDC(hdcDesk);
    hBmp = CreateCompatibleBitmap(hdcDesk, nWidth, nHeight);
    SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcDesk, 0, 0, SRCCOPY);
    SetClipboardData(CF_BITMAP, hBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    DeleteDC(hdcDesk);

    // �Զ�������
    LPVOID lpMem;
    CUSTOM_DATA customData = { TEXT("����"), 40 };
    lpMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CUSTOM_DATA));
    if (lpMem)
    {
        memcpy_s(lpMem, sizeof(CUSTOM_DATA), &customData, sizeof(CUSTOM_DATA));
        SetClipboardData(g_uFormat, lpMem);
    }

    // �رռ�����
    CloseClipboard();
}


VOID OnRenderFormat(UINT uFormat)  // WM_RENDERFORMAT
{
    SendMessage(g_hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("WM_RENDERFORMAT"));

    // ���ü�����λͼ����
    HDC hdcDesk, hdcMem;
    HBITMAP hBmp;
    int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);
    hdcDesk = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hdcMem = CreateCompatibleDC(hdcDesk);
    hBmp = CreateCompatibleBitmap(hdcDesk, nWidth, nHeight);
    SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcDesk, 0, 0, SRCCOPY);
    SetClipboardData(uFormat, hBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    DeleteDC(hdcDesk);
}

VOID OnDestroyClipbord()           // WM_DESTROYCLIPBOARD
{
    SendMessage(g_hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("WM_DESTROYCLIPBOARD"));
}

VOID OnRenderAllFormats()          // WM_RENDERALLFORMATS
{
    SendMessage(g_hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("WM_RENDERALLFORMATS"));
    OnBtnBitmap();
}