#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "DataStructure.h"

// ��������
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
            // ���Ҿ���ָ�������ʹ������Ĵ��ڵĴ��ھ��
            hwndTarget = FindWindow(TEXT("#32770"), TEXT("��������"));
            if (hwndTarget)
            {
                // ��ȡ���������䡢���
                GetDlgItemText(hwndDlg, IDC_EDIT_NAME, ps.m_szName, _countof(ps.m_szName));
                ps.m_nAge = GetDlgItemInt(hwndDlg, IDC_EDIT_AGE, NULL, FALSE);
                GetDlgItemText(hwndDlg, IDC_EDIT_MONEY, szBuf, _countof(szBuf));
                ps.m_dMoney = _ttof(szBuf);

                // ����WM_COPYDATA��Ϣ
                cds.dwData = PERSONDATA;
                cds.cbData = sizeof(PersonStruct);
                cds.lpData = &ps;
                SendMessage(hwndTarget, WM_COPYDATA, (WPARAM)hwndTarget, (LPARAM)&cds);
            }
            break;

        case IDC_BTN_SCORE:
            // ���Ҿ���ָ�������ʹ������Ĵ��ڵĴ��ھ��
            hwndTarget = FindWindow(TEXT("#32770"), TEXT("��������"));
            if (hwndTarget)
            {
                // ��ȡ���ġ���ѧ��Ӣ��ɼ�
                GetDlgItemText(hwndDlg, IDC_EDIT_CHINESE, szBuf, _countof(szBuf));
                ss.m_dChinese = _ttof(szBuf);
                GetDlgItemText(hwndDlg, IDC_EDIT_MATH, szBuf, _countof(szBuf));
                ss.m_dMath = _ttof(szBuf);
                GetDlgItemText(hwndDlg, IDC_EDIT_ENGLISH, szBuf, _countof(szBuf));
                ss.m_dEnglish = _ttof(szBuf);

                // ����WM_COPYDATA��Ϣ
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