#include <windows.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//////////////////////////////////////////////////////////////////////////
typedef unsigned __int64 QWORD;

// 原始SMBIOS固件表结构
typedef struct _RawSMBIOSTable
{
   BYTE    m_bUsed20CallingMethod;
   BYTE    m_bSMBIOSMajorVersion;
   BYTE    m_bSMBIOSMinorVersion;
   BYTE    m_bDmiRevision;         // 开头4个字段我们不关心
   DWORD   m_dwLength;             // 原始SMBIOS固件表数据的长度，以字节为单位
   BYTE    m_bSMBIOSTableData[1];  // 偏移8字节，原始SMBIOS固件表数据，可变长度
}RawSMBIOSTable, * PRawSMBIOSTable;

// SMBIOS结构的格式化区域中的结构头
typedef struct _SMBIOSStructHeader
{
   BYTE    m_bType;    // 结构类型
   BYTE    m_bLength;  // 该类型结构的格式化区域长度(请注意长度取决于主板或系统支持的版本)
   WORD    m_wHandle;  // 结构句柄(0～0xFEFF范围内的数字)
}SMBIOSStructHeader, * PSMBIOSStructHeader;

#pragma pack(1)
// 系统信息(Type 1)SMBIOS结构的格式化区域的完整定义
typedef struct _Type1SystemInformation
{
   SMBIOSStructHeader m_sHeader; // SMBIOS结构头SMBIOSStructHeader
   BYTE    m_bManufacturer;      // Manufacturer字符串的编号
   BYTE    m_bProductName;       // Product Name字符串的编号
   BYTE    m_bVersion;           // Version字符串的编号
   BYTE    m_bSerialNumber;      // BIOS Serial Number字符串的编号
   UUID    m_uuid;               // UUID
   BYTE    m_bWakeupType;        // 标识导致系统启动的事件(原因)
   BYTE    m_bSKUNumber;         // SKU Number字符串的编号
   BYTE    m_bFamily;            // Family字符串的编号
}Type1SystemInformation, * PType1SystemInformation;

// 基板信息(Type 2)SMBIOS结构的格式化区域的完整定义
typedef struct _Type2BaseboardInformation
{
   SMBIOSStructHeader m_sHeader; // SMBIOS结构头SMBIOSStructHeader
   BYTE    m_bManufactur;        // Manufactur字符串的编号
   BYTE    m_bProduct;           // Product字符串的编号
   BYTE    m_bVersion;           // Version字符串的编号
   BYTE    m_bSerialNumber;      // Baseboard Serial Number字符串的编号
   BYTE    m_bAssetTag;          // Asset Tag字符串的编号
   BYTE    m_bFeatureFlags;      // 基板特征标志
   BYTE    m_bLocationInChassis; // Location In Chassis字符串的编号
   WORD    m_wChassisHandle;     // Chassis Handle
   BYTE    m_bBoardType;         // 基板类型
   //BYTE  m_bNumberOfContainedObjectHandles;
   //WORD  m_wContainedObjectHandles[1];
}Type2BaseboardInformation, * PType2BaseboardInformation;

// 处理器信息(Type 4)SMBIOS结构的格式化区域的完整定义
typedef struct _Type4ProcessorInformation
{
   SMBIOSStructHeader m_sHeader;       // SMBIOS结构头SMBIOSStructHeader
   BYTE    m_bSocketDesignation;       // Socket Designation字符串的编号
   BYTE    m_bProcessorType;           // 处理器类型，ENUM值，例如03是中央处理器
   BYTE    m_bProcessorFamily;         // 处理器家族，ENUM值，例如0xC6是Intel® Core™ i7 processor
   BYTE    m_bProcessorManufacturer;   // Processor Manufacturer字符串的编号
   QWORD   m_qProcessorID;             // CPUID(本书结尾还会介绍)，包含描述处理器功能的特定信息
   BYTE    m_bProcessorVersion;        // Processor Version字符串的编号
   BYTE    m_bVoltage;                 // Voltage(电压)
   WORD    m_wExternalClock;           // 外部时钟频率，以MHz为单位
   WORD    m_wMaxSpeed;                // 适用于233MHz处理器
   WORD    m_wCurrentSpeed;            // 处理器在系统引导时的速度，处理器可以支持多种速度
   BYTE    m_bStatus;                  // CPU和CPU插槽的状态
   BYTE    m_bProcessorUpgrade;        // Processor Upgrade，ENUM值
   WORD    m_wL1CacheHandle;           // 一级缓存信息结构的句柄
   WORD    m_wL2CacheHandle;           // 二级缓存信息结构的句柄
   WORD    m_wL3CacheHandle;           // 三级缓存信息结构的句柄
   BYTE    m_bSerialNumber;            // 处理器序列号字符串的编号，由制造商设置，通常不可更改
   BYTE    m_bAssetTag;                // Asset Tag字符串的编号
   BYTE    m_bPartNumber;              // 处理器部件号字符串的编号，由制造商设置，通常不可更改
   BYTE    m_bCoreCount;               // 处理器的核心数
   BYTE    m_bCoreEnabled;             // 由BIOS启用并可供系统使用的核心数
   BYTE    m_bThreadCount;             // 处理器的线程数
   WORD    m_wProcessorCharacteristics;// 处理器特性，例如0x0004表示64位处理器
   // 最后4个字段2.6及以上版本才支持
   //WORD    m_wProcessorFamily2;      // 处理器家族2
   //WORD    m_wCoreCount2;            // 处理器的核心数，用于个数大于255时
   //WORD    m_wCoreEnabled2;          // 由BIOS启用并可供系统使用的核心数，用于个数大于255时
   //WORD    m_wThreadCount2;          // 处理器的线程数，用于个数大于255时
}Type4ProcessorInformation, * PType4ProcessorInformation;
#pragma pack()
//////////////////////////////////////////////////////////////////////////

