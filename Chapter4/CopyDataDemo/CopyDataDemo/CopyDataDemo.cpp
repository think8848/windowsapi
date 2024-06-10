#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "DataStructure.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    COPYDATASTRUCT cds = { 0 };
    PersonStruct ps = { 0 };
    ScoreStruct ss = { 0 };
    TCHAR szBuf[32] = { 0 };
    HWND hwndTarget;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_PERSON:
            // 查找具有指定类名和窗口名的窗口的窗口句柄
            hwndTarget = FindWindow(TEXT("#32770"), TEXT("接收数据"));
            if (hwndTarget)
            {
                // 获取姓名、年龄、存款
                GetDlgItemText(hwndDlg, IDC_EDIT_NAME, ps.m_szName, _countof(ps.m_szName));
                ps.m_nAge = GetDlgItemInt(hwndDlg, IDC_EDIT_AGE, NULL, FALSE);
                GetDlgItemText(hwndDlg, IDC_EDIT_MONEY, szBuf, _countof(szBuf));
                ps.m_dMoney = _ttof(szBuf);

                // 发送WM_COPYDATA消息
                cds.dwData = PERSONDATA;
                cds.cbData = sizeof(PersonStruct);
                cds.lpData = &ps;
                SendMessage(hwndTarget, WM_COPYDATA, (WPARAM)hwndTarget, (LPARAM)&cds);
            }
            break;

        case IDC_BTN_SCORE:
            // 查找具有指定类名和窗口名的窗口的窗口句柄
            hwndTarget = FindWindow(TEXT("#32770"), TEXT("接收数据"));
            if (hwndTarget)
            {
                // 获取语文、数学、英语成绩
                GetDlgItemText(hwndDlg, IDC_EDIT_CHINESE, szBuf, _countof(szBuf));
                ss.m_dChinese = _ttof(szBuf);
                GetDlgItemText(hwndDlg, IDC_EDIT_MATH, szBuf, _countof(szBuf));
                ss.m_dMath = _ttof(szBuf);
                GetDlgItemText(hwndDlg, IDC_EDIT_ENGLISH, szBuf, _countof(szBuf));
                ss.m_dEnglish = _ttof(szBuf);

                // 发送WM_COPYDATA消息
                cds.dwData = SCOREDATA;
                cds.cbData = sizeof(ScoreStruct);
                cds.lpData = &ss;
                SendMessage(hwndTarget, WM_COPYDATA, (WPARAM)hwndTarget, (LPARAM)&cds);
            }
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}