#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static TCHAR szFileName[MAX_PATH] = { 0 };          // INI�ļ�����
   LPCTSTR lpAppName = TEXT("INIDemoPositionSize");    // С������
   LPCTSTR lpKeyNameX = TEXT("X");
   LPCTSTR lpKeyNameY = TEXT("Y");
   LPCTSTR lpKeyNameWidth = TEXT("Width");
   LPCTSTR lpKeyNameHeight = TEXT("Height");
   UINT unX = 0, unY = 0, unWidth = 0, unHeight = 0;
   RECT rect;
   TCHAR szBuf[16] = { 0 };

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // ��ȡ��ǰ���̵Ŀ�ִ���ļ�����·����Ȼ��ƴ�ӳ�INI�ļ�����·��
      GetModuleFileName(NULL, szFileName, _countof(szFileName));
      StringCchCopy(_tcsrchr(szFileName, TEXT('\\')) + 1, _countof(szFileName), TEXT("INIDemo.ini"));

      // ��ȡX��Y��Width��Height���ļ�ֵ
      unX = GetPrivateProfileInt(lpAppName, lpKeyNameX, NULL, szFileName);
      unY = GetPrivateProfileInt(lpAppName, lpKeyNameY, NULL, szFileName);
      unWidth = GetPrivateProfileInt(lpAppName, lpKeyNameWidth, NULL, szFileName);
      unHeight = GetPrivateProfileInt(lpAppName, lpKeyNameHeight, NULL, szFileName);

      // ���ó��򴰿�λ�á���С
      if (unWidth && unHeight)
         SetWindowPos(hwndDlg, HWND_TOP, unX, unY, unWidth, unHeight, SWP_SHOWWINDOW);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDCANCEL:
         // ������򴰿�λ�á���С
         GetWindowRect(hwndDlg, &rect);
         wsprintf(szBuf, TEXT("%d"), rect.left);
         WritePrivateProfileString(lpAppName, lpKeyNameX, szBuf, szFileName);
         wsprintf(szBuf, TEXT("%d"), rect.top);
         WritePrivateProfileString(lpAppName, lpKeyNameY, szBuf, szFileName);
         wsprintf(szBuf, TEXT("%d"), rect.right - rect.left);
         WritePrivateProfileString(lpAppName, lpKeyNameWidth, szBuf, szFileName);
         wsprintf(szBuf, TEXT("%d"), rect.bottom - rect.top);
         WritePrivateProfileString(lpAppName, lpKeyNameHeight, szBuf, szFileName);

         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}