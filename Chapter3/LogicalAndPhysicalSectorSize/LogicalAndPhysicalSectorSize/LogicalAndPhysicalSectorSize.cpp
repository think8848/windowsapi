#include <Windows.h>
#include <strsafe.h>
#include <strsafe.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // �߼�����������һ�ڽ�����GetDiskFreeSpace�������÷������ﲻʹ�øú���
    // ��ȡӲ�����к�һ�ڽ�����DeviceIoControl�������÷�
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    DISK_GEOMETRY_EX diskGeometryEx = { 0 };

    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    storagePropertyQuery.PropertyId = StorageAccessAlignmentProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;
    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR saad = { sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR) };
    DWORD dwBytesReturned;

    TCHAR szBuf[512] = { 0 };
    TCHAR szTemp[256] = { 0 };

    // ������0���߼�������С������������С
    hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive0"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometryEx, sizeof(diskGeometryEx), &dwBytesReturned, NULL);
        DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), &saad, sizeof(saad), &dwBytesReturned, NULL);
        StringCchPrintf(szBuf, _countof(szBuf), TEXT("������0��\r\n�߼�������С��%d\r\n����������С��%d\r\n\r\n"), diskGeometryEx.Geometry.BytesPerSector, saad.BytesPerPhysicalSector);
        CloseHandle(hDevice);
    }

    // ������1���߼�������С������������С
    hDevice = INVALID_HANDLE_VALUE;
    diskGeometryEx = { 0 };
    saad = { sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR) };
    hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive1"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometryEx, sizeof(diskGeometryEx), &dwBytesReturned, NULL);
        DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), &saad, sizeof(saad), &dwBytesReturned, NULL);
        StringCchPrintf(szTemp, _countof(szTemp), TEXT("������1��\r\n�߼�������С��%d\r\n����������С��%d\r\n\r\n"), diskGeometryEx.Geometry.BytesPerSector, saad.BytesPerPhysicalSector);
        StringCchCat(szBuf, _countof(szBuf), szTemp);
        CloseHandle(hDevice);
    }

    MessageBox(NULL, szBuf, TEXT("������������С"), MB_OK);

    return 0;
}

//typedef struct _DISK_GEOMETRY_EX {
//    DISK_GEOMETRY Geometry;     // DISK_GEOMETRY�ṹ
//    LARGE_INTEGER DiskSize;     // Ӳ�̴�С�����ֽ�Ϊ��λ
//    BYTE  Data[1];              // ��������
//} DISK_GEOMETRY_EX, * PDISK_GEOMETRY_EX;
//
//typedef struct __WRAPPED__ _DISK_GEOMETRY {
//    LARGE_INTEGER Cylinders;            // ������
//    MEDIA_TYPE MediaType;               // �豸���ͣ�ö��ֵ���������̡�Ӳ�̡�U�̵�
//    DWORD TracksPerCylinder;            // ��ͷ��(������)
//    DWORD SectorsPerTrack;              // ÿ�ŵ�������
//    DWORD BytesPerSector;               // ÿ�����ֽ���
//} DISK_GEOMETRY, * PDISK_GEOMETRY;
//
//typedef struct _STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR {
//    DWORD Version;                      // �ýṹ�Ĵ�С
//    DWORD Size;                         // ���ص����ݴ�С
//    DWORD BytesPerCacheLine;            // 
//    DWORD BytesOffsetForCacheAlignment; // 
//    DWORD BytesPerLogicalSector;        // ÿ�߼������ֽ���
//    DWORD BytesPerPhysicalSector;       // ÿ���������ֽ���
//    DWORD BytesOffsetForSectorAlignment;// 
//
//} STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR, * PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR;