#include <windows.h>
#include <tchar.h>
#include <time.h>
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    HFONT hFont, hFontOld;
    TCHAR szTextBlue[] = TEXT("用户名：老王");    // 蓝色水印
    TCHAR szTextRed[] = TEXT("Powered By 老王");  // 红色水印
    static SIZE sizeBlue, sizeRed;                // 蓝色、红色水印字符串的宽度高度
    static RECT rcBlue, rcRed;                    // 要绘制的蓝色、红色水印的开始位置范围
    static RECT rcExtTextOut = { 0 };             // 保存ExtTextOut函数的上次绘制区域
    static RECT rcDrawText = { 0 };               // 保存DrawText函数的上次绘制区域
    RECT rcClient;
    int x, y;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        PlaySound(TEXT("三生石上刻相思.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

        // 蓝色、红色水印字符串的宽度高度
        hdc = GetDC(hwndDlg);
        hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("宋体"));
        hFontOld = (HFONT)SelectObject(hdc, hFont);
        GetTextExtentPoint32(hdc, szTextBlue, _tcslen(szTextBlue), &sizeBlue);
        GetTextExtentPoint32(hdc, szTextRed, _tcslen(szTextRed), &sizeRed);
        SelectObject(hdc, hFontOld);
        ReleaseDC(hwndDlg, hdc);

        // 要绘制的蓝色、红色水印的矩形范围
        GetClientRect(hwndDlg, &rcClient);
        SetRect(&rcBlue, 0, 0, rcClient.right - sizeBlue.cx, rcClient.bottom - sizeBlue.cy);
        SetRect(&rcRed, 0, 0, rcClient.right - sizeRed.cx, rcClient.bottom - sizeRed.cy);

        // 创建两个计时器，分别显示蓝色、红色水印
        SetTimer(hwndDlg, 1, 2000, NULL);       // 蓝色水印，2秒触发一次
        SetTimer(hwndDlg, 2, 5000, NULL);       // 红色水印，5秒触发一次
        return TRUE;

    case WM_TIMER:
        hdc = GetDC(hwndDlg);
        SetBkMode(hdc, TRANSPARENT);
        hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("宋体"));
        hFontOld = (HFONT)SelectObject(hdc, hFont);

        switch (wParam)
        {
        case 1:
            // 处理蓝色水印，2秒触发一次，要绘制的字符串的开始位置随机
            InvalidateRect(hwndDlg, &rcExtTextOut, TRUE);   // 擦除上次的文字
            SetTextColor(hdc, RGB(0, 0, 255));
            srand((UINT)time(NULL));
            x = rand() % (rcBlue.right + 1);
            y = rand() % (rcBlue.bottom + 1);
            SetRect(&rcExtTextOut, x, y, x + sizeBlue.cx, y + sizeBlue.cy);
            ExtTextOut(hdc, x, y, 0, NULL, szTextBlue, _tcslen(szTextBlue), NULL);
            break;

        case 2:
            // 处理红色水印，5秒触发一次，要绘制的字符串的开始位置随机
            InvalidateRect(hwndDlg, &rcDrawText, TRUE);     // 擦除上次的文字
            SetTextColor(hdc, RGB(255, 0, 0));
            srand(GetTickCount());
            x = rand() % (rcRed.right + 1);
            y = rand() % (rcRed.bottom + 1);
            SetRect(&rcDrawText, x, y, x + sizeRed.cx, y + sizeRed.cy);
            DrawText(hdc, szTextRed, _tcslen(szTextRed), &rcDrawText, DT_SINGLELINE);
            break;
        }

        SelectObject(hdc, hFontOld);
        DeleteObject(hFont);
        ReleaseDC(hwndDlg, hdc);
        return TRUE;

    case WM_SIZE:
        SetRect(&rcBlue, 0, 0, LOWORD(lParam) - sizeBlue.cx, HIWORD(lParam) - sizeBlue.cy);
        SetRect(&rcRed, 0, 0, LOWORD(lParam) - sizeRed.cx, HIWORD(lParam) - sizeRed.cy);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}