#include <windows.h>
#include <dbghelp.h>
#include <TlHelp32.h>

#pragma comment(lib, "Dbghelp.lib")

// 目标函数原函数指针
static HMODULE(WINAPI* OrigLoadLibraryA)(LPCSTR lpLibFileName) = LoadLibraryA;
static HMODULE(WINAPI* OrigLoadLibraryW)(LPCWSTR lpLibFileName) = LoadLibraryW;
static HMODULE(WINAPI* OrigLoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExA;
static HMODULE(WINAPI* OrigLoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExW;

// 自定义函数
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5);
HMODULE WINAPI HookLoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName);
HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

// 替换进程的指定模块导入表中的一个IAT项(导入函数地址)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// 替换进程的所有模块导入表中的一个IAT项(导入函数地址)
VOID ReplaceIATInAllMod(LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// 替换指定模块导出表中的一个EAT项(导出函数地址)
BOOL ReplaceEATInOneMod(HMODULE hModule, LPCSTR pszFuncName, PROC pfnNew);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryA, (PROC)HookLoadLibraryA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryW, (PROC)HookLoadLibraryW);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExA, (PROC)HookLoadLibraryExA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExW, (PROC)HookLoadLibraryExW);

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }

   return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 替换进程的指定模块导入表中的一个IAT项(导入函数地址)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew)
{
   ULONG                    ulSize;                   // 导入表的大小
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDesc = NULL;  // 导入表起始地址
   PIMAGE_THUNK_DATA        pImageThunkData = NULL;   // IMAGE_THUNK_DATA数组起始地址

   // 获取导入表起始地址
   pImageImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(hModule, TRUE,
      IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize, NULL);
   if (!pImageImportDesc)
      return FALSE;

   // 遍历导入表，查找目标函数
   while (pImageImportDesc->OriginalFirstThunk || pImageImportDesc->TimeDateStamp ||
      pImageImportDesc->ForwarderChain || pImageImportDesc->Name || pImageImportDesc->FirstThunk)
   {
      if (_stricmp(pszDllName, (LPSTR)((LPBYTE)hModule + pImageImportDesc->Name)) == 0)
      {
         pImageThunkData = (PIMAGE_THUNK_DATA)((LPBYTE)hModule + pImageImportDesc->FirstThunk);
         while (pImageThunkData->u1.AddressOfData != 0)
         {
            PROC* ppfn = (PROC*)&pImageThunkData->u1.Function;
            if (*ppfn == pfnTarget)
            {
               DWORD dwOldProtect;
               BOOL bRet = FALSE;

               // 替换目标IAT项的值为pfnNew
               VirtualProtect(ppfn, sizeof(pfnNew), PAGE_READWRITE, &dwOldProtect);
               bRet = WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
               VirtualProtect(ppfn, sizeof(pfnNew), dwOldProtect, &dwOldProtect);

               return bRet;
            }

            // 指向下一个IMAGE_THUNK_DATA结构
            pImageThunkData++;
         }
      }

      // 指向下一个导入表描述符
      pImageImportDesc++;
   }

   return FALSE;
}

// 替换进程的所有模块导入表中的一个IAT项(导入函数地址)
VOID ReplaceIATInAllMod(LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew)
{
   MEMORY_BASIC_INFORMATION mbi = { 0 };
   HMODULE                  hModuleThis;     // 当前代码所处的模块
   HANDLE                   hSnapshot = INVALID_HANDLE_VALUE;
   MODULEENTRY32            me = { sizeof(MODULEENTRY32) };
   BOOL                     bRet = FALSE;

   VirtualQuery(ReplaceIATInAllMod, &mbi, sizeof(mbi));
   hModuleThis = (HMODULE)mbi.AllocationBase;

   hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
   if (hSnapshot == INVALID_HANDLE_VALUE)
      return;

   bRet = Module32First(hSnapshot, &me);
   while (bRet)
   {
      if (me.hModule != hModuleThis)         // 排除当前代码所处的模块
         ReplaceIATInOneMod(me.hModule, pszDllName, pfnTarget, pfnNew);

      bRet = Module32Next(hSnapshot, &me);
   }

   CloseHandle(hSnapshot);
}

// 自定义函数
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5)
{
   MessageBox(NULL, TEXT("延迟加载dll中的GetMd5函数已被Hook"), TEXT("提示"), MB_OK);

   return TRUE;
}

