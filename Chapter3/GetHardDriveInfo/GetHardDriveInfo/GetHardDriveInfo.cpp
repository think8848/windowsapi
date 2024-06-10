#include <windows.h>
#include "resource.h"

// ��������
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
    STORAGE_PROPERTY_QUERY storagePropertyQuery;        // ���뻺����
    CHAR cOutBuffer[1024] = { 0 };                      // ���������
    PSTORAGE_DEVICE_DESCRIPTOR pStorageDeviceDesc;
    DWORD dwBytesReturned;
    CHAR szBuf[1024] = { 0 };
    LPCSTR arrBusType[] = {
    "δ֪����������",          "SCSI��������",             "ATAPI��������",
    "ATA��������",             "IEEE 1394��������",        "SSA��������",
    "����ͨ����������",        "USB",                      "RAID��������",
    "iSCSI��������",           "�������ӵ�SCSI��������",   "SATA",
    "��ȫ����(SD)��������",    "��ý�忨(MMC)��������",    "������������",
    "�ļ�֧�ֵ�������������" };

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
                // ������������
                wsprintf(szDriverName, TEXT("\\\\.\\PhysicalDrive%d"), i);
                hDriver = CreateFile(szDriverName, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, 0, NULL);
                if (hDriver == INVALID_HANDLE_VALUE)
                {
                    wsprintfA(szBuf, "������������%dʧ�ܣ�\r\n\r\n", i);
                    SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                    SendMessageA(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
                    continue;
                }

                // ���ƴ���IOCTL_STORAGE_QUERY_PROPERTY
                ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
                storagePropertyQuery.PropertyId = StorageDeviceProperty;
                storagePropertyQuery.QueryType = PropertyStandardQuery;
                DeviceIoControl(hDriver, IOCTL_STORAGE_QUERY_PROPERTY,
                    &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
                    cOutBuffer, sizeof(cOutBuffer),
                    &dwBytesReturned, NULL);

                pStorageDeviceDesc = (PSTORAGE_DEVICE_DESCRIPTOR)cOutBuffer;
                wsprintfA(szBuf, "����������%d\r\n��Ʒ ID��\t%s\r\n���кţ�\t%s\r\n�ӿ����ͣ�\t%s\r\n\r\n",
                    i, (LPBYTE)pStorageDeviceDesc + pStorageDeviceDesc->ProductIdOffset,
                    (LPBYTE)pStorageDeviceDesc + pStorageDeviceDesc->SerialNumberOffset,
                    arrBusType[pStorageDeviceDesc->BusType]);
                SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                SendMessageA(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                // �ر��豸���
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