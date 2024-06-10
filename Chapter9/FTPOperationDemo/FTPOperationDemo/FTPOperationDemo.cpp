#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Wininet.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static TCHAR  szFile[MAX_PATH] = { 0 };      // 返回用户选择的本地文件名的缓冲区
   static TCHAR  szFileTitle[MAX_PATH] = { 0 }; // 返回用户所选本地文件的文件名和扩展名的缓冲区
   OPENFILENAME  ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter = TEXT("All(*.*)\0*.*\0");
   ofn.lpstrFile = szFile;
   //ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFile);
   ofn.lpstrFileTitle = szFileTitle;
   ofn.nMaxFileTitle = _countof(szFileTitle);
   ofn.lpstrInitialDir = TEXT("C:\\");
   ofn.lpstrTitle = TEXT("请选择要打开的文件");
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

   static HINTERNET hInternet = NULL;                           // Internet初始化句柄
   static HINTERNET hConnect = NULL;                            // FTP会话句柄

   TCHAR            szCurrentDirectory[MAX_PATH] = { 0 };
   DWORD            dwCurrentDirectory = _countof(szCurrentDirectory);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // 打开并初始化WinINet库
      hInternet = InternetOpen(TEXT("Mozilla/5.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
      // 连接到指定站点的FTP、HTTP(或HTTPS)服务
      hConnect = InternetConnect(hInternet, TEXT("127.0.0.1"), INTERNET_DEFAULT_FTP_PORT,
         TEXT("admin"), TEXT("123456"), INTERNET_SERVICE_FTP, 0, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_FILE, szFile);
         break;

      case IDC_BTN_PUTFILE:
         // 获取本地文件路径
         GetDlgItemText(hwndDlg, IDC_EDIT_FILE, szFile, _countof(szFile));

         // 上传本地文件到FTP服务器上
         FtpPutFile(hConnect, szFile, szFileTitle, FTP_TRANSFER_TYPE_BINARY, 0);
         break;

      case IDC_BTN_GETFILE:
         // 从FTP服务器下载一个文件到程序当前目录
         GetModuleFileName(NULL, szFile, _countof(szFile));
         StringCchCopy(_tcsrchr(szFile, TEXT('\\')) + 1, _tcslen(szFileTitle) + 1, szFileTitle);

         FtpGetFile(hConnect, szFileTitle, szFile, FALSE, FILE_ATTRIBUTE_NORMAL,
            FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RESYNCHRONIZE, 0);
         break;

      case IDC_BTN_CREATEDIRECTORY:
         // 在FTP服务器上创建一个新的目录
         FtpCreateDirectory(hConnect, TEXT("新建文件夹"));
         break;

      case IDC_BTN_REMOVEDIRECTORY:
         // 删除FTP服务器上的一个目录，注意，只能删除非空目录，无法删除当前目录
         FtpSetCurrentDirectory(hConnect, TEXT("/"));

         FtpRemoveDirectory(hConnect, TEXT("新建文件夹"));
         break;

      case IDC_BTN_SETDIRECTORY:
         // 设置FTP会话的当前目录
         FtpSetCurrentDirectory(hConnect, TEXT("新建文件夹"));

         FtpGetCurrentDirectory(hConnect, szCurrentDirectory, &dwCurrentDirectory);
         SetDlgItemText(hwndDlg, IDC_EDIT_DIRECTORY, szCurrentDirectory);
         break;

      case IDC_BTN_GETDIRECTORY:
         // 获取FTP会话的当前目录
         FtpGetCurrentDirectory(hConnect, szCurrentDirectory, &dwCurrentDirectory);

         SetDlgItemText(hwndDlg, IDC_EDIT_DIRECTORY, szCurrentDirectory);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      // 关闭相关句柄
      InternetCloseHandle(hConnect);
      InternetCloseHandle(hInternet);
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}