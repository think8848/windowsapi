#include <windows.h>
#include <strsafe.h>
#include "resource.h"
#include "../CopyDataDemo/DataStructure.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PCOPYDATASTRUCT pCDS;
    PPersonStruct pPS;
    PScoreStruct pSS;
    TCHAR szBuf[128] = { 0 };

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_COPYDATA:
        pCDS = (PCOPYDATASTRUCT)lParam;
        if (pCDS->dwData == PERSONDATA)
        {
            pPS = (PPersonStruct)(pCDS->lpData);
            StringCchPrintf(szBuf, _countof(szBuf),
                TEXT("个人信息：\n姓名：%s\n年龄：%d\n存款：%.2lf"),
                pPS->m_szName, pPS->m_nAge, pPS->m_dMoney);
            MessageBox(hwndDlg, szBuf, TEXT("个人信息"), MB_OK);
        }
        else if (pCDS->dwData == SCOREDATA)
        {
            pSS = (PScoreStruct)pCDS->lpData;
            StringCchPrintf(szBuf, _countof(szBuf),
                TEXT("考试成绩：\n语文：%6.2lf\n数学：%6.2lf\n英语：%6.2lf"),
                pSS->m_dChinese, pSS->m_dMath, pSS->m_dEnglish);
            MessageBox(hwndDlg, szBuf, TEXT("考试成绩"), MB_OK);
        }
        return TRUE;
    }

    return FALSE;
}