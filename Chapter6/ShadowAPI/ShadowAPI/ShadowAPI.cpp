#include <windows.h>
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
   typedef int (WINAPI* pfnMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
   pfnMessageBoxW pMessageBoxW = NULL;
   static pfnMessageBoxW pMessageBoxWNew = NULL;   // �����ڴ�ռ���MessageBoxW����������
   BYTE bArr[30] = { 0 };                          // ���MessageBoxW����ʵ�ִ���Ļ�����
   LPBYTE pMessageBoxTimeoutW = NULL;              // MessageBoxTimeoutW�����ĵ�ַ
   DWORD dwReplace;                                // MessageBoxTimeoutW��������Ե�ַ

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // ��ȡMessageBoxW�����ĵ�ַ������ȡ��������
      pMessageBoxW = (pfnMessageBoxW)GetProcAddress(GetModuleHandle(TEXT("User32.dll")),
         "MessageBoxW");
      ReadProcessMemory(GetCurrentProcess(), pMessageBoxW, bArr, sizeof(bArr), NULL);

      // �����ڴ�ռ���MessageBoxW�������ɶ���д��ִ��
      pMessageBoxWNew = (pfnMessageBoxW)VirtualAlloc(NULL, sizeof(bArr), MEM_RESERVE | MEM_COMMIT,
         PAGE_EXECUTE_READWRITE);
      WriteProcessMemory(GetCurrentProcess(), pMessageBoxWNew, bArr, sizeof(bArr), NULL);

      // ��ȡMessageBoxTimeoutW�����ĵ�ַ
      pMessageBoxTimeoutW = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("User32.dll")),
         "MessageBoxTimeoutW");

      // ���㲢�޸���Ե�ַ������pMessageBoxWNew����ƫ��22��DWORD����
      dwReplace = pMessageBoxTimeoutW - (LPBYTE)pMessageBoxWNew - 26;
      WriteProcessMemory(GetCurrentProcess(), (LPBYTE)pMessageBoxWNew + 22, &dwReplace,
         4, NULL);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OK:
         // ����ڵ������ж�MessageBoxW��������int3�ϵ㣬�п��ܵ�һ���ֽڱ��޸�Ϊ0xCC
         if (*(LPBYTE)pMessageBoxWNew == 0xCC)
            *(LPBYTE)pMessageBoxWNew = 0x8B;
         pMessageBoxWNew(hwndDlg, TEXT("����"), TEXT("����"), MB_OK);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}