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
   TCHAR szCommandLine[MAX_PATH] = TEXT("Test_UPX.exe");
   STARTUPINFO si = { sizeof(STARTUPINFO) };
   PROCESS_INFORMATION pi = { 0 };
   static LPVOID lpPopad, lpPatch;// popad��ַ(����ַ + 0x18C4E)��������ַ(����ַ + 0x1000 + 0x9)
   BYTE bInt3 = 0xCC;             // popadָ���ַ��д��int 3ָ��Ļ�����0xCC
   BYTE bOld = 0x61;              // �ָ�popadָ��Ļ�����
   WORD wCodeNew = 0x9090;
   DEBUG_EVENT debugEvent;
   CONTEXT context;

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

         while (TRUE)
         {
            // �ȴ������¼�����
            WaitForDebugEvent(&debugEvent, INFINITE);

            // ���̱�����
            if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
            {
               // lpPopad, lpPatch
               lpPopad = (LPBYTE)(debugEvent.u.CreateProcessInfo.lpBaseOfImage) + 0x18C4E;
               lpPatch = (LPBYTE)(debugEvent.u.CreateProcessInfo.lpBaseOfImage) + 0x1000 + 0x9;
               // popadָ���int 3�ϵ�
               WriteProcessMemory(pi.hProcess, lpPopad, &bInt3, 1, NULL);

               CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
            }

            // �����Խ����з����쳣�¼�
            else if (debugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
            {
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode)
               {
               case EXCEPTION_BREAKPOINT:          // �ϵ��ж�
                  context.ContextFlags = CONTEXT_CONTROL;
                  GetThreadContext(pi.hThread, &context);
                  // int 3ָ��ִ���Ժ�Żᷢ���쳣����ʱ��eip�Ѿ�ָ������һ��ָ��
                  if (context.Eip == (DWORD)((LPBYTE)lpPopad + 1))
                  {
                     // popadָ���int 3�ϵ�Ļ�ԭpopadָ��
                     WriteProcessMemory(pi.hProcess, lpPopad, &bOld, 1, NULL);
                     // �ڴ油����JEָ���޸�Ϊ����NOPָ��
                     WriteProcessMemory(pi.hProcess, lpPatch, &wCodeNew, 2, NULL);

                     // ����ִ��popadָ��
                     context.Eip -= 1;
                     SetThreadContext(pi.hThread, &context);
                  }
                  break;

               case EXCEPTION_SINGLE_STEP:         // �����ж�
                  break;
               }

            }

            // �����Խ����˳�
            else if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            {
               break;
            }

            // ������һ�������¼��Ժ����ContinueDebugEvent�ָ��߳�ִ�в������ȴ��¸��¼�
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