#include <Windows.h>
#include <tchar.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   TCHAR szBuf[MAX_PATH] = { 0 };          // ģ������
   LPBYTE lpAddress = NULL;                // ҳ���������ʼ��ַ
   MEMORY_BASIC_INFORMATION mbi = { 0 };   // ����ҳ����Ϣ
   int nLen;
   TCHAR szModName[MAX_PATH] = { 0 };
   HWND hwndRemoteApp;

   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      // RemoteApp�Ĵ��ھ��
       hwndRemoteApp = FindWindow(TEXT("#32770"), TEXT("APCInjectApp"));

      while (VirtualQuery(lpAddress, &mbi, sizeof(mbi)) == sizeof(mbi))
      {
         // ҳ��������ҳ���״̬ΪMEM_FREE����
         if (mbi.State == MEM_FREE)
            mbi.AllocationBase = mbi.BaseAddress;

         if ((mbi.AllocationBase == NULL) || (mbi.AllocationBase == hModule) ||
            (mbi.BaseAddress != mbi.AllocationBase))
         {
            // ����ռ�����Ļ���ַΪNULL�����߿ռ�����Ļ���ַ�Ǳ�ģ�����ַ��
            // ����ҳ������Ļ���ַ�����ǿռ�����Ļ���ַ(ÿһ��ģ�����һ��ռ�����)����Ҫ��ģ��������ӵ��б���
            nLen = 0;
         }
         else
         {
            // ��ȡ���ص��ռ��������ַ����ģ���ļ���
            nLen = GetModuleFileName(HMODULE(mbi.AllocationBase), szModName, _countof(szModName));
         }

         if (nLen > 0)
         {
            wsprintf(szBuf, TEXT("%p\t%s\r\n"), mbi.AllocationBase, szModName);
            // ģ��������ʾ��RemoteApp�ı༭�ؼ���
            SendDlgItemMessage(hwndRemoteApp, 1005, EM_SETSEL, -1, -1);
            SendDlgItemMessage(hwndRemoteApp, 1005, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
         }
         lpAddress += mbi.RegionSize;
      }
      break;

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }

   return TRUE;
}