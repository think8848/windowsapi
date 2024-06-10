#include <windows.h>
#include "resource.h"

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   TCHAR szRundll32Path[MAX_PATH] = TEXT("C:\\Windows\\System32\\rundll32.exe");
   TCHAR szDllPath[MAX_PATH] = TEXT("F:\\Source\\Windows\\Chapter20\\PassUAC\\Debug\\PassUAC.dll");
   TCHAR szExcuteFileName[MAX_PATH] = TEXT("F:\\Source\\Windows\\Chapter20\\ProcessList.exe");
   TCHAR szParameters[MAX_PATH] = { 0 };

   switch (uMsg)
   {
   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_CALL:
         wsprintf(szParameters, TEXT("\"%s\" %s \"%s\""), szDllPath, TEXT("PassUAC"), szExcuteFileName);
         ShellExecute(NULL, TEXT("open"), szRundll32Path, szParameters, NULL, SW_SHOW);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}