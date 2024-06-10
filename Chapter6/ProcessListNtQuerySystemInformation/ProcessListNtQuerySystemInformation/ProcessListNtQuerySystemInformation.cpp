#include <windows.h>
#include <winternl.h>
#include "resource.h"

#pragma comment(lib, "Ntdll.lib")

// ��������
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
         // ��ȡ����Ļ�������С
         status = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &uReturnLength);

         // ������ʴ�С�Ļ������ٴε��ã���Ϊ���ε���֮������б�һ����ͬ�����Է���Ĵ�һ��
         lpSystemInformation = new BYTE[uReturnLength * 2];
         status = NtQuerySystemInformation(SystemProcessInformation, lpSystemInformation,
            uReturnLength * 2, &uReturnLength);
         if (!NT_SUCCESS(status))
            MessageBox(hwndDlg, TEXT("NtQuerySystemInformation����ʧ��"), TEXT("��ʾ"), MB_OK);

         // �������صĽ�����Ϣ�б�
         pSPI = (PSYSTEM_PROCESS_INFORMATION)lpSystemInformation;
         while (TRUE)
         {
            wsprintf(szBuf, TEXT("%d\t%s\r\n"), (UINT)(pSPI->UniqueProcessId), pSPI->ImageName.Buffer);
            SendMessage(hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // ����Ѿ��������
            if (pSPI->NextEntryOffset == 0)
               break;

            // ָ����һ��SYSTEM_PROCESS_INFORMATION�ṹ
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