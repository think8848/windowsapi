#include <windows.h>
#include <Shlwapi.h>
#include <delayimp.h>
#include "resource.h"
#include "GetMd5.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "GetMd5.lib")

// ȫ�ֱ���
HINSTANCE g_hInstance;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   HRSRC hResBlock;
   HANDLE hRes;
   LPVOID lpDll;
   DWORD dwDllSize;
   HANDLE hFile;
   TCHAR szFileName[MAX_PATH] = { 0 };
   TCHAR szMd5[64] = { 0 };

   HDROP hDrop;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // ����ChangeWindowMessageFilterEx��������μ��û�������Ȩ����һ��
      ChangeWindowMessageFilterEx(hwndDlg, WM_DROPFILES, MSGFLT_ALLOW, NULL);
      ChangeWindowMessageFilterEx(hwndDlg, 0x49, MSGFLT_ALLOW, NULL);// 0x49 == WM_COPYGLOBALDATA

       //ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
       //ChangeWindowMessageFilter(0x49, MSGFLT_ADD);          

        // �����ǰĿ¼�²�����GetMd5.dll
      if (!PathFileExists(TEXT("GetMd5.dll")))
      {
         hResBlock = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_MYDLL), TEXT("MyDll"));
         if (!hResBlock)
            return FALSE;
         hRes = LoadResource(g_hInstance, hResBlock);
         if (!hRes)
            return FALSE;
         lpDll = LockResource(hRes);
         dwDllSize = SizeofResource(g_hInstance, hResBlock);

         hFile = CreateFile(TEXT("GetMd5.dll"), GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;
         WriteFile(hFile, lpDll, dwDllSize, NULL, NULL);

         CloseHandle(hFile);
      }
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_GETMD5:
         // ��ȡָ���ļ���MD5
         if (GetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFileName, _countof(szFileName)))
         {
            if (GetMd5(szFileName, szMd5))
            {
               SetDlgItemText(hwndDlg, IDC_EDIT_MD5, szMd5);
               // ж���ӳټ��ص�dll
               __FUnloadDelayLoadedDLL2("GetMd5.dll");
            }
         }
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;

   case WM_DROPFILES:
      hDrop = (HDROP)wParam;
      DragQueryFile(hDrop, 0, szFileName, _countof(szFileName));
      SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFileName);
      DragFinish(hDrop);
      return FALSE;
   }

   return FALSE;
}