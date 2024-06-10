#include <windows.h>
#include <dbghelp.h>
#include <TlHelp32.h>

#pragma comment(lib, "Dbghelp.lib")

// Ŀ�꺯��ԭ����ָ��
static FARPROC(WINAPI* OrigGetProcAddress)(HMODULE hModule, LPCSTR lpProcName) = GetProcAddress;

// �Զ��庯��
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5);
FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName);

// �滻���̵�ָ��ģ�鵼����е�һ��IAT��(���뺯����ַ)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// �滻���̵�����ģ�鵼����е�һ��IAT��(���뺯����ַ)
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
// �滻���̵�ָ��ģ�鵼����е�һ��IAT��(���뺯����ַ)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew)
{
   ULONG                    ulSize;                   // �����Ĵ�С
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDesc = NULL;  // �������ʼ��ַ
   PIMAGE_THUNK_DATA        pImageThunkData = NULL;   // IMAGE_THUNK_DATA������ʼ��ַ

   // ��ȡ�������ʼ��ַ
   pImageImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(hModule, TRUE,
      IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize, NULL);
   if (!pImageImportDesc)
      return FALSE;

   // �������������Ŀ�꺯��
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

               // �滻Ŀ��IAT���ֵΪpfnNew
               VirtualProtect(ppfn, sizeof(pfnNew), PAGE_READWRITE, &dwOldProtect);
               bRet = WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
               VirtualProtect(ppfn, sizeof(pfnNew), dwOldProtect, &dwOldProtect);

               return bRet;
            }

            // ָ����һ��IMAGE_THUNK_DATA�ṹ
            pImageThunkData++;
         }
      }

      // ָ����һ�������������
      pImageImportDesc++;
   }

   return FALSE;
}

// �滻���̵�����ģ�鵼����е�һ��IAT��(���뺯����ַ)
VOID ReplaceIATInAllMod(LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew)
{
   MEMORY_BASIC_INFORMATION mbi = { 0 };
   HMODULE                  hModuleThis;     // ��ǰ����������ģ��
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
      if (me.hModule != hModuleThis)         // �ų���ǰ����������ģ��
         ReplaceIATInOneMod(me.hModule, pszDllName, pfnTarget, pfnNew);

      bRet = Module32Next(hSnapshot, &me);
   }

   CloseHandle(hSnapshot);
}

// �Զ��庯��
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5)
{
   MessageBox(NULL, TEXT("�ӳټ���dll�е�GetMd5�����ѱ�Hook"), TEXT("��ʾ"), MB_OK);

   return TRUE;
}

FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
   // ����ԭGetProcAddress����
   FARPROC pfn = OrigGetProcAddress(hModule, lpProcName);

   // ���ԭGetProcAddress������ȡ����GetMd5�����ĵ�ַ�����滻
   if (_stricmp(lpProcName, "GetMd5") == 0)
      pfn = (FARPROC)HookGetMd5;

   return pfn;
}