#include <windows.h>
#include <tchar.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ��dll�ڴ�ӳ���и��ݺ���������ȡ������ַ(RVAֵ)
INT GetFuncRvaByOrdinal(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwOrdinal);

// ��dll�ڴ�ӳ���и��ݺ������ƻ�ȡ������ַ(RVAֵ)
INT GetFuncRvaByName(PIMAGE_DOS_HEADER pImageDosHeader, LPCTSTR lpFuncName);

// �����ڴ�ӳ���ļ��еĿ�ִ���ļ��������ڴ���ִ��
BOOL LoadExecutable(LPVOID lpMemory);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   // ���ļ����ñ���
   TCHAR szFileName[MAX_PATH] = { 0 };
   OPENFILENAME ofn = { 0 };
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter =
      TEXT("exe�ļ�(*.exe)\0*.exe\0dll�ļ�(*.dll)\0*.dll\0All(*.*)\0*.*\0");
   ofn.nFilterIndex = 3;
   ofn.lpstrFile = szFileName;
   ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFileName);
   ofn.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵�PE�ļ�");
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   // �ڴ�ӳ���ļ����ñ���
   HANDLE hFile, hFileMap;
   LARGE_INTEGER liFileSize;
   LPVOID lpMemory;

   // DOSͷָ���PE�ļ�ͷָ��
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

         // ��PE�ļ�
         hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("��ʾ"), MB_OK);
            return NULL;
         }
         else
         {
            GetFileSizeEx(hFile, &liFileSize);
            if (liFileSize.QuadPart == 0)
            {
               MessageBox(g_hwndDlg, TEXT("�ļ���СΪ0"), TEXT("��ʾ"), MB_OK);
               return NULL;
            }
         }

         // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
         hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
         if (!hFileMap)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFileMapping����ʧ��"), TEXT("��ʾ"), MB_OK);
            return NULL;
         }

         // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
         lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
         if (!lpMemory)
         {
            MessageBox(g_hwndDlg, TEXT("MapViewOfFile����ʧ��"), TEXT("��ʾ"), MB_OK);
            return NULL;
         }

         // �򿪵��ļ��ǲ���PE�ļ�
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }
         pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         // ���ؿ�ִ���ļ��������ڴ���
         LoadExecutable(lpMemory);

         // ������
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
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE�ļ�ͷ��ʼ��ַ
   PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;  // ������Ŀ¼�ṹ����ʼ��ַ
   PDWORD pAddressOfFunctions;                     // ����������ַ�����ʼ��ַ
   DWORD dwIndexAddressOfFunctions;                // �����ڵ���������ַ���е�����
   DWORD dwFuncRva;                                // ������RVA

   // PE�ļ�ͷ��ʼ��ַ
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE��PE32+�ĵ�����Ŀ¼�ṹ��λ��ͬ
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);
   else
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);

   // �����ڵ���������ַ���е�����
   dwIndexAddressOfFunctions = dwOrdinal - pImageExportDirectory->Base;
   if (dwIndexAddressOfFunctions > pImageExportDirectory->NumberOfFunctions)
      return -1;

   // ����������ַ�����ʼ��ַ
   pAddressOfFunctions = (PDWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfFunctions);

   dwFuncRva = pAddressOfFunctions[dwIndexAddressOfFunctions];

   return dwFuncRva;
}

