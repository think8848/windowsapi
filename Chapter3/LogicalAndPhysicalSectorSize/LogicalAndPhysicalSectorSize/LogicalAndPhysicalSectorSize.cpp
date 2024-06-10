#include <Windows.h>
#include <strsafe.h>
#include <strsafe.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 逻辑驱动器操作一节讲述了GetDiskFreeSpace函数的用法，这里不使用该函数
    // 获取硬盘序列号一节讲述了DeviceIoControl函数的用法
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    DISK_GEOMETRY_EX diskGeometryEx = { 0 };

    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    storagePropertyQuery.PropertyId = StorageAccessAlignmentProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;
    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR saad = { sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR) };
    DWORD dwBytesReturned;

    TCHAR szBuf[512] = { 0 };
    TCHAR szTemp[256] = { 0 };

    // 驱动器0的逻辑扇区大小和物理扇区大小
    hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive0"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometryEx, sizeof(diskGeometryEx), &dwBytesReturned, NULL);
        DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), &saad, sizeof(saad), &dwBytesReturned, NULL);
        StringCchPrintf(szBuf, _countof(szBuf), TEXT("驱动器0：\r\n逻辑扇区大小：%d\r\n物理扇区大小：%d\r\n\r\n"), diskGeometryEx.Geometry.BytesPerSector, saad.BytesPerPhysicalSector);
        CloseHandle(hDevice);
    }

    // 驱动器1的逻辑扇区大小和物理扇区大小
    hDevice = INVALID_HANDLE_VALUE;
    diskGeometryEx = { 0 };
    saad = { sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR) };
    hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive1"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometryEx, sizeof(diskGeometryEx), &dwBytesReturned, NULL);
        DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), &saad, sizeof(saad), &dwBytesReturned, NULL);
        StringCchPrintf(szTemp, _countof(szTemp), TEXT("驱动器1：\r\n逻辑扇区大小：%d\r\n物理扇区大小：%d\r\n\r\n"), diskGeometryEx.Geometry.BytesPerSector, saad.BytesPerPhysicalSector);
        StringCchCat(szBuf, _countof(szBuf), szTemp);
        CloseHandle(hDevice);
    }

    MessageBox(NULL, szBuf, TEXT("驱动器扇区大小"), MB_OK);

    return 0;
}

//typedef struct _DISK_GEOMETRY_EX {
//    DISK_GEOMETRY Geometry;     // DISK_GEOMETRY结构
//    LARGE_INTEGER DiskSize;     // 硬盘大小，以字节为单位
//    BYTE  Data[1];              // 附加数据
//} DISK_GEOMETRY_EX, * PDISK_GEOMETRY_EX;
//
//typedef struct __WRAPPED__ _DISK_GEOMETRY {
//    LARGE_INTEGER Cylinders;            // 柱面数
//    MEDIA_TYPE MediaType;               // 设备类型，枚举值，例如软盘、硬盘、U盘等
//    DWORD TracksPerCylinder;            // 磁头数(盘面数)
//    DWORD SectorsPerTrack;              // 每磁道扇区数
//    DWORD BytesPerSector;               // 每扇区字节数
//} DISK_GEOMETRY, * PDISK_GEOMETRY;
//
//typedef struct _STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR {
//    DWORD Version;                      // 该结构的大小
//    DWORD Size;                         // 返回的数据大小
//    DWORD BytesPerCacheLine;            // 
//    DWORD BytesOffsetForCacheAlignment; // 
//    DWORD BytesPerLogicalSector;        // 每逻辑扇区字节数
//    DWORD BytesPerPhysicalSector;       // 每物理扇区字节数
//    DWORD BytesOffsetForSectorAlignment;// 
//
//} STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR, * PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR;