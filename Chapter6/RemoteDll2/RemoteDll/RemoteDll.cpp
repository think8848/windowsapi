#include <Windows.h>
#include <tchar.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   TCHAR szBuf[MAX_PATH] = { 0 };          // 模块名称
   LPBYTE lpAddress = NULL;                // 页面区域的起始地址
   MEMORY_BASIC_INFORMATION mbi = { 0 };   // 返回页面信息
   int nLen;
   TCHAR szModName[MAX_PATH] = { 0 };
   HWND hwndRemoteApp;

   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      // RemoteApp的窗口句柄
       hwndRemoteApp = FindWindow(TEXT("#32770"), TEXT("APCInjectApp"));

      while (VirtualQuery(lpAddress, &mbi, sizeof(mbi)) == sizeof(mbi))
      {
         // 页面区域中页面的状态为MEM_FREE空闲
         if (mbi.State == MEM_FREE)
            mbi.AllocationBase = mbi.BaseAddress;

         if ((mbi.AllocationBase == NULL) || (mbi.AllocationBase == hModule) ||
            (mbi.BaseAddress != mbi.AllocationBase))
         {
            // 如果空间区域的基地址为NULL，或者空间区域的基地址是本模块基地址，
            // 或者页面区域的基地址并不是空间区域的基地址(每一个模块就是一块空间区域)，则不要将模块名称添加到列表中
            nLen = 0;
         }
         else
         {
            // 获取加载到空间区域基地址处的模块文件名
            nLen = GetModuleFileName(HMODULE(mbi.AllocationBase), szModName, _countof(szModName));
         }

         if (nLen > 0)
         {
            wsprintf(szBuf, TEXT("%p\t%s\r\n"), mbi.AllocationBase, szModName);
            // 模块名称显示到RemoteApp的编辑控件中
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