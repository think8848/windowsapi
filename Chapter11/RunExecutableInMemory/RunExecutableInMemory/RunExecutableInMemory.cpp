#include <windows.h>
#include <tchar.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 在dll内存映像中根据函数序数获取函数地址(RVA值)
INT GetFuncRvaByOrdinal(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwOrdinal);

// 在dll内存映像中根据函数名称获取函数地址(RVA值)
INT GetFuncRvaByName(PIMAGE_DOS_HEADER pImageDosHeader, LPCTSTR lpFuncName);

// 加载内存映射文件中的可执行文件到进程内存中执行
BOOL LoadExecutable(LPVOID lpMemory);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   // 打开文件所用变量
   TCHAR szFileName[MAX_PATH] = { 0 };
   OPENFILENAME ofn = { 0 };
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter =
      TEXT("exe文件(*.exe)\0*.exe\0dll文件(*.dll)\0*.dll\0All(*.*)\0*.*\0");
   ofn.nFilterIndex = 3;
   ofn.lpstrFile = szFileName;
   ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFileName);
   ofn.lpstrTitle = TEXT("请选择要打开的PE文件");
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   // 内存映射文件所用变量
   HANDLE hFile, hFileMap;
   LARGE_INTEGER liFileSize;
   LPVOID lpMemory;

   // DOS头指针和PE文件头指针
   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS pImageNtHeader;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFileName);
         break;

      case IDC_BTN_LOAD:
         GetDlgItemText(g_hwndDlg, IDC_EDIT_FILENAME, szFileName, _countof(szFileName));

         // 打开PE文件
         hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("提示"), MB_OK);
            return NULL;
         }
         else
         {
            GetFileSizeEx(hFile, &liFileSize);
            if (liFileSize.QuadPart == 0)
            {
               MessageBox(g_hwndDlg, TEXT("文件大小为0"), TEXT("提示"), MB_OK);
               return NULL;
            }
         }

         // 为hFile文件对象创建一个文件映射内核对象
         hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
         if (!hFileMap)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
            return NULL;
         }

         // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
         lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
         if (!lpMemory)
         {
            MessageBox(g_hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
            return NULL;
         }

         // 打开的文件是不是PE文件
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }
         pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 加载可执行文件到进程内存中
         LoadExecutable(lpMemory);

         // 清理工作
         UnmapViewOfFile(lpMemory);
         CloseHandle(hFileMap);
         CloseHandle(hFile);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

INT GetFuncRvaByOrdinal(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwOrdinal)
{
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE文件头起始地址
   PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;  // 导出表目录结构的起始地址
   PDWORD pAddressOfFunctions;                     // 导出函数地址表的起始地址
   DWORD dwIndexAddressOfFunctions;                // 函数在导出函数地址表中的索引
   DWORD dwFuncRva;                                // 函数的RVA

   // PE文件头起始地址
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE和PE32+的导出表目录结构定位不同
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);
   else
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);

   // 函数在导出函数地址表中的索引
   dwIndexAddressOfFunctions = dwOrdinal - pImageExportDirectory->Base;
   if (dwIndexAddressOfFunctions > pImageExportDirectory->NumberOfFunctions)
      return -1;

   // 导出函数地址表的起始地址
   pAddressOfFunctions = (PDWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfFunctions);

   dwFuncRva = pAddressOfFunctions[dwIndexAddressOfFunctions];

   return dwFuncRva;
}

INT GetFuncRvaByName(PIMAGE_DOS_HEADER pImageDosHeader, LPCTSTR lpFuncName)
{
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE文件头起始地址
   PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;  // 导出表目录结构的起始地址
   PDWORD pAddressOfFunctions;                     // 导出函数地址表的起始地址
   PWORD pAddressOfNameOrdinals;                   // 函数序数表的起始地址
   PDWORD pAddressOfNames;                         // 函数名称地址表的起始地址
   LPSTR lpFuncNameInExport;                       // 函数名称指针
   TCHAR szFuncNameInExport[128];                  // 宽字符函数名称
   WORD wOrdinal;                                  // 函数在导出函数地址表EAT在的索引
   DWORD dwFuncRva;                                // 函数的RVA

   // PE文件头起始地址
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE和PE32+的导出表目录结构定位不同
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);
   else
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);

   // 导出函数地址表的起始地址
   pAddressOfFunctions = (PDWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfFunctions);
   // 函数序数表的起始地址
   pAddressOfNameOrdinals = (PWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfNameOrdinals);
   // 函数名称地址表的起始地址
   pAddressOfNames = (PDWORD)((LPBYTE)pImageDosHeader + pImageExportDirectory->AddressOfNames);

   // 遍历函数名称地址表
   for (DWORD i = 0; i < pImageExportDirectory->NumberOfNames; i++)
   {
      lpFuncNameInExport = (LPSTR)((LPBYTE)pImageDosHeader + pAddressOfNames[i]);
      MultiByteToWideChar(CP_UTF8, 0, lpFuncNameInExport, -1,
         szFuncNameInExport, _countof(szFuncNameInExport));
      if (_tcsicmp(szFuncNameInExport, lpFuncName) == 0)
      {
         wOrdinal = pAddressOfNameOrdinals[i];
         dwFuncRva = pAddressOfFunctions[wOrdinal];

         return dwFuncRva;
      }
   }

   return -1;
}

