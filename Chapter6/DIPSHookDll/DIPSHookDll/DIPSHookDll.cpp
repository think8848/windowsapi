// ����DLL�ĵ�������

#include <Windows.h>
#include <Commctrl.h>
#include "resource.h"

#define DLL_EXPORT
#include "DIPSHookDll.h"

// ȫ�ֱ���
HINSTANCE g_hMod;
HHOOK g_hHook;
TCHAR g_szRegSubKey[] = TEXT("Software\\Desktop Item Position Saver");

// �ڲ�����
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID SaveListViewItemPositions(HWND hwndLV);
VOID RestoreListViewItemPositions(HWND hwndLV);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hMod = hModule;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// ��������
BOOL InstallHook(int idHook, DWORD dwThreadId)
{
    if (!g_hHook)
    {
        g_hHook = SetWindowsHookEx(idHook, GetMsgProc, g_hMod, dwThreadId);
        if (!g_hHook)
            return FALSE;
    }

    // ��Ϣ�����Ѿ���װ��֪ͨ��Դ�������̵߳���GetMsgProc���Ӻ���(Ϊ�˼�ʱ��Ӧ��������֪ͨ)
    PostThreadMessage(dwThreadId, WM_NULL, 0, 0);

    return TRUE;
}

BOOL UninstallHook()
{
    if (g_hHook)
    {
        if (!UnhookWindowsHookEx(g_hHook))
            return FALSE;
    }

    g_hHook = NULL;
    return TRUE;
}

// �ڲ�����
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // dll�Ƿ��Ǹձ�ע��
    static BOOL bFirst = TRUE;

    if (nCode < 0)
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        if (bFirst)
        {
            bFirst = FALSE;

            // ����Դ�����������д���һ��������������������Ƴ��������(���桢�ָ�����ͼ���)
            CreateDialogParam(g_hMod, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_APP:
        if (lParam)
            SaveListViewItemPositions((HWND)wParam);
        else
            RestoreListViewItemPositions((HWND)wParam);
        return TRUE;

    case WM_CLOSE:
        DestroyWindow(hwndDlg);
        return TRUE;
    }

    return FALSE;
}

VOID SaveListViewItemPositions(HWND hwndLV)
{
    int nCount;
    HKEY hKey;
    LVITEM lvi = { 0 };
    TCHAR szName[MAX_PATH] = { 0 };
    POINT pt;

    // ��ɾ����ע���
    RegDeleteKey(HKEY_CURRENT_USER, g_szRegSubKey);

    // ��ȡ�����б�������
    nCount = SendMessage(hwndLV, LVM_GETITEMCOUNT, 0, 0);

    // �����Ӽ�HKEY_CURRENT_USER\Software\Desktop Item Position Saver
    RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE, NULL, &hKey, NULL);

    lvi.mask = LVIF_TEXT;
    lvi.pszText = szName;
    lvi.cchTextMax = _countof(szName);
    // Ϊÿ���б����һ����ֵ����б�����ı�Ϊ���������б����λ��Ϊ��ֵ
    for (int i = 0; i < nCount; i++)
    {
        ZeroMemory(szName, _countof(szName) * sizeof(TCHAR));
        SendMessage(hwndLV, LVM_GETITEMTEXT, i, (LPARAM)&lvi);
        SendMessage(hwndLV, LVM_GETITEMPOSITION, i, (LPARAM)&pt);
        RegSetValueEx(hKey, szName, 0, REG_BINARY, (LPBYTE)&pt, sizeof(pt));
    }

    RegCloseKey(hKey);
}

VOID RestoreListViewItemPositions(HWND hwndLV)
{
    HKEY hKey;
    TCHAR szName[MAX_PATH] = { 0 };
    POINT pt;
    DWORD dwType;
    LONG_PTR lStyle;
    LONG lResult;
    LVFINDINFO lvfi = { 0 };
    int nItem;

    // ���Ӽ�HKEY_CURRENT_USER\Software\Desktop Item Position Saver
    RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegSubKey, 0, KEY_QUERY_VALUE, &hKey);

    // �ر�����ͼ���Զ�����
    lStyle = GetWindowLongPtr(hwndLV, GWL_STYLE);
    if (lStyle & LVS_AUTOARRANGE)
        SetWindowLongPtr(hwndLV, GWL_STYLE, lStyle & ~LVS_AUTOARRANGE);

    // ö���Ӽ�HKEY_CURRENT_USER\Software\Desktop Item Position Saver�µ����м�ֵ��
    lResult = ERROR_SUCCESS;
    for (int i = 0; lResult != ERROR_NO_MORE_ITEMS; i++)
    {
        DWORD dwchName = _countof(szName);
        DWORD dwcbDaata = sizeof(pt);
        lResult = RegEnumValue(hKey, i, szName, &dwchName, NULL, &dwType, (LPBYTE)&pt, &dwcbDaata);
        if (lResult == ERROR_NO_MORE_ITEMS)
            continue;

        // ���������Ͼ���ָ���б����ı����б���������ø��б����λ��
        lvfi.flags = LVFI_STRING;
        lvfi.psz = szName;
        if ((dwType == REG_BINARY) && (dwcbDaata == sizeof(pt)))
        {
            nItem = SendMessage(hwndLV, LVM_FINDITEM, -1, (LPARAM)&lvfi);
            if (nItem != -1)
                SendMessage(hwndLV, LVM_SETITEMPOSITION, nItem, MAKELPARAM(pt.x, pt.y));
        }
    }

    SetWindowLongPtr(hwndLV, GWL_STYLE, lStyle);
    RegCloseKey(hKey);
}