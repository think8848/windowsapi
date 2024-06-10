// 定义DLL的导出函数

#include <Windows.h>

#define DLL_EXPORT
#include "GetMd5.h"

// 函数
BOOL GetMd5(LPCTSTR lpFileName, LPTSTR lpMd5)
{
    HANDLE hFile, hFileMap;
    HCRYPTPROV hProv, hHash;
    TCHAR szContainer[] = TEXT("MyKeyContainer");
    LPVOID lpMemory;

    // 打开文件
    hFile = CreateFile(lpFileName, GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    // 获取指定加密服务提供程序(CSP)中指定密钥容器的句柄
    if (!CryptAcquireContext(&hProv, szContainer, NULL, PROV_RSA_FULL, 0))
    {
        if (!CryptAcquireContext(&hProv, szContainer, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            return FALSE;
    }

    // 创建加密服务提供程序(CSP)哈希对象的句柄，第2个参数指定为不同的值可以获取不同的哈希例如SHA
    if (!CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hHash))
        return FALSE;

    // 为了能够处理大型文件，所以使用内存映射文件。为hFile文件对象创建一个文件映射内核对象
    hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hFileMap)
        return FALSE;

    // 获取文件大小，内存分配粒度
    __int64 qwFileSize;
    DWORD dwFileSizeHigh;
    SYSTEM_INFO si;
    qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
    qwFileSize += (((__int64)dwFileSizeHigh) << 32);
    GetSystemInfo(&si);

    // 把文件映射对象hFileMap不断映射到进程的虚拟地址空间中
    __int64 qwFileOffset = 0;   // 文件映射对象偏移量
    DWORD dwBytesInBlock;       // 本次映射大小
    while (qwFileSize > 0)
    {
        dwBytesInBlock = si.dwAllocationGranularity;
        if (qwFileSize < dwBytesInBlock)
            dwBytesInBlock = (DWORD)qwFileSize;

        lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ,
            (DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF), dwBytesInBlock);
        if (!lpMemory)
            return FALSE;

        // 对已映射部分进行操作，将数据添加到哈希对象
        if (!CryptHashData(hHash, (LPBYTE)lpMemory, dwBytesInBlock, 0))
            return FALSE;

        // 取消本次映射，进行下一轮映射
        UnmapViewOfFile(lpMemory);
        qwFileOffset += dwBytesInBlock;
        qwFileSize -= dwBytesInBlock;
    }

    // 获取哈希值中的字节数
    DWORD dwHashLen = 0;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
        return FALSE;

    // 获取哈希值
    LPBYTE lpHash = new BYTE[dwHashLen];
    if (!CryptGetHashParam(hHash, HP_HASHVAL, lpHash, &dwHashLen, 0))
        return FALSE;
    for (DWORD i = 0; i < dwHashLen; i++)
    {
        wsprintf(lpMd5 + i * 2, TEXT("%02X"), lpHash[i]);
    }
    delete[]lpHash;

    // 关闭文件映射内核对象句柄
    CloseHandle(hFileMap);
    // 关闭文件句柄
    CloseHandle(hFile);
    // 释放哈希句柄
    CryptDestroyHash(hHash);
    // 释放CSP句柄
    CryptReleaseContext(hProv, 0);

    return TRUE;
}

/*
  CryptAcquireContext函数用于获取指定加密服务提供程序(CSP)中指定密钥容器的句柄，返回的句柄用于调用所选CSP的CryptoAPI函数；
  CryptCreateHash函数启动数据流的哈希处理， 该函数创建加密服务提供程序(CSP)哈希对象的句柄，该句柄可以用于后续的CryptHashData函数调用；
  CryptHashData函数用于将数据添加到指定的哈希对象，可以多次调用该函数来计算长数据流或不连续数据流的哈希；
  CryptGetHashParam函数用于获取具体的哈希值。
*/