BOOL LoadExecutable(LPVOID lpMemory)            // lpMemory是PE内存映射文件基地址
{
   PIMAGE_DOS_HEADER pImageDosHeader;          // 内存映射文件中的DOS头起始地址
   PIMAGE_NT_HEADERS pImageNtHeader;           // 内存映射文件中的PE文件头起始地址
   SIZE_T nSizeOfImage;                        // PE内存映像大小(基于内存对齐后的大小)
   LPVOID lpBaseAddress;                       // 在本进程中分配内存用于装载可执行文件
   DWORD dwSizeOfHeaders;                      // DOS头+PE头+节表的大小(基于内存对齐后的大小)
   WORD wNumberOfSections;                     // 可执行文件的节区个数
   PIMAGE_SECTION_HEADER pImageSectionHeader;  // 节表的起始地址

   // 获取PE内存映像大小
   pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
   nSizeOfImage = pImageNtHeader->OptionalHeader.SizeOfImage;

   // 在本进程的内存地址空间中分配nSizeOfImage + 20字节大小的可读可写可执行内存
   // 多出的20个字节后面会用到
   lpBaseAddress = VirtualAlloc(NULL, nSizeOfImage + 20, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
   ZeroMemory(lpBaseAddress, nSizeOfImage + 20);

   // ***************************************************************************************
   // 把可执行文件按pImageNtHeader.OptionalHeader.SectionAlignment对齐粒度映射到分配的内存中
   dwSizeOfHeaders = pImageNtHeader->OptionalHeader.SizeOfHeaders;
   wNumberOfSections = pImageNtHeader->FileHeader.NumberOfSections;

   // 获取节表的起始地址
   pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS));

   // 加载DOS头+PE头+节表
   memcpy_s(lpBaseAddress, dwSizeOfHeaders, (LPVOID)pImageDosHeader, dwSizeOfHeaders);

   // 加载所有节区到节表中指定的RVA处
   for (int i = 0; i < wNumberOfSections; i++)
   {
      if (pImageSectionHeader->VirtualAddress == 0 || pImageSectionHeader->SizeOfRawData == 0)
      {
         pImageSectionHeader++;
         continue;
      }

      memcpy_s((LPBYTE)lpBaseAddress + pImageSectionHeader->VirtualAddress,
         pImageSectionHeader->SizeOfRawData,
         (LPBYTE)pImageDosHeader + pImageSectionHeader->PointerToRawData,
         pImageSectionHeader->SizeOfRawData);

      pImageSectionHeader++;
   }
   // ***************************************************************************************

   // 映射到进程中的DOS头和PE文件头起始地址
   PIMAGE_DOS_HEADER pImageDosHeaderMap;           // 映射到进程中的DOS头起始地址
   PIMAGE_NT_HEADERS pImageNtHeaderMap;            // 映射到进程中的PE文件头起始地址
   pImageDosHeaderMap = (PIMAGE_DOS_HEADER)lpBaseAddress;
   pImageNtHeaderMap = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeaderMap + pImageDosHeaderMap->e_lfanew);

   // ***************************************************************************************
   // 修正映射到进程中的PE内存映像的重定位代码
   PIMAGE_BASE_RELOCATION pImageBaseRelocationMap; // 映射到进程中的重定位表的起始地址
   PWORD pRelocationItem;                          // 重定位项数组的起始地址
   DWORD dwRelocationItem;                         // 重定位项的个数
   PDWORD pdwRelocationAddress;                    // PE重定位地址
   PULONGLONG pullRelocationAddress;               // PE32+重定位地址
   DWORD dwRelocationDelta;                        // PE实际装入地址与建议装载地址的差值
   ULONGLONG ullRelocationDelta;                   // PE32+实际装入地址与建议装载地址的差值

   // 获取重定位表的起始地址
   pImageBaseRelocationMap = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageDosHeaderMap +
      pImageNtHeaderMap->OptionalHeader.DataDirectory[5].VirtualAddress);

   // 这里就不判断是否存在重定位表，因为通常都会存在

   // 遍历重定位表
   while (pImageBaseRelocationMap->VirtualAddress != 0)
   {
      // 重定位项数组的起始地址
      pRelocationItem = (PWORD)((LPBYTE)pImageBaseRelocationMap + sizeof(IMAGE_BASE_RELOCATION));
      // 重定位项的个数
      dwRelocationItem = (pImageBaseRelocationMap->SizeOfBlock -
         sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

      for (DWORD i = 0; i < dwRelocationItem; i++)
      {
         // 区分PE和PE32+的重定位
         if (pRelocationItem[i] >> 12 == 3)
         {
            pdwRelocationAddress = (PDWORD)((LPBYTE)pImageDosHeaderMap +
               pImageBaseRelocationMap->VirtualAddress + (pRelocationItem[i] & 0x0FFF));
            dwRelocationDelta = (DWORD)pImageDosHeaderMap -
               pImageNtHeaderMap->OptionalHeader.ImageBase;
            *pdwRelocationAddress += dwRelocationDelta;
         }
         else if (pRelocationItem[i] >> 12 == 0xA)
         {
            pullRelocationAddress = (PULONGLONG)((LPBYTE)pImageDosHeaderMap +
               pImageBaseRelocationMap->VirtualAddress + (pRelocationItem[i] & 0x0FFF));
            ullRelocationDelta = (ULONGLONG)pImageDosHeaderMap -
               pImageNtHeaderMap->OptionalHeader.ImageBase;
            *pullRelocationAddress += ullRelocationDelta;
         }
      }

      // 指向下一个重定位块结构
      pImageBaseRelocationMap = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageBaseRelocationMap +
         pImageBaseRelocationMap->SizeOfBlock);
   }
   // ***************************************************************************************

   // ***************************************************************************************
   // 修正映射到进程中的PE内存映像的导入函数地址表IAT
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor;// 映射到进程中的导入表起始地址
   PIMAGE_THUNK_DATA pImageThunkData;              // IMAGE_THUNK_DATA数组起始地址
   PIMAGE_IMPORT_BY_NAME pImageImportByName;       // IMAGE_IMPORT_BY_NAME结构指针
   TCHAR szDllName[MAX_PATH] = { 0 };              // dll名称
   HMODULE hDll;                                   // dll模块句柄
   DWORD dwFuncAddress;                            // 32位函数地址
   ULONGLONG ullFuncAddress;                       // 64位函数地址

   // 是否有导入表(当然，没有的可能性不大)
   if (pImageNtHeaderMap->OptionalHeader.DataDirectory[1].Size != 0)
   {
      // 导入表起始地址
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeaderMap +
         pImageNtHeaderMap->OptionalHeader.DataDirectory[1].VirtualAddress);

      // 遍历导入表
      while (pImageImportDescriptor->OriginalFirstThunk || pImageImportDescriptor->TimeDateStamp ||
         pImageImportDescriptor->ForwarderChain || pImageImportDescriptor->Name ||
         pImageImportDescriptor->FirstThunk)
      {
         // 在进程中加载dll
         MultiByteToWideChar(CP_UTF8, 0,
            (LPSTR)((LPBYTE)pImageDosHeaderMap + pImageImportDescriptor->Name), -1,
            szDllName, _countof(szDllName));
         hDll = LoadLibrary(szDllName);

         // IMAGE_THUNK_DATA数组起始地址
         pImageThunkData = (PIMAGE_THUNK_DATA)((LPBYTE)pImageDosHeaderMap +
            pImageImportDescriptor->FirstThunk);
         while (pImageThunkData->u1.AddressOfData != 0)
         {
            // 区分PE和PE32+的IAT
            if (pImageNtHeaderMap->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            {
               // 按序号导入还是按函数名称导入
               if (pImageThunkData->u1.AddressOfData & IMAGE_ORDINAL_FLAG32)
               {
                  // 获取加载的dll中函数的地址
                  dwFuncAddress = (DWORD)GetProcAddress(hDll,
                     (LPSTR)(pImageThunkData->u1.AddressOfData & 0xFFFF));
               }
               else
               {
                  // IMAGE_IMPORT_BY_NAME结构指针
                  pImageImportByName = (PIMAGE_IMPORT_BY_NAME)
                     ((LPBYTE)pImageDosHeaderMap + pImageThunkData->u1.AddressOfData);

                  // 获取加载的dll中函数的地址
                  dwFuncAddress = (DWORD)GetProcAddress(hDll, (LPSTR)pImageImportByName->Name);
               }
               // 修复IAT项
               pImageThunkData->u1.Function = dwFuncAddress;
            }
            else
            {
               // 按序号导入还是按函数名称导入
               if (pImageThunkData->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
               {
                  // 获取加载的dll中函数的地址
                  ullFuncAddress = (ULONGLONG)GetProcAddress(hDll,
                     (LPSTR)(pImageThunkData->u1.AddressOfData & 0xFFFF));
               }
               else
               {
                  // IMAGE_IMPORT_BY_NAME结构指针
                  pImageImportByName = (PIMAGE_IMPORT_BY_NAME)
                     ((LPBYTE)pImageDosHeaderMap + pImageThunkData->u1.AddressOfData);

                  // 获取加载的dll中函数的地址
                  ullFuncAddress = (ULONGLONG)GetProcAddress(hDll,
                     (LPSTR)pImageImportByName->Name);
               }
               // 修复IAT项
               pImageThunkData->u1.Function = ullFuncAddress;
            }

            // 指向下一个IMAGE_THUNK_DATA结构
            pImageThunkData++;
         }

         // 指向下一个导入表描述符
         pImageImportDescriptor++;
      }
   }
   // ***************************************************************************************

   // ***************************************************************************************
   // 修改建议装载地址，并执行可执行文件
   LPVOID lpExeEntry;                          // 可执行文件入口点

   if (pImageNtHeaderMap->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
   {
      ((PIMAGE_NT_HEADERS32)pImageNtHeaderMap)->OptionalHeader.ImageBase = (DWORD)lpBaseAddress;

      lpExeEntry = (LPVOID)((LPBYTE)pImageDosHeaderMap +
         ((PIMAGE_NT_HEADERS32)pImageNtHeaderMap)->OptionalHeader.AddressOfEntryPoint);
   }
   else
   {
      ((PIMAGE_NT_HEADERS64)pImageNtHeaderMap)->OptionalHeader.ImageBase = (ULONGLONG)lpBaseAddress;

      lpExeEntry = (LPVOID)((LPBYTE)pImageDosHeaderMap +
         ((PIMAGE_NT_HEADERS64)pImageNtHeaderMap)->OptionalHeader.AddressOfEntryPoint);
   }

   // 如果本程序编译为64位，不支持内联汇编，因此我们采取直接写入可执行机器码的方式执行exe
#ifndef _WIN64
    // mov eax, 0x12345678
    // jmp eax
   BYTE bDataJmp[7] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
   *(PINT_PTR)(bDataJmp + 1) = (INT_PTR)lpExeEntry;
   memcpy_s((LPBYTE)lpBaseAddress + nSizeOfImage, 7, bDataJmp, 7);
#else
    // mov rax, 0x1234567812345678
    // jmp rax
   BYTE bDataJmp[12] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
   *(PINT_PTR)(bDataJmp + 2) = (INT_PTR)lpExeEntry;
   memcpy_s((LPBYTE)lpBaseAddress + nSizeOfImage, 12, bDataJmp, 12);
#endif

   // 可以根据每个节区的属性设置其对应内存页的内存保护属性，此处省略

   // 是exe还是dll，如果是exe则执行上面的“jmp 入口地址”指令，否则执行DllMain
   if (pImageNtHeaderMap->FileHeader.Characteristics & IMAGE_FILE_DLL)
   {
      // 执行DllMain入口点函数
      typedef BOOL(APIENTRY* pfnDllMain)(HMODULE hModule, DWORD ulreason, LPVOID lpReserved);
      pfnDllMain fnDllMain = (pfnDllMain)(lpExeEntry);
      fnDllMain((HMODULE)lpBaseAddress, DLL_PROCESS_ATTACH, 0);

      // 执行一个导出函数试试
      typedef VOID(*pfnShowMessage)();
      // 如果调用GetProcAddress函数获取ShowMessage函数的地址会提示找不到指定的模块
      /*pfnShowMessage fnShowMessage = (pfnShowMessage)
          GetProcAddress((HMODULE)lpBaseAddress, "ShowMessage");*/

          // GetFuncRvaByName是自定义函数，用于获取指定函数的RVA
      pfnShowMessage fnShowMessage = (pfnShowMessage)((LPBYTE)lpBaseAddress +
         GetFuncRvaByName((PIMAGE_DOS_HEADER)lpBaseAddress, TEXT("ShowMessage")));
      fnShowMessage();
   }
   else
   {
      // 跳转到exe入口点执行
      typedef VOID(WINAPI* pfnExe)();
      pfnExe fnExe = (pfnExe)((LPBYTE)lpBaseAddress + nSizeOfImage);
      fnExe();
   }
   // ***************************************************************************************

   return TRUE;
}