#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 定义CF_TCHAR，不管程序是Unicode还是ANSI版本，剪贴板文本数据格式都会被正确设置
#ifdef UNICODE
    #define CF_TCHAR CF_UNICODETEXT
#else
    #define CF_TCHAR CF_TEXT
#endif

// 自定义剪贴板数据格式
typedef struct _CUSTOM_DATA
{
    // 假设存储的是一个人的姓名、年龄
    TCHAR szName[128];
    UINT uAge;
}CUSTOM_DATA, * PCUSTOM_DATA;

// 全局变量
HINSTANCE g_hInstance;  // 实例句柄
HWND g_hwnd;            // 窗口句柄
HWND g_hwndList;        // 列表框窗口句柄
UINT g_uFormat;         // 注册剪贴板数据格式

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID OnInit(HWND hwndDlg);          // WM_INITDIALOG

VOID OnBtnText();                   // 写入文本按钮按下
VOID OnBtnBitmap();                 // 写入位图按钮按下
VOID OnBtnCustom();                 // 写入自定义数据按钮按下
VOID OnBtnDelay();                  // 延迟提交按钮按下
VOID OnBtnMultiple();               // 写入多项数据按钮按下

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

    // 注册一个自定义剪贴板格式
    g_uFormat = RegisterClipboardFormat(TEXT("RegisterFormat"));
}

VOID OnBtnText()                   // 写入文本按钮按下
{
    TCHAR szText[128] = { 0 };
    LPTSTR lpStr;

    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 清空剪贴板
    EmptyClipboard();
    // 获取文本数据编辑框的文本
    GetDlgItemText(g_hwnd, IDC_EDIT_TEXT, szText, _countof(szText));
    // 分配内存
    lpStr = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        (_tcslen(szText) + 1) * sizeof(TCHAR));
    if (lpStr)
    {
        // 复制文本数据到刚刚分配的内存中
        StringCchCopy(lpStr, _tcslen(szText) + 1, szText);
        // 将内存中的数据放置到剪贴板
        //SetClipboardData(CF_TEXT, lpStr);
        //SetClipboardData(CF_UNICODETEXT, lpStr);
        SetClipboardData(CF_TCHAR, lpStr);
    }

    // 关闭剪贴板
    CloseClipboard();
}

VOID OnBtnBitmap()                 // 写入位图按钮按下
{
    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 清空剪贴板
    EmptyClipboard();

    // 设置剪贴板位图数据
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

    // 关闭剪贴板
    CloseClipboard();
}

VOID OnBtnCustom()                 // 写入自定义数据按钮按下
{
    LPVOID lpMem;
    // 自定义数据
    CUSTOM_DATA customData = { TEXT("老王"), 40 };

    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 清空剪贴板
    EmptyClipboard();
    // 分配内存
    lpMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CUSTOM_DATA));
    if (lpMem)
    {
        // 复制自定义数据到刚刚分配的内存中
        memcpy_s(lpMem, sizeof(CUSTOM_DATA), &customData, sizeof(CUSTOM_DATA));
        // 设置剪贴板自定义数据
        SetClipboardData(g_uFormat, lpMem);
    }

    // 关闭剪贴板
    CloseClipboard();
}

VOID OnBtnDelay()                  // 延迟提交按钮按下
{
    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 清空剪贴板
    EmptyClipboard();

    // 设置剪贴板延迟提交位图数据
    SetClipboardData(CF_BITMAP, NULL);

    // 关闭剪贴板
    CloseClipboard();
}

VOID OnBtnMultiple()               // 写入多项数据按钮按下
{
    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 清空剪贴板
    EmptyClipboard();

    // 文本数据
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

    // 位图数据
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

    // 自定义数据
    LPVOID lpMem;
    CUSTOM_DATA customData = { TEXT("老王"), 40 };
    lpMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CUSTOM_DATA));
    if (lpMem)
    {
        memcpy_s(lpMem, sizeof(CUSTOM_DATA), &customData, sizeof(CUSTOM_DATA));
        SetClipboardData(g_uFormat, lpMem);
    }

    // 关闭剪贴板
    CloseClipboard();
}


VOID OnRenderFormat(UINT uFormat)  // WM_RENDERFORMAT
{
    SendMessage(g_hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("WM_RENDERFORMAT"));

    // 设置剪贴板位图数据
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