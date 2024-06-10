#include <windows.h>
#include <tchar.h>
#include <Shlobj.h>
#include <strsafe.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL MyCreateShortcut(LPTSTR lpszDestFileName, LPTSTR lpszShortcutFileName,
   LPTSTR lpszWorkingDirectory, WORD wHotKey, int iShowCmd, LPTSTR lpszDescription);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   GUID guid = { 0xB97D20BB, 0xF46A, 0x4C97, {0xBA, 0x10, 0x5E, 0x36, 0x08, 0x43, 0x08, 0x54} };
   LPTSTR lpStrStartup;                        // 返回当前用户的开机自动启动程序目录
   TCHAR szDestFileName[MAX_PATH] = { 0 };     // 可执行文件完整路径
   TCHAR szFileName[MAX_PATH] = { 0 };         // 可执行文件名称
   TCHAR szShortcutFileName[MAX_PATH] = { 0 }; // 快捷方式的保存路径

   switch (uMsg)
   {
   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OK:
         // 获取当前用户的开机自动启动程序目录
         SHGetKnownFolderPath(guid, 0, NULL, &lpStrStartup);

         // 获取当前进程的可执行文件完整路径
         GetModuleFileName(NULL, szDestFileName, _countof(szDestFileName));

         // 拼凑快捷方式的保存路径
         // 开机自动启动程序目录后面加一个反斜杠
         StringCchCopy(szShortcutFileName, _countof(szShortcutFileName), lpStrStartup);
         if (szShortcutFileName[_tcslen(szShortcutFileName) - 1] != TEXT('\\'))
            StringCchCat(szShortcutFileName, _countof(szShortcutFileName), TEXT("\\"));
         // 可执行文件名称.lnk
         StringCchCopy(szFileName, _countof(szFileName), _tcsrchr(szDestFileName, TEXT('\\')) + 1);
         *(_tcsrchr(szFileName, TEXT('.')) + 1) = TEXT('\0');
         StringCchCat(szFileName, _countof(szFileName), TEXT("lnk"));
         // 开机自动启动程序目录\可执行文件名称.lnk
         StringCchCat(szShortcutFileName, _countof(szShortcutFileName), szFileName);

         // 调用自定义函数MyCreateShortcut创建快捷方式
         MyCreateShortcut(szDestFileName, szShortcutFileName, NULL, 0, 0, NULL);

         CoTaskMemFree(lpStrStartup);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

/*********************************************************************************
  * 函数功能：		通过调用COM库接口函数创建程序快捷方式
  * 输入参数的说明：
    1. lpszDestFileName参数表示快捷方式指向的目标文件路径，必须指定
    2. lpszShortcutFileName参数表示快捷方式的保存路径(扩展名为.lnk)，必须指定
    3. lpszWorkingDirectory参数表示起始位置(工作目录)，如果设置为NULL表示程序所在目录
    4. wHotKey参数表示快捷键，设置为0表示不设置快捷键
    5. iShowCmd参数表示运行方式，可以设置为SW_SHOWNORMAL、SW_SHOWMINNOACTIVE或SW_SHOWMAXIMIZED
       分别表示常规窗口、最小化或最大化，设置为0表示常规窗口
    6. lpszDescription参数表示备注(描述)，可以设置为NULL
  * 注意：该函数需要使用tchar.h和Shlobj.h头文件
**********************************************************************************/
BOOL MyCreateShortcut(LPTSTR lpszDestFileName, LPTSTR lpszShortcutFileName,
   LPTSTR lpszWorkingDirectory, WORD wHotKey, int iShowCmd, LPTSTR lpszDescription)
{
   HRESULT hr;

   if (lpszDestFileName == NULL || lpszShortcutFileName == NULL)
      return FALSE;

   // 初始化COM库
   CoInitializeEx(NULL, 0);

   // 创建一个IShellLink对象
   IShellLink* pShellLink;
   hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
   if (FAILED(hr))
      return FALSE;

   // 使用返回的IShellLink对象中的方法设置快捷方式的属性
   // 目标文件路径
   pShellLink->SetPath(lpszDestFileName);

   // 起始位置(工作目录)
   if (!lpszWorkingDirectory)
   {
      TCHAR szWorkingDirectory[MAX_PATH] = { 0 };
      StringCchCopy(szWorkingDirectory, _countof(szWorkingDirectory), lpszDestFileName);
      LPTSTR lpsz = _tcsrchr(szWorkingDirectory, TEXT('\\'));
      *lpsz = TEXT('\0');
      pShellLink->SetWorkingDirectory(szWorkingDirectory);
   }
   else
   {
      pShellLink->SetWorkingDirectory(lpszWorkingDirectory);
   }

   // 快捷键(低字节表示虚拟键码，高字节表示修饰键)
   if (wHotKey != 0)
      pShellLink->SetHotkey(wHotKey);

   // 运行方式
   if (!iShowCmd)
      pShellLink->SetShowCmd(SW_SHOWNORMAL);
   else
      pShellLink->SetShowCmd(iShowCmd);

   // 备注(描述)
   if (lpszDescription != NULL)
      pShellLink->SetDescription(lpszDescription);


   // 调用IShellLink的父类IUnknown中的QueryInterface方法获取IPersistFile对象
   IPersistFile* pPersistFile;
   hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
   if (FAILED(hr))
   {
      pShellLink->Release();
      return FALSE;
   }
   // 使用获取到的IPersistFile对象中的Save方法保存快捷方式到指定位置
   pPersistFile->Save(lpszShortcutFileName, TRUE);

   // 释放相关对象
   pPersistFile->Release();
   pShellLink->Release();
   // 关闭COM库
   CoUninitialize();

   return TRUE;
}