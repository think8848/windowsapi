#include <windows.h>
#include <dbghelp.h>
#include <TlHelp32.h>

#pragma comment(lib, "Dbghelp.lib")

// 目标函数原函数指针
static FARPROC(WINAPI* OrigGetProcAddress)(HMODULE hModule, LPCSTR lpProcName) = GetProcAddress;

// 自定义函数
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5);
FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName);

// 替换进程的指定模块导入表中的一个IAT项(导入函数地址)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// 替换进程的所有模块导入表中的一个IAT项(导入函数地址)
VOID ReplaceIATInAllMod(LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      ReplaceIATInAllMod("kernel32.dll", (PROC)OrigGetProcAddress, (PROC)HookGetProcAddress);

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

FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
   // 调用原GetProcAddress函数
   FARPROC pfn = OrigGetProcAddress(hModule, lpProcName);

   // 如果原GetProcAddress函数获取的是GetMd5函数的地址，则替换
   if (_stricmp(lpProcName, "GetMd5") == 0)
      pfn = (FARPROC)HookGetMd5;

   return pfn;
}