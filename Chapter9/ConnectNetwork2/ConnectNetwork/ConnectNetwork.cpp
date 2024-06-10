#include <windows.h>
#include <SetupAPI.h>
#include "resource.h"

#pragma comment(lib, "Rpcrt4")
#pragma comment(lib, "SetupAPI")

// ȫ�ֱ���
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL ConnectNetwork(BOOL bConnect);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CONNECT:
            ConnectNetwork(TRUE);
            break;

        case IDC_BTN_DISCONNECT:
            ConnectNetwork(FALSE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL ConnectNetwork(BOOL bConnect)
{
    GUID guid;
    DWORD dwNewState;
    HDEVINFO hDevInfoSet;
    SP_DEVINFO_DATA spDevInfoData;
    int nDeviceIndex = 0;
    SP_PROPCHANGE_PARAMS spPropChangeParams;

    if (bConnect)
        dwNewState = DICS_ENABLE;     //����
    else
        dwNewState = DICS_DISABLE;    //����

    // ������װ��GUID
    UuidFromString((RPC_WSTR)TEXT("4D36E972-E325-11CE-BFC1-08002BE10318"), &guid);

    // ��ȡ�豸��Ϣ�����
    hDevInfoSet = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfoSet == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hwndDlg, TEXT("��ȡ�豸��Ϣ���������"), TEXT("������ʾ"), MB_OK);
        return FALSE;
    }

    // ö���豸
    ZeroMemory(&spDevInfoData, sizeof(SP_DEVINFO_DATA));
    spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    ZeroMemory(&spPropChangeParams, sizeof(SP_PROPCHANGE_PARAMS));
    spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    spPropChangeParams.StateChange = dwNewState;    // ���û����
    spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

    while (TRUE)
    {
        if (!SetupDiEnumDeviceInfo(hDevInfoSet, nDeviceIndex, &spDevInfoData))
        {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;
        }
        nDeviceIndex++;

        // ��װ���豸
        SetupDiSetClassInstallParams(hDevInfoSet, &spDevInfoData,
            (PSP_CLASSINSTALL_HEADER)&spPropChangeParams, sizeof(spPropChangeParams));
        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfoSet, &spDevInfoData);
    }

    // �����豸��Ϣ�����
    SetupDiDestroyDeviceInfoList(hDevInfoSet);
    return TRUE;
}