#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Imm32.lib")

// 全局变量
HWND  g_hwndDlg;
HKL   g_hklDefault;     // 原来的默认输入法的键盘布局句柄
HKL   g_hklMy;          // 自己的输入法的键盘布局句柄
TCHAR g_szKLID[16];     // 自己的输入法的键盘布局句柄的字符串形式

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID InstallMyIME();    // 安装输入法
VOID UninstallMyIME();  // 卸载输入法
VOID ClearMyIME();      // 清理输入法

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HANDLE hFileMap;
   static LPVOID lpMemory;
   TCHAR         szInjectDllName[MAX_PATH] = { 0 };      // 注入dll完整路径
   LPTSTR        lpStr = NULL;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      // 同目录下注入dll的完整路径
      GetModuleFileName(NULL, szInjectDllName, _countof(szInjectDllName));
      if (lpStr = _tcsrchr(szInjectDllName, TEXT('\\')))
         StringCchCopy(lpStr + 1, _tcslen(TEXT("MyIMETestDll.dll")) + 1, TEXT("MyIMETestDll.dll"));

      // 创建一个命名文件映射内核对象，4096字节，用于存放注入dll的完整路径
      hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
         0, 4096, TEXT("DEAE59A6-F81B-4DC4-B375-68437206A1A4"));
      if (!hFileMap)
      {
         MessageBox(hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
         return TRUE;
      }

      // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
      lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
      if (!lpMemory)
      {
         MessageBox(hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
         return TRUE;
      }

      // 复制注入dll的完整路径到内存映射文件
      StringCchCopy((LPTSTR)lpMemory, MAX_PATH, szInjectDllName);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_INJECT:
         InstallMyIME();
         break;

      case IDC_BTN_EJECT:
         UninstallMyIME();
         break;

      case IDC_BTN_CLEAR:
         ClearMyIME();
         break;
      }
      return TRUE;

   case WM_CLOSE:
      UnmapViewOfFile(lpMemory);
      CloseHandle(hFileMap);
      UninstallMyIME();
      ClearMyIME();
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}

VOID InstallMyIME()
{
   // 复制.ime文件到系统目录中
   CopyFile(TEXT("MyIME.ime"), TEXT("C:\\WINDOWS\\system32\\MyIME.ime"), FALSE);

   // 获取当前默认输入法的键盘布局句柄(句柄包括语言ID和物理布局ID)，通过全局变量g_hklDefault返回
   SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &g_hklDefault, FALSE);

   // 安装自己的输入法(我这里返回值为0xE0200804)
   g_hklMy = ImmInstallIME(TEXT("MyIME.ime"), TEXT("我的输入法"));
   StringCchPrintf(g_szKLID, _countof(g_szKLID), TEXT("%08X"), (DWORD)g_hklMy);

   // 如果自己的输入法安装成功
   if (ImmIsIME(g_hklMy))
   {
      // 加载自己的输入法到系统中
      LoadKeyboardLayout(g_szKLID, KLF_ACTIVATE);
      // 投递一条WM_INPUTLANGCHANGEREQUEST消息到前台窗口(模拟用户选择新的输入法)
      PostMessage(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 
         INPUTLANGCHANGE_SYSCHARSET, (LPARAM)g_hklMy);
      // 设置为默认输入法
      SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &g_hklMy, SPIF_SENDCHANGE);
      MessageBox(g_hwndDlg, TEXT("我的输入法已经设置为默认输入法"), TEXT("提示"), MB_OK);
   }
}

VOID UninstallMyIME()
{
   // 先设置回原来的默认输入法
   SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &g_hklDefault, SPIF_SENDCHANGE);

   // 卸载自己的输入法
   if (UnloadKeyboardLayout(g_hklMy))
      MessageBox(g_hwndDlg, TEXT("我的输入法已经卸载成功"), TEXT("提示"), MB_OK);
}

VOID ClearMyIME()
{
   // 删除HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layouts\E0200804子键
   TCHAR szSubKey[MAX_PATH] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\");
   StringCchCat(szSubKey, _countof(szSubKey), g_szKLID);
   RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

   // 删除HKEY_CURRENT_USER\Keyboard Layout\Preload下面键值为"E0200804"的键值项
   HKEY    hKey;
   LPCTSTR lpSubKey = TEXT("Keyboard Layout\\Preload");
   DWORD   dwIndex = 0;
   TCHAR   szValueName[16] = { 0 };
   DWORD   dwchValueName;
   TCHAR   szValueData[MAX_PATH] = { 0 };
   DWORD   dwcbValueData;
   LONG    lRet;

   RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ | KEY_WRITE, &hKey);
   while (TRUE)
   {
      dwchValueName = _countof(szValueName);
      dwcbValueData = sizeof(szValueData);
      lRet = RegEnumValue(hKey, dwIndex, szValueName, &dwchValueName, NULL, NULL, 
         (LPBYTE)szValueData, &dwcbValueData);
      if (lRet == ERROR_NO_MORE_ITEMS)
         break;

      if (_tcsicmp(g_szKLID, szValueData) == 0)
         RegDeleteValue(hKey, szValueName);

      dwIndex++;
   }

   // 删除输入法文件
   if (!DeleteFile(TEXT("C:\\WINDOWS\\system32\\MyIME.ime")))
   {
      // 下次重新启动系统以后删除
      MoveFileEx(TEXT("C:\\WINDOWS\\system32\\MyIME.ime"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
      MessageBox(g_hwndDlg, TEXT("我的输入法已清理完毕，重启后删除.ime文件"), TEXT("提示"), MB_OK);
   }
   else
   {
      MessageBox(g_hwndDlg, TEXT("我的输入法已清理完毕"), TEXT("提示"), MB_OK);
   }
}