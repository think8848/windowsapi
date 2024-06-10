#include <windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include "..\..\..\Detours-master\include\detours.h"
#include "resource.h"

// ����Ϊx86ʱ��Ҫʹ�õ�.lib
#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X86\\detours.lib")
// ����Ϊx64ʱ��Ҫʹ�õ�.lib
//#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X64\\detours.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ��������
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
   CHAR szInjectDll[MAX_PATH] = { 0 };       // ע��dll·��
   TCHAR szTargetProcess[MAX_PATH] = { 0 };  // Ŀ�����·��
   STARTUPINFO si = { sizeof(STARTUPINFO) };
   PROCESS_INFORMATION pi = { 0 };
   BOOL bRet = FALSE;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      hwndComboDllPath = GetDlgItem(hwndDlg, IDC_COMBO_DLLPATH);
      hwndComboTarget = GetDlgItem(hwndDlg, IDC_COMBO_TARGET);

      // ע��dll��Ͽ����һЩ�б���
      SendMessage(hwndComboDllPath, CB_ADDSTRING, 0, (LPARAM)TEXT("InjectDll32.dll"));
      SendMessage(hwndComboDllPath, CB_ADDSTRING, 0, (LPARAM)TEXT("InjectDll64.dll"));
      SendMessage(hwndComboDllPath, CB_SETCURSEL, 0, 0);

      // Ŀ�������Ͽ����һЩ�б���
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
            MessageBox(hwndDlg, TEXT("����Ŀ�����ʧ�ܣ�"), TEXT("������ʾ"), MB_OK);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}