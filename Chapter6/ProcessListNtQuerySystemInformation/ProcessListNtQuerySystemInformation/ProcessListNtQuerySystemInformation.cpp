#include <windows.h>
#include <winternl.h>
#include "resource.h"

#pragma comment(lib, "Ntdll.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HWND hwndEdit;
   LPVOID lpSystemInformation = NULL;
   PSYSTEM_PROCESS_INFORMATION pSPI = NULL;
   ULONG uReturnLength = 0;
   NTSTATUS status;
   TCHAR szBuf[64] = { 0 };

   switch (uMsg)
   {
   case WM_INITDIALOG:
      hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_PROCESSLIST);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_GET:
         // 获取所需的缓冲区大小
         status = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &uReturnLength);

         // 分配合适大小的缓冲区再次调用，因为两次调用之间进程列表不一定相同，所以分配的大一倍
         lpSystemInformation = new BYTE[uReturnLength * 2];
         status = NtQuerySystemInformation(SystemProcessInformation, lpSystemInformation,
            uReturnLength * 2, &uReturnLength);
         if (!NT_SUCCESS(status))
            MessageBox(hwndDlg, TEXT("NtQuerySystemInformation调用失败"), TEXT("提示"), MB_OK);

         // 遍历返回的进程信息列表
         pSPI = (PSYSTEM_PROCESS_INFORMATION)lpSystemInformation;
         while (TRUE)
         {
            wsprintf(szBuf, TEXT("%d\t%s\r\n"), (UINT)(pSPI->UniqueProcessId), pSPI->ImageName.Buffer);
            SendMessage(hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // 如果已经遍历完毕
            if (pSPI->NextEntryOffset == 0)
               break;

            // 指向下一个SYSTEM_PROCESS_INFORMATION结构
            pSPI = (PSYSTEM_PROCESS_INFORMATION)((LPBYTE)pSPI + pSPI->NextEntryOffset);
         }

         delete[]lpSystemInformation;
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}