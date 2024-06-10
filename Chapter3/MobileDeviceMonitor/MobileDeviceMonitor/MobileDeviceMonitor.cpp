#include <windows.h>
#include <Dbt.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PDEV_BROADCAST_HDR pDevBroadcastHdr = NULL;
    PDEV_BROADCAST_VOLUME pDevBroadcastVolume = NULL;
    DWORD dwDriverMask, dwIndex;

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

    case WM_DEVICECHANGE:
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
            pDevBroadcastHdr = (PDEV_BROADCAST_HDR)lParam;
            if (pDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                pDevBroadcastVolume = (PDEV_BROADCAST_VOLUME)lParam;
                dwDriverMask = pDevBroadcastVolume->dbcv_unitmask;
                dwIndex = 0x00000001;
                TCHAR szDriverName[] = TEXT("A:\\");
                for (szDriverName[0] = TEXT('A'); szDriverName[0] <= TEXT('Z'); szDriverName[0]++)
                {
                    if ((dwDriverMask & dwIndex) > 0)
                        MessageBox(hwndDlg, szDriverName, TEXT("设备已插入"), MB_OK);

                    // 检测下一个辑驱动器位掩码
                    dwIndex = dwIndex << 1;
                }
            }
            break;

        case DBT_DEVICEREMOVECOMPLETE:
            pDevBroadcastHdr = (PDEV_BROADCAST_HDR)lParam;
            if (pDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                pDevBroadcastVolume = (PDEV_BROADCAST_VOLUME)lParam;
                dwDriverMask = pDevBroadcastVolume->dbcv_unitmask;
                dwIndex = 0x00000001;
                TCHAR szDriverName[] = TEXT("A:\\");
                for (szDriverName[0] = TEXT('A'); szDriverName[0] <= TEXT('Z'); szDriverName[0]++)
                {
                    if ((dwDriverMask & dwIndex) > 0)
                        MessageBox(hwndDlg, szDriverName, TEXT("设备已拔出"), MB_OK);

                    // 检测下一个辑驱动器位掩码
                    dwIndex = dwIndex << 1;
                }
            }
            break;
        }
        return TRUE;
    }

    return FALSE;
}