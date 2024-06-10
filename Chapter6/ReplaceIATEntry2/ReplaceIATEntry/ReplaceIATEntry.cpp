#include <windows.h>
#include <dbghelp.h>
#include <TlHelp32.h>

#pragma comment(lib, "Dbghelp.lib")

// Ŀ�꺯��ԭ����ָ��
static HMODULE(WINAPI* OrigLoadLibraryA)(LPCSTR lpLibFileName) = LoadLibraryA;
static HMODULE(WINAPI* OrigLoadLibraryW)(LPCWSTR lpLibFileName) = LoadLibraryW;
static HMODULE(WINAPI* OrigLoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExA;
static HMODULE(WINAPI* OrigLoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExW;

// �Զ��庯��
BOOL HookGetMd5(LPCTSTR lpFileName, LPTSTR lpMd5);
HMODULE WINAPI HookLoadLibraryA(LPCSTR lpLibFileName);
HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName);
HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

// �滻���̵�ָ��ģ�鵼����е�һ��IAT��(���뺯����ַ)
BOOL ReplaceIATInOneMod(HMODULE hModule, LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// �滻���̵�����ģ�鵼����е�һ��IAT��(���뺯����ַ)
VOID ReplaceIATInAllMod(LPCSTR pszDllName, PROC pfnTarget, PROC pfnNew);
// �滻ָ��ģ�鵼�����е�һ��EAT��(����������ַ)
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

HMODULE WINAPI HookLoadLibraryA(LPCSTR lpLibFileName)
{
   // ����ԭLoadLibraryA����
   HMODULE hModule = OrigLoadLibraryA(lpLibFileName);

   // ԭLoadLibraryA����ִ����ϣ���������ģ��ĵ�����е����IAT�����滻һ��
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

// �滻ָ��ģ�鵼�����е�һ��EAT��(����������ַ)
BOOL ReplaceEATInOneMod(HMODULE hModule, LPCSTR pszFuncName, PROC pfnNew)
{
   ULONG                   ulSize;                         // ������Ĵ�С
   PIMAGE_EXPORT_DIRECTORY pImageExportDir = NULL;         // ��������ʼ��ַ
   PDWORD                  pAddressOfFunctions = NULL;     // ����������ַ�����ʼ��ַ
   PWORD                   pAddressOfNameOrdinals = NULL;  // �������������ʼ��ַ
   PDWORD                  pAddressOfNames = NULL;         // �������Ƶ�ַ�����ʼ��ַ

   // ��ȡ��������ʼ��ַ
   pImageExportDir = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx(hModule, TRUE,
      IMAGE_DIRECTORY_ENTRY_EXPORT, &ulSize, NULL);
   if (!pImageExportDir)
      return FALSE;

   // ����������ַ�������������������Ƶ�ַ�����ʼ��ַ
   pAddressOfFunctions = (PDWORD)((LPBYTE)hModule + pImageExportDir->AddressOfFunctions);
   pAddressOfNameOrdinals = (PWORD)((LPBYTE)hModule + pImageExportDir->AddressOfNameOrdinals);
   pAddressOfNames = (PDWORD)((LPBYTE)hModule + pImageExportDir->AddressOfNames);

   // �����������Ƶ�ַ��
   for (DWORD i = 0; i < pImageExportDir->NumberOfNames; i++)
   {
      if (_stricmp(pszFuncName, (LPSTR)((LPBYTE)hModule + pAddressOfNames[i])) != 0)
         continue;

      // �Ѿ��ҵ�Ŀ�꺯������ȡ����������ַ
      PROC* ppfn = (PROC*)&pAddressOfFunctions[pAddressOfNameOrdinals[i]];
      pfnNew = (PROC)((LPBYTE)pfnNew - (LPBYTE)hModule);    // To RVA

      DWORD dwOldProtect;
      BOOL bRet = FALSE;

      // �滻Ŀ��EAT���ֵΪpfnNew
      VirtualProtect(ppfn, sizeof(pfnNew), PAGE_READWRITE, &dwOldProtect);
      bRet = WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
      VirtualProtect(ppfn, sizeof(pfnNew), dwOldProtect, &dwOldProtect);

      return bRet;
   }

   return FALSE;
}