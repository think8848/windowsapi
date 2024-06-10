#include <windows.h>
#include <SetupAPI.h>
#include "resource.h"

#pragma comment(lib, "Rpcrt4")
#pragma comment(lib, "SetupAPI")

// 全局变量
HWND g_hwndDlg;

// 函数声明
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
        dwNewState = DICS_ENABLE;     //启用
    else
        dwNewState = DICS_DISABLE;    //禁用

    // 网卡安装类GUID
    UuidFromString((RPC_WSTR)TEXT("4D36E972-E325-11CE-BFC1-08002BE10318"), &guid);

    // 获取设备信息集句柄
    hDevInfoSet = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfoSet == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hwndDlg, TEXT("获取设备信息集句柄出错！"), TEXT("错误提示"), MB_OK);
        return FALSE;
    }

    // 枚举设备
    ZeroMemory(&spDevInfoData, sizeof(SP_DEVINFO_DATA));
    spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    ZeroMemory(&spPropChangeParams, sizeof(SP_PROPCHANGE_PARAMS));
    spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    spPropChangeParams.StateChange = dwNewState;    // 启用或禁用
    spPropChangeParams.Scope = DICS_FLAG_GLOBAL;

    while (TRUE)
    {
        if (!SetupDiEnumDeviceInfo(hDevInfoSet, nDeviceIndex, &spDevInfoData))
        {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;
        }
        nDeviceIndex++;

        // 安装该设备
        SetupDiSetClassInstallParams(hDevInfoSet, &spDevInfoData,
            (PSP_CLASSINSTALL_HEADER)&spPropChangeParams, sizeof(spPropChangeParams));
        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfoSet, &spDevInfoData);
    }

    // 销毁设备信息集句柄
    SetupDiDestroyDeviceInfoList(hDevInfoSet);
    return TRUE;
}