INT GetFuncRvaByName(PIMAGE_DOS_HEADER pImageDosHeader, LPCTSTR lpFuncName)
{
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE�ļ�ͷ��ʼ��ַ
   PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;  // ������Ŀ¼�ṹ����ʼ��ַ
   PDWORD pAddressOfFunctions;                     // ����������ַ�����ʼ��ַ
   PWORD pAddressOfNameOrdinals;                   // �������������ʼ��ַ
   PDWORD pAddressOfNames;                         // �������Ƶ�ַ�����ʼ��ַ
   LPSTR lpFuncNameInExport;                       // ��������ָ��
   TCHAR szFuncNameInExport[128];                  // ���ַ���������
   WORD wOrdinal;                                  // �����ڵ���������ַ��EAT�ڵ�����
   DWORD dwFuncRva;                                // ������RVA

   // PE�ļ�ͷ��ʼ��ַ
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE��PE32+�ĵ�����Ŀ¼�ṹ��λ��ͬ
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);
   else
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader +
         ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress);

   // ����������ַ�����ʼ��ַ
   pAddressOfFunctions = (PDWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfFunctions);
   // �������������ʼ��ַ
   pAddressOfNameOrdinals = (PWORD)((LPBYTE)pImageDosHeader +
      pImageExportDirectory->AddressOfNameOrdinals);
   // �������Ƶ�ַ�����ʼ��ַ
   pAddressOfNames = (PDWORD)((LPBYTE)pImageDosHeader + pImageExportDirectory->AddressOfNames);

   // �����������Ƶ�ַ��
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

