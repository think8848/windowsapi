#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 定义CF_TCHAR，不管程序是Unicode还是ANSI版本，剪贴板文本格式数据都能被正确获取
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
UINT g_uFormat;         // 注册剪贴板数据格式

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID OnInit(HWND hwndDlg);
VOID OnBtnText();                   // 读取文本数据按钮按下
VOID OnBtnBitmap();                 // 读取位图按钮按下
VOID OnBtnCustom();                 // 读取自定义数据按钮按下
VOID OnBtnDelay();                  // 读取延迟提交按钮按下
VOID OnBtnMultiple();               // 读取多项数据按钮按下

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
    // 注册一个自定义剪贴板格式，和写入端使用相同的数据格式名称，因此返回相同的格式标识值
    g_uFormat = RegisterClipboardFormat(TEXT("RegisterFormat"));

    // 显示图片的静态控件设置SS_BITMAP | SS_REALSIZECONTROL样式
    LONG lStyle = GetWindowLongPtr(GetDlgItem(g_hwnd, IDC_STATIC_BITMAP), GWL_STYLE);
    SetWindowLongPtr(GetDlgItem(g_hwnd, IDC_STATIC_BITMAP), GWL_STYLE,
        lStyle | SS_BITMAP | SS_REALSIZECONTROL);
}

VOID OnBtnText()                   // 读取文本数据按钮按下
{
    LPTSTR lpStr;

    // 剪贴板是否包含CF_TCHAR格式的文本
    if (IsClipboardFormatAvailable(CF_TCHAR))
    {
        // 打开剪贴板
        OpenClipboard(g_hwnd);
        // 获取剪贴板中文本格式的数据，并复制到szText缓冲区
        lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
        // 显示到编辑框中
        SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, lpStr);

        // 关闭剪贴板
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("剪贴板没有文本格式的数据！"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnBitmap()                 // 读取位图按钮按下
{
    // 剪贴板是否包含CF_BITMAP格式的数据
    if (IsClipboardFormatAvailable(CF_BITMAP))
    {
        // 打开剪贴板
        OpenClipboard(g_hwnd);
        // 获取剪贴板中位图格式的数据
        HBITMAP hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
        // 显示图片
        SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

        // 关闭剪贴板
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("剪贴板没有位图格式的数据！"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnCustom()                 // 读取自定义数据按钮按下
{
    TCHAR szBuf[128] = { 0 };
    PCUSTOM_DATA pCustomData;

    // 剪贴板是否包含g_uFormat格式的数据
    if (IsClipboardFormatAvailable(g_uFormat))
    {
        // 打开剪贴板
        OpenClipboard(g_hwnd);
        // 获取剪贴板中g_uFormat格式的数据
        pCustomData = (PCUSTOM_DATA)GetClipboardData(g_uFormat);
        wsprintf(szBuf, TEXT("%s, %d"), pCustomData->szName, pCustomData->uAge);
        // 显示到编辑框中
        SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, szBuf);

        // 关闭剪贴板
        CloseClipboard();
    }
    else
    {
        MessageBox(g_hwnd, TEXT("剪贴板没有自定义格式的数据！"), TEXT("Error"), MB_OK);
    }
}

VOID OnBtnDelay()                  // 读取延迟提交按钮按下
{
    // 本程序读取延迟提交实际上读取的也是位图，所以直接调用OnBtnBitmap
    OnBtnBitmap();
}

VOID OnBtnMultiple()                    // 读取多项数据按钮按下
{
    // 清空文本数据编辑框、自定义数据编辑框和图像静态控件的内容
    SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, TEXT(""));
    SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, TEXT(""));
    SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

    // 打开剪贴板
    OpenClipboard(g_hwnd);
    // 枚举剪贴板上当前可用的数据格式
    UINT uFormat = EnumClipboardFormats(0);
    while (uFormat)
    {
        // 这里只处理这3种格式
        if (uFormat == CF_TCHAR)
        {
            LPTSTR lpStr;

            // 获取剪贴板中文本格式的数据，并复制到szText缓冲区
            lpStr = (LPTSTR)GetClipboardData(CF_TCHAR);
            // 显示到编辑框中
            SetDlgItemText(g_hwnd, IDC_EDIT_TEXT, lpStr);
        }
        else if (uFormat == CF_BITMAP)
        {
            // 获取剪贴板中位图格式的数据
            HBITMAP hBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
            // 显示图片
            SendDlgItemMessage(g_hwnd, IDC_STATIC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
        }
        else if (uFormat == g_uFormat)
        {
            TCHAR szBuf[128] = { 0 };
            PCUSTOM_DATA pCustomData;

            // 获取剪贴板中g_uFormat格式的数据
            pCustomData = (PCUSTOM_DATA)GetClipboardData(g_uFormat);
            wsprintf(szBuf, TEXT("%s, %d"), pCustomData->szName, pCustomData->uAge);
            // 显示到编辑框中
            SetDlgItemText(g_hwnd, IDC_EDIT_CUSTOM, szBuf);
        }

        uFormat = EnumClipboardFormats(uFormat);
    }

    // 关闭剪贴板
    CloseClipboard();
}