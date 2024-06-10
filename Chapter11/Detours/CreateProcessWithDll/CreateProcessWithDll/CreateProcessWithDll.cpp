#include <windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include "..\..\..\Detours-master\include\detours.h"
#include "resource.h"

// 编译为x86时需要使用的.lib
#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X86\\detours.lib")
// 编译为x64时需要使用的.lib
//#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X64\\detours.lib")

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
   HWND hwndComboDllPath;
   HWND hwndComboTarget;
   CHAR szInjectDll[MAX_PATH] = { 0 };       // 注入dll路径
   TCHAR szTargetProcess[MAX_PATH] = { 0 };  // 目标程序路径
   STARTUPINFO si = { sizeof(STARTUPINFO) };
   PROCESS_INFORMATION pi = { 0 };
   BOOL bRet = FALSE;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      hwndComboDllPath = GetDlgItem(hwndDlg, IDC_COMBO_DLLPATH);
      hwndComboTarget = GetDlgItem(hwndDlg, IDC_COMBO_TARGET);

      // 注入dll组合框添加一些列表项
      SendMessage(hwndComboDllPath, CB_ADDSTRING, 0, (LPARAM)TEXT("InjectDll32.dll"));
      SendMessage(hwndComboDllPath, CB_ADDSTRING, 0, (LPARAM)TEXT("InjectDll64.dll"));
      SendMessage(hwndComboDllPath, CB_SETCURSEL, 0, 0);

      // 目标程序组合框添加一些列表项
      SendMessage(hwndComboTarget, CB_ADDSTRING, 0, (LPARAM)TEXT("FloatingWaterMark32.exe"));
      SendMessage(hwndComboTarget, CB_ADDSTRING, 0, (LPARAM)TEXT("FloatingWaterMark64.exe"));
      SendMessage(hwndComboTarget, CB_SETCURSEL, 0, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_CREATE:
         GetDlgItemTextA(hwndDlg, IDC_COMBO_DLLPATH, szInjectDll, _countof(szInjectDll));
         GetDlgItemText(hwndDlg, IDC_COMBO_TARGET, szTargetProcess, _countof(szTargetProcess));

         GetStartupInfo(&si);
         bRet = DetourCreateProcessWithDllEx(NULL, szTargetProcess, NULL, NULL, FALSE, 0,
            NULL, NULL, &si, &pi, szInjectDll, NULL);
         if (!bRet)
            MessageBox(hwndDlg, TEXT("创建目标进程失败！"), TEXT("错误提示"), MB_OK);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}