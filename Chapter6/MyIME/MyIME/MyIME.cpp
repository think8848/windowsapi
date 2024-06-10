#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   static HANDLE  hFileMap;
   static LPVOID  lpMemory;
   static TCHAR   szInjectDllName[MAX_PATH] = { 0 };        // ע��dll����·��
   static HMODULE hModuleInject;                            // ע��dllģ����

   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      // �������ļ�ӳ���ں˶��󣬴��ڴ�ӳ���ļ��л�ȡע��dll������·��
      hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
         0, 4096, TEXT("DEAE59A6-F81B-4DC4-B375-68437206A1A4"));
      if (!hFileMap)
         return FALSE;

      // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
      lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
      if (!lpMemory)
         return FALSE;

      // ��ȡע��dll������·��
      StringCchCopy(szInjectDllName, _countof(szInjectDllName), (LPTSTR)lpMemory);

      // ����ע��dll
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