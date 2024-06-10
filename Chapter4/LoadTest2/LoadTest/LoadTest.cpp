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
   TCHAR szCommandLine[MAX_PATH] = TEXT("Test.exe");
   STARTUPINFO si = { sizeof(STARTUPINFO) };
   PROCESS_INFORMATION pi = { 0 };
   LPVOID lpBaseAddr;
   WORD wCodeOld, wCodeNew = 0x9090;


   switch (uMsg)
   {
   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_LOADTEST:
         GetStartupInfo(&si);
         if (!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS,
            NULL, NULL, &si, &pi))
            break;

         DEBUG_EVENT debugEvent;
         while (TRUE)
         {
            // �ȴ������¼�����
            WaitForDebugEventEx(&debugEvent, INFINITE);

            // ��������¼�
            if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
            {
               lpBaseAddr = (LPBYTE)(debugEvent.u.CreateProcessInfo.lpBaseOfImage) + 0x1009;
               if (ReadProcessMemory(pi.hProcess, lpBaseAddr, &wCodeOld, sizeof(WORD), NULL))
               {
                  // Ŀ�����lpBaseAddr��ַ�������������Ƿ�Ϊ0x1774����������滻
                  if (wCodeOld == 0x1774)
                  {
                     WriteProcessMemory(pi.hProcess, lpBaseAddr, &wCodeNew, sizeof(WORD), NULL);
                  }
                  else
                  {
                     MessageBox(hwndDlg, TEXT("Ŀ������汾����"), TEXT("��ʾ"), MB_OK);
                     TerminateProcess(pi.hProcess, 0);
                  }
               }

               CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
            }

            else if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            {
               MessageBox(hwndDlg, TEXT("�����Խ����˳�"), TEXT("��ʾ"), MB_OK);
               break;
            }

            // ������һ�������¼��Ժ����ContinueDebugEvent�ָ��߳�ִ�в������ȴ��¸������¼�
            ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
         }

         CloseHandle(pi.hThread);
         CloseHandle(pi.hProcess);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}