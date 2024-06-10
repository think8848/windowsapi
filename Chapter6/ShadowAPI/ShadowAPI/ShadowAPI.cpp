#include <windows.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   typedef int (WINAPI* pfnMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
   pfnMessageBoxW pMessageBoxW = NULL;
   static pfnMessageBoxW pMessageBoxWNew = NULL;   // 分配内存空间存放MessageBoxW函数机器码
   BYTE bArr[30] = { 0 };                          // 存放MessageBoxW函数实现代码的缓冲区
   LPBYTE pMessageBoxTimeoutW = NULL;              // MessageBoxTimeoutW函数的地址
   DWORD dwReplace;                                // MessageBoxTimeoutW函数的相对地址

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // 获取MessageBoxW函数的地址，并读取函数数据
      pMessageBoxW = (pfnMessageBoxW)GetProcAddress(GetModuleHandle(TEXT("User32.dll")),
         "MessageBoxW");
      ReadProcessMemory(GetCurrentProcess(), pMessageBoxW, bArr, sizeof(bArr), NULL);

      // 分配内存空间存放MessageBoxW函数，可读可写可执行
      pMessageBoxWNew = (pfnMessageBoxW)VirtualAlloc(NULL, sizeof(bArr), MEM_RESERVE | MEM_COMMIT,
         PAGE_EXECUTE_READWRITE);
      WriteProcessMemory(GetCurrentProcess(), pMessageBoxWNew, bArr, sizeof(bArr), NULL);

      // 获取MessageBoxTimeoutW函数的地址
      pMessageBoxTimeoutW = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("User32.dll")),
         "MessageBoxTimeoutW");

      // 计算并修改相对地址，就是pMessageBoxWNew函数偏移22的DWORD数据
      dwReplace = pMessageBoxTimeoutW - (LPBYTE)pMessageBoxWNew - 26;
      WriteProcessMemory(GetCurrentProcess(), (LPBYTE)pMessageBoxWNew + 22, &dwReplace,
         4, NULL);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OK:
         // 如果在调试器中对MessageBoxW函数下了int3断点，有可能第一个字节被修改为0xCC
         if (*(LPBYTE)pMessageBoxWNew == 0xCC)
            *(LPBYTE)pMessageBoxWNew = 0x8B;
         pMessageBoxWNew(hwndDlg, TEXT("内容"), TEXT("标题"), MB_OK);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}