#include <windows.h>
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
    static HWND hwndEditInfo;
    TCHAR szDriverName[MAX_PATH] = { 0 };
    HANDLE hDriver;
    STORAGE_PROPERTY_QUERY storagePropertyQuery;        // 输入缓冲区
    CHAR cOutBuffer[1024] = { 0 };                      // 输出缓冲区
    PSTORAGE_DEVICE_DESCRIPTOR pStorageDeviceDesc;
    DWORD dwBytesReturned;
    CHAR szBuf[1024] = { 0 };
    LPCSTR arrBusType[] = {
    "未知的总线类型",          "SCSI总线类型",             "ATAPI总线类型",
    "ATA总线类型",             "IEEE 1394总线类型",        "SSA总线类型",
    "光纤通道总线类型",        "USB",                      "RAID总线类型",
    "iSCSI总线类型",           "串行连接的SCSI总线类型",   "SATA",
    "安全数字(SD)总线类型",    "多媒体卡(MMC)总线类型",    "虚拟总线类型",
    "文件支持的虚拟总线类型" };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndEditInfo = GetDlgItem(hwndDlg, IDC_EDIT_INFO);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_GETINFO:
            for (int i = 0; i < 5; i++)
            {
                // 打开物理驱动器
                wsprintf(szDriverName, TEXT("\\\\.\\PhysicalDrive%d"), i);
                hDriver = CreateFile(szDriverName, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, 0, NULL);
                if (hDriver == INVALID_HANDLE_VALUE)
                {
                    wsprintfA(szBuf, "打开物理驱动器%d失败！\r\n\r\n", i);
                    SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                    SendMessageA(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
                    continue;
                }

                // 控制代码IOCTL_STORAGE_QUERY_PROPERTY
                ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
                storagePropertyQuery.PropertyId = StorageDeviceProperty;
                storagePropertyQuery.QueryType = PropertyStandardQuery;
                DeviceIoControl(hDriver, IOCTL_STORAGE_QUERY_PROPERTY,
                    &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
                    cOutBuffer, sizeof(cOutBuffer),
                    &dwBytesReturned, NULL);

                pStorageDeviceDesc = (PSTORAGE_DEVICE_DESCRIPTOR)cOutBuffer;
                wsprintfA(szBuf, "物理驱动器%d\r\n产品 ID：\t%s\r\n序列号：\t%s\r\n接口类型：\t%s\r\n\r\n",
                    i, (LPBYTE)pStorageDeviceDesc + pStorageDeviceDesc->ProductIdOffset,
                    (LPBYTE)pStorageDeviceDesc + pStorageDeviceDesc->SerialNumberOffset,
                    arrBusType[pStorageDeviceDesc->BusType]);
                SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                SendMessageA(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                // 关闭设备句柄
                CloseHandle(hDriver);
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