HMODULE WINAPI HookLoadLibraryA(LPCSTR lpLibFileName)
{
   // 调用原LoadLibraryA函数
   HMODULE hModule = OrigLoadLibraryA(lpLibFileName);

   // 原LoadLibraryA函数执行完毕，进程所有模块的导入表中的相关IAT项再替换一次
   if (hModule != NULL)
   {
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryA, (PROC)HookLoadLibraryA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryW, (PROC)HookLoadLibraryW);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExA, (PROC)HookLoadLibraryExA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExW, (PROC)HookLoadLibraryExW);

      if (strstr(lpLibFileName, "GetMd5.dll"))
         ReplaceEATInOneMod(hModule, "GetMd5", (PROC)HookGetMd5);
   }

   return hModule;
}
HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
   HMODULE hModule = OrigLoadLibraryW(lpLibFileName);
   if (hModule != NULL)
   {
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryA, (PROC)HookLoadLibraryA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryW, (PROC)HookLoadLibraryW);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExA, (PROC)HookLoadLibraryExA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExW, (PROC)HookLoadLibraryExW);

      if (wcsstr(lpLibFileName, L"GetMd5.dll"))
         ReplaceEATInOneMod(hModule, "GetMd5", (PROC)HookGetMd5);
   }

   return hModule;
}
HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
   HMODULE hModule = OrigLoadLibraryExA(lpLibFileName, hFile, dwFlags);
   if ((hModule != NULL) &&
      ((dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0) &&
      ((dwFlags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE) == 0) &&
      ((dwFlags & LOAD_LIBRARY_AS_IMAGE_RESOURCE) == 0))
   {
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryA, (PROC)HookLoadLibraryA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryW, (PROC)HookLoadLibraryW);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExA, (PROC)HookLoadLibraryExA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExW, (PROC)HookLoadLibraryExW);

      if (strstr(lpLibFileName, "GetMd5.dll"))
         ReplaceEATInOneMod(hModule, "GetMd5", (PROC)HookGetMd5);
   }

   return hModule;
}
HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
   HMODULE hModule = OrigLoadLibraryExW(lpLibFileName, hFile, dwFlags);
   if ((hModule != NULL) &&
      ((dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0) &&
      ((dwFlags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE) == 0) &&
      ((dwFlags & LOAD_LIBRARY_AS_IMAGE_RESOURCE) == 0))
   {
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryA, (PROC)HookLoadLibraryA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryW, (PROC)HookLoadLibraryW);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExA, (PROC)HookLoadLibraryExA);
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigLoadLibraryExW, (PROC)HookLoadLibraryExW);

      if (wcsstr(lpLibFileName, L"GetMd5.dll"))
         ReplaceEATInOneMod(hModule, "GetMd5", (PROC)HookGetMd5);
   }

   return hModule;
}

// 替换指定模块导出表中的一个EAT项(导出函数地址)
BOOL ReplaceEATInOneMod(HMODULE hModule, LPCSTR pszFuncName, PROC pfnNew)
{
   ULONG                   ulSize;                         // 导出表的大小
   PIMAGE_EXPORT_DIRECTORY pImageExportDir = NULL;         // 导出表起始地址
   PDWORD                  pAddressOfFunctions = NULL;     // 导出函数地址表的起始地址
   PWORD                   pAddressOfNameOrdinals = NULL;  // 函数序数表的起始地址
   PDWORD                  pAddressOfNames = NULL;         // 函数名称地址表的起始地址

   // 获取导出表起始地址
   pImageExportDir = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx(hModule, TRUE,
      IMAGE_DIRECTORY_ENTRY_EXPORT, &ulSize, NULL);
   if (!pImageExportDir)
      return FALSE;

   // 导出函数地址表、函数序数表、函数名称地址表的起始地址
   pAddressOfFunctions = (PDWORD)((LPBYTE)hModule + pImageExportDir->AddressOfFunctions);
   pAddressOfNameOrdinals = (PWORD)((LPBYTE)hModule + pImageExportDir->AddressOfNameOrdinals);
   pAddressOfNames = (PDWORD)((LPBYTE)hModule + pImageExportDir->AddressOfNames);

   // 遍历函数名称地址表
   for (DWORD i = 0; i < pImageExportDir->NumberOfNames; i++)
   {
      if (_stricmp(pszFuncName, (LPSTR)((LPBYTE)hModule + pAddressOfNames[i])) != 0)
         continue;

      // 已经找到目标函数，获取导出函数地址
      PROC* ppfn = (PROC*)&pAddressOfFunctions[pAddressOfNameOrdinals[i]];
      pfnNew = (PROC)((LPBYTE)pfnNew - (LPBYTE)hModule);    // To RVA

      DWORD dwOldProtect;
      BOOL bRet = FALSE;

      // 替换目标EAT项的值为pfnNew
      VirtualProtect(ppfn, sizeof(pfnNew), PAGE_READWRITE, &dwOldProtect);
      bRet = WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
      VirtualProtect(ppfn, sizeof(pfnNew), dwOldProtect, &dwOldProtect);

      return bRet;
   }

   return FALSE;
}