BOOL LoadExecutable(LPVOID lpMemory)            // lpMemory��PE�ڴ�ӳ���ļ�����ַ
{
   PIMAGE_DOS_HEADER pImageDosHeader;          // �ڴ�ӳ���ļ��е�DOSͷ��ʼ��ַ
   PIMAGE_NT_HEADERS pImageNtHeader;           // �ڴ�ӳ���ļ��е�PE�ļ�ͷ��ʼ��ַ
   SIZE_T nSizeOfImage;                        // PE�ڴ�ӳ���С(�����ڴ�����Ĵ�С)
   LPVOID lpBaseAddress;                       // �ڱ������з����ڴ�����װ�ؿ�ִ���ļ�
   DWORD dwSizeOfHeaders;                      // DOSͷ+PEͷ+�ڱ�Ĵ�С(�����ڴ�����Ĵ�С)
   WORD wNumberOfSections;                     // ��ִ���ļ��Ľ�������
   PIMAGE_SECTION_HEADER pImageSectionHeader;  // �ڱ����ʼ��ַ

   // ��ȡPE�ڴ�ӳ���С
   pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
   nSizeOfImage = pImageNtHeader->OptionalHeader.SizeOfImage;

   // �ڱ����̵��ڴ��ַ�ռ��з���nSizeOfImage + 20�ֽڴ�С�Ŀɶ���д��ִ���ڴ�
   // �����20���ֽں�����õ�
   lpBaseAddress = VirtualAlloc(NULL, nSizeOfImage + 20, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
   ZeroMemory(lpBaseAddress, nSizeOfImage + 20);

   // ***************************************************************************************
   // �ѿ�ִ���ļ���pImageNtHeader.OptionalHeader.SectionAlignment��������ӳ�䵽������ڴ���
   dwSizeOfHeaders = pImageNtHeader->OptionalHeader.SizeOfHeaders;
   wNumberOfSections = pImageNtHeader->FileHeader.NumberOfSections;

   // ��ȡ�ڱ����ʼ��ַ
   pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS));

   // ����DOSͷ+PEͷ+�ڱ�
   memcpy_s(lpBaseAddress, dwSizeOfHeaders, (LPVOID)pImageDosHeader, dwSizeOfHeaders);

   // �������н������ڱ���ָ����RVA��
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

   // ӳ�䵽�����е�DOSͷ��PE�ļ�ͷ��ʼ��ַ
   PIMAGE_DOS_HEADER pImageDosHeaderMap;           // ӳ�䵽�����е�DOSͷ��ʼ��ַ
   PIMAGE_NT_HEADERS pImageNtHeaderMap;            // ӳ�䵽�����е�PE�ļ�ͷ��ʼ��ַ
   pImageDosHeaderMap = (PIMAGE_DOS_HEADER)lpBaseAddress;
   pImageNtHeaderMap = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeaderMap + pImageDosHeaderMap->e_lfanew);

   // ***************************************************************************************
   // ����ӳ�䵽�����е�PE�ڴ�ӳ����ض�λ����
   PIMAGE_BASE_RELOCATION pImageBaseRelocationMap; // ӳ�䵽�����е��ض�λ�����ʼ��ַ
   PWORD pRelocationItem;                          // �ض�λ���������ʼ��ַ
   DWORD dwRelocationItem;                         // �ض�λ��ĸ���
   PDWORD pdwRelocationAddress;                    // PE�ض�λ��ַ
   PULONGLONG pullRelocationAddress;               // PE32+�ض�λ��ַ
   DWORD dwRelocationDelta;                        // PEʵ��װ���ַ�뽨��װ�ص�ַ�Ĳ�ֵ
   ULONGLONG ullRelocationDelta;                   // PE32+ʵ��װ���ַ�뽨��װ�ص�ַ�Ĳ�ֵ

   // ��ȡ�ض�λ�����ʼ��ַ
   pImageBaseRelocationMap = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageDosHeaderMap +
      pImageNtHeaderMap->OptionalHeader.DataDirectory[5].VirtualAddress);

   // ����Ͳ��ж��Ƿ�����ض�λ����Ϊͨ���������

   // �����ض�λ��
   while (pImageBaseRelocationMap->VirtualAddress != 0)
   {
      // �ض�λ���������ʼ��ַ
      pRelocationItem = (PWORD)((LPBYTE)pImageBaseRelocationMap + sizeof(IMAGE_BASE_RELOCATION));
      // �ض�λ��ĸ���
      dwRelocationItem = (pImageBaseRelocationMap->SizeOfBlock -
         sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

      for (DWORD i = 0; i < dwRelocationItem; i++)
      {
         // ����PE��PE32+���ض�λ
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

      // ָ����һ���ض�λ��ṹ
      pImageBaseRelocationMap = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageBaseRelocationMap +
         pImageBaseRelocationMap->SizeOfBlock);
   }
   // ***************************************************************************************

   // ***************************************************************************************
   // ����ӳ�䵽�����е�PE�ڴ�ӳ��ĵ��뺯����ַ��IAT
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor;// ӳ�䵽�����еĵ������ʼ��ַ
   PIMAGE_THUNK_DATA pImageThunkData;              // IMAGE_THUNK_DATA������ʼ��ַ
   PIMAGE_IMPORT_BY_NAME pImageImportByName;       // IMAGE_IMPORT_BY_NAME�ṹָ��
   TCHAR szDllName[MAX_PATH] = { 0 };              // dll����
   HMODULE hDll;                                   // dllģ����
   DWORD dwFuncAddress;                            // 32λ������ַ
   ULONGLONG ullFuncAddress;                       // 64λ������ַ

   // �Ƿ��е����(��Ȼ��û�еĿ����Բ���)
   if (pImageNtHeaderMap->OptionalHeader.DataDirectory[1].Size != 0)
   {
      // �������ʼ��ַ
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeaderMap +
         pImageNtHeaderMap->OptionalHeader.DataDirectory[1].VirtualAddress);

      // ���������
      while (pImageImportDescriptor->OriginalFirstThunk || pImageImportDescriptor->TimeDateStamp ||
         pImageImportDescriptor->ForwarderChain || pImageImportDescriptor->Name ||
         pImageImportDescriptor->FirstThunk)
      {
         // �ڽ����м���dll
         MultiByteToWideChar(CP_UTF8, 0,
            (LPSTR)((LPBYTE)pImageDosHeaderMap + pImageImportDescriptor->Name), -1,
            szDllName, _countof(szDllName));
         hDll = LoadLibrary(szDllName);

         // IMAGE_THUNK_DATA������ʼ��ַ
         pImageThunkData = (PIMAGE_THUNK_DATA)((LPBYTE)pImageDosHeaderMap +
            pImageImportDescriptor->FirstThunk);
         while (pImageThunkData->u1.AddressOfData != 0)
         {
            // ����PE��PE32+��IAT
            if (pImageNtHeaderMap->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            {
               // ����ŵ��뻹�ǰ��������Ƶ���
               if (pImageThunkData->u1.AddressOfData & IMAGE_ORDINAL_FLAG32)
               {
                  // ��ȡ���ص�dll�к����ĵ�ַ
                  dwFuncAddress = (DWORD)GetProcAddress(hDll,
                     (LPSTR)(pImageThunkData->u1.AddressOfData & 0xFFFF));
               }
               else
               {
                  // IMAGE_IMPORT_BY_NAME�ṹָ��
                  pImageImportByName = (PIMAGE_IMPORT_BY_NAME)
                     ((LPBYTE)pImageDosHeaderMap + pImageThunkData->u1.AddressOfData);

                  // ��ȡ���ص�dll�к����ĵ�ַ
                  dwFuncAddress = (DWORD)GetProcAddress(hDll, (LPSTR)pImageImportByName->Name);
               }
               // �޸�IAT��
               pImageThunkData->u1.Function = dwFuncAddress;
            }
            else
            {
               // ����ŵ��뻹�ǰ��������Ƶ���
               if (pImageThunkData->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
               {
                  // ��ȡ���ص�dll�к����ĵ�ַ
                  ullFuncAddress = (ULONGLONG)GetProcAddress(hDll,
                     (LPSTR)(pImageThunkData->u1.AddressOfData & 0xFFFF));
               }
               else
               {
                  // IMAGE_IMPORT_BY_NAME�ṹָ��
                  pImageImportByName = (PIMAGE_IMPORT_BY_NAME)
                     ((LPBYTE)pImageDosHeaderMap + pImageThunkData->u1.AddressOfData);

                  // ��ȡ���ص�dll�к����ĵ�ַ
                  ullFuncAddress = (ULONGLONG)GetProcAddress(hDll,
                     (LPSTR)pImageImportByName->Name);
               }
               // �޸�IAT��
               pImageThunkData->u1.Function = ullFuncAddress;
            }

            // ָ����һ��IMAGE_THUNK_DATA�ṹ
            pImageThunkData++;
         }

         // ָ����һ�������������
         pImageImportDescriptor++;
      }
   }
   // ***************************************************************************************

   // ***************************************************************************************
   // �޸Ľ���װ�ص�ַ����ִ�п�ִ���ļ�
   LPVOID lpExeEntry;                          // ��ִ���ļ���ڵ�

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

   // ������������Ϊ64λ����֧��������࣬������ǲ�ȡֱ��д���ִ�л�����ķ�ʽִ��exe
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

   // ���Ը���ÿ�������������������Ӧ�ڴ�ҳ���ڴ汣�����ԣ��˴�ʡ��

   // ��exe����dll�������exe��ִ������ġ�jmp ��ڵ�ַ��ָ�����ִ��DllMain
   if (pImageNtHeaderMap->FileHeader.Characteristics & IMAGE_FILE_DLL)
   {
      // ִ��DllMain��ڵ㺯��
      typedef BOOL(APIENTRY* pfnDllMain)(HMODULE hModule, DWORD ulreason, LPVOID lpReserved);
      pfnDllMain fnDllMain = (pfnDllMain)(lpExeEntry);
      fnDllMain((HMODULE)lpBaseAddress, DLL_PROCESS_ATTACH, 0);

      // ִ��һ��������������
      typedef VOID(*pfnShowMessage)();
      // �������GetProcAddress������ȡShowMessage�����ĵ�ַ����ʾ�Ҳ���ָ����ģ��
      /*pfnShowMessage fnShowMessage = (pfnShowMessage)
          GetProcAddress((HMODULE)lpBaseAddress, "ShowMessage");*/

          // GetFuncRvaByName���Զ��庯�������ڻ�ȡָ��������RVA
      pfnShowMessage fnShowMessage = (pfnShowMessage)((LPBYTE)lpBaseAddress +
         GetFuncRvaByName((PIMAGE_DOS_HEADER)lpBaseAddress, TEXT("ShowMessage")));
      fnShowMessage();
   }
   else
   {
      // ��ת��exe��ڵ�ִ��
      typedef VOID(WINAPI* pfnExe)();
      pfnExe fnExe = (pfnExe)((LPBYTE)lpBaseAddress + nSizeOfImage);
      fnExe();
   }
   // ***************************************************************************************

   return TRUE;
}