#include <windows.h>
#include <tchar.h>
#include <time.h>
#include "resource.h"

#pragma comment(lib, "Winmm.lib")

// ��������
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
    TCHAR szTextBlue[] = TEXT("�û���������");    // ��ɫˮӡ
    TCHAR szTextRed[] = TEXT("Powered By ����");  // ��ɫˮӡ
    static SIZE sizeBlue, sizeRed;                // ��ɫ����ɫˮӡ�ַ����Ŀ�ȸ߶�
    static RECT rcBlue, rcRed;                    // Ҫ���Ƶ���ɫ����ɫˮӡ�Ŀ�ʼλ�÷�Χ
    static RECT rcExtTextOut = { 0 };             // ����ExtTextOut�������ϴλ�������
    static RECT rcDrawText = { 0 };               // ����DrawText�������ϴλ�������
    RECT rcClient;
    int x, y;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        PlaySound(TEXT("����ʯ�Ͽ���˼.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

        // ��ɫ����ɫˮӡ�ַ����Ŀ�ȸ߶�
        hdc = GetDC(hwndDlg);
        hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("����"));
        hFontOld = (HFONT)SelectObject(hdc, hFont);
        GetTextExtentPoint32(hdc, szTextBlue, _tcslen(szTextBlue), &sizeBlue);
        GetTextExtentPoint32(hdc, szTextRed, _tcslen(szTextRed), &sizeRed);
        SelectObject(hdc, hFontOld);
        ReleaseDC(hwndDlg, hdc);

        // Ҫ���Ƶ���ɫ����ɫˮӡ�ľ��η�Χ
        GetClientRect(hwndDlg, &rcClient);
        SetRect(&rcBlue, 0, 0, rcClient.right - sizeBlue.cx, rcClient.bottom - sizeBlue.cy);
        SetRect(&rcRed, 0, 0, rcClient.right - sizeRed.cx, rcClient.bottom - sizeRed.cy);

        // ����������ʱ�����ֱ���ʾ��ɫ����ɫˮӡ
        SetTimer(hwndDlg, 1, 2000, NULL);       // ��ɫˮӡ��2�봥��һ��
        SetTimer(hwndDlg, 2, 5000, NULL);       // ��ɫˮӡ��5�봥��һ��
        return TRUE;

    case WM_TIMER:
        hdc = GetDC(hwndDlg);
        SetBkMode(hdc, TRANSPARENT);
        hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("����"));
        hFontOld = (HFONT)SelectObject(hdc, hFont);

        switch (wParam)
        {
        case 1:
            // ������ɫˮӡ��2�봥��һ�Σ�Ҫ���Ƶ��ַ����Ŀ�ʼλ�����
            InvalidateRect(hwndDlg, &rcExtTextOut, TRUE);   // �����ϴε�����
            SetTextColor(hdc, RGB(0, 0, 255));
            srand((UINT)time(NULL));
            x = rand() % (rcBlue.right + 1);
            y = rand() % (rcBlue.bottom + 1);
            SetRect(&rcExtTextOut, x, y, x + sizeBlue.cx, y + sizeBlue.cy);
            ExtTextOut(hdc, x, y, 0, NULL, szTextBlue, _tcslen(szTextBlue), NULL);
            break;

        case 2:
            // �����ɫˮӡ��5�봥��һ�Σ�Ҫ���Ƶ��ַ����Ŀ�ʼλ�����
            InvalidateRect(hwndDlg, &rcDrawText, TRUE);     // �����ϴε�����
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