// 全局变量
HINSTANCE g_hInstance;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 从SMBIOS结构的未格式化区域中查找指定编号的字符串
LPSTR FingStrFromStruct(PSMBIOSStructHeader pStructHeader, BYTE bNum);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   HWND hwndEdit = NULL;
   DWORD dwBufferSize = 0;
   LPBYTE lpBuf = NULL;
   PRawSMBIOSTable pRawSMBIOSTable = NULL;   // 原始SMBIOS固件表
   LPBYTE lpData = NULL;                     // 原始SMBIOS固件表数据
   PSMBIOSStructHeader pStructHeader = NULL; // SMBIOS结构的格式化区域中的结构头
   CHAR szBuf[1024] = { 0 };

   switch (uMsg)
   {
   case WM_INITDIALOG:
      hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_INFO);

      // 第1次调用获取所需的缓冲区大小
      dwBufferSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
      if (dwBufferSize == 0)
         return TRUE;

      // 第2次调用获取原始SMBIOS固件表
      lpBuf = new BYTE[dwBufferSize];
      ZeroMemory(lpBuf, dwBufferSize);
      if (GetSystemFirmwareTable('RSMB', 0, lpBuf, dwBufferSize) != dwBufferSize)
         return TRUE;

      // 解析获取到的原始SMBIOS固件表数据
      pRawSMBIOSTable = (PRawSMBIOSTable)lpBuf;
      lpData = pRawSMBIOSTable->m_bSMBIOSTableData;
      while ((lpData - pRawSMBIOSTable->m_bSMBIOSTableData) < pRawSMBIOSTable->m_dwLength)
      {
         // 根据SMBIOS结构的格式化区域中的结构头的m_bType字段确定结构类型
         // 确定结构类型以后再把pStructHeader转换为指向对应的格式化区域完整定义的指针
         pStructHeader = (PSMBIOSStructHeader)lpData;

         if (pStructHeader->m_bType == 1)
         {
            PType1SystemInformation pType1 = (PType1SystemInformation)pStructHeader;

            CHAR szUUID[64] = { 0 };
            wsprintfA(szUUID, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
               pType1->m_uuid.Data1, pType1->m_uuid.Data2, pType1->m_uuid.Data3,
               pType1->m_uuid.Data4[0], pType1->m_uuid.Data4[1], pType1->m_uuid.Data4[2], pType1->m_uuid.Data4[3],
               pType1->m_uuid.Data4[4], pType1->m_uuid.Data4[5], pType1->m_uuid.Data4[6], pType1->m_uuid.Data4[7]);

            ZeroMemory(szBuf, sizeof(szBuf));
            StringCchPrintfA(szBuf, _countof(szBuf), "系统信息(类型1)：\r\nType\t\t%d\r\nLength\t\t0x%X\r\nHandle\t\t%d\r\nManufacturer\t%s\r\nProduct Name\t%s\r\nVersion\t\t%s\r\nSerial Number\t%s\r\nUUID\t\t%s\r\nWake-up Type\t%d\r\nSKU Number\t%s\r\nFamily\t\t%s\r\n\r\n",
               pType1->m_sHeader.m_bType,
               pType1->m_sHeader.m_bLength,
               pType1->m_sHeader.m_wHandle,
               FingStrFromStruct(pStructHeader, pType1->m_bManufacturer),
               FingStrFromStruct(pStructHeader, pType1->m_bProductName),
               FingStrFromStruct(pStructHeader, pType1->m_bVersion),
               FingStrFromStruct(pStructHeader, pType1->m_bSerialNumber),
               szUUID,
               pType1->m_bWakeupType,
               FingStrFromStruct(pStructHeader, pType1->m_bSKUNumber),
               FingStrFromStruct(pStructHeader, pType1->m_bFamily));
            SendMessageA(hwndEdit, EM_SETSEL, -1, -1);
            SendMessageA(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
         }

         else if (pStructHeader->m_bType == 2)
         {
            PType2BaseboardInformation pType2 = (PType2BaseboardInformation)pStructHeader;

            ZeroMemory(szBuf, sizeof(szBuf));
            StringCchPrintfA(szBuf, _countof(szBuf), "基板信息(类型2)：\r\nType\t\t%d\r\nLength\t\t0x%X\r\nHandle\t\t%d\r\nManufactur\t%s\r\nProduct\t\t%s\r\nVersion\t\t%s\r\nSerial Number\t%s\r\nAsset Tag\t%s\r\nFeature Flags\t%d\r\nLocation In Chassis\t%s\r\nChassis Handle\t%d\r\nBoard Type\t%d\r\n\r\n",
               pType2->m_sHeader.m_bType,
               pType2->m_sHeader.m_bLength,
               pType2->m_sHeader.m_wHandle,
               FingStrFromStruct(pStructHeader, pType2->m_bManufactur),
               FingStrFromStruct(pStructHeader, pType2->m_bProduct),
               FingStrFromStruct(pStructHeader, pType2->m_bVersion),
               FingStrFromStruct(pStructHeader, pType2->m_bSerialNumber),
               FingStrFromStruct(pStructHeader, pType2->m_bAssetTag),
               pType2->m_bFeatureFlags,
               FingStrFromStruct(pStructHeader, pType2->m_bLocationInChassis),
               pType2->m_wChassisHandle,
               pType2->m_bBoardType);
            SendMessageA(hwndEdit, EM_SETSEL, -1, -1);
            SendMessageA(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
         }

         else if (pStructHeader->m_bType == 4)
         {
            PType4ProcessorInformation pType4 = (PType4ProcessorInformation)pStructHeader;

            ZeroMemory(szBuf, sizeof(szBuf));
            StringCchPrintfA(szBuf, _countof(szBuf), "处理器信息(类型4)：\r\nType\t\t\t%d\r\nLength\t\t\t0x%X\r\nHandle\t\t\t%d\r\nSocket Designation\t\t%s\r\nProcessor Type\t\t%d\r\nProcessor Family\t\t%d\r\nProcessor Manufacturer\t%s\r\nProcessor ID\t\t%I64X\r\nProcessor Version\t\t%s\r\nVoltage\t\t\t%d\r\nExternal Clock\t\t%d\r\nMax Speed\t\t%d\r\nCurrent Speed\t\t%d\r\nStatus\t\t\t%d\r\nProcessor Upgrade\t\t%d\r\nL1 Cache Handle\t\t%d\r\nL2 Cache Handle\t\t%d\r\nL3 Cache Handle\t\t%d\r\nSerial Number\t\t%s\r\nAsset Tag\t\t%s\r\nPart Number\t\t%s\r\nCore Count\t\t%d\r\nCore Enabled\t\t%d\r\nThread Count\t\t%d\r\nProcessor Characteristics\t%d\r\n\r\n",
               pType4->m_sHeader.m_bType,
               pType4->m_sHeader.m_bLength,
               pType4->m_sHeader.m_wHandle,
               FingStrFromStruct(pStructHeader, pType4->m_bSocketDesignation),
               pType4->m_bProcessorType,
               pType4->m_bProcessorFamily,
               FingStrFromStruct(pStructHeader, pType4->m_bProcessorManufacturer),
               pType4->m_qProcessorID,
               FingStrFromStruct(pStructHeader, pType4->m_bProcessorVersion),
               pType4->m_bVoltage,
               pType4->m_wExternalClock,
               pType4->m_wMaxSpeed,
               pType4->m_wCurrentSpeed,
               pType4->m_bStatus,
               pType4->m_bProcessorUpgrade,
               pType4->m_wL1CacheHandle,
               pType4->m_wL2CacheHandle,
               pType4->m_wL3CacheHandle,
               FingStrFromStruct(pStructHeader, pType4->m_bSerialNumber),
               FingStrFromStruct(pStructHeader, pType4->m_bAssetTag),
               FingStrFromStruct(pStructHeader, pType4->m_bPartNumber),
               pType4->m_bCoreCount,
               pType4->m_bCoreEnabled,
               pType4->m_bThreadCount,
               pType4->m_wProcessorCharacteristics);
            SendMessageA(hwndEdit, EM_SETSEL, -1, -1);
            SendMessageA(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
         }

         // 遍历完本SMBIOS结构的未格式化区域，以指向下一个SMBIOS结构
         lpData += pStructHeader->m_bLength;
         while ((*(LPWORD)lpData) != 0) lpData++;
         lpData += 2;
      }

      delete[]lpBuf;
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OPEN:
         break;
      }
      return TRUE;

   case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}

LPSTR FingStrFromStruct(PSMBIOSStructHeader pStructHeader, BYTE bNum)
{
   // 指向SMBIOS结构的未格式化区域(字符串数组)
   LPBYTE lpByte = (LPBYTE)pStructHeader + pStructHeader->m_bLength;

   // 字符串编号从1开始
   for (BYTE i = 1; i < bNum; i++)
      lpByte += strlen((LPSTR)lpByte) + 1;

   return (LPSTR)lpByte;
}