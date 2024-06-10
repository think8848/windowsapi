#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   static HANDLE  hFileMap;
   static LPVOID  lpMemory;
   static TCHAR   szInjectDllName[MAX_PATH] = { 0 };        // 注入dll完整路径
   static HMODULE hModuleInject;                            // 注入dll模块句柄

   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      // 打开命名文件映射内核对象，从内存映射文件中获取注入dll的完整路径
      hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
         0, 4096, TEXT("DEAE59A6-F81B-4DC4-B375-68437206A1A4"));
      if (!hFileMap)
         return FALSE;

      // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
      lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
      if (!lpMemory)
         return FALSE;

      // 获取注入dll的完整路径
      StringCchCopy(szInjectDllName, _countof(szInjectDllName), (LPTSTR)lpMemory);

      // 加载注入dll
      hModuleInject = LoadLibrary(szInjectDllName);
      break;

   case DLL_THREAD_ATTACH:
      break;

   case DLL_THREAD_DETACH:
      break;

   case DLL_PROCESS_DETACH:
      UnmapViewOfFile(lpMemory);
      CloseHandle(hFileMap);
      if (hModuleInject)
         FreeLibrary(hModuleInject);
      break;
   }

   return TRUE;
}