#include <windows.h>
#include "resource.h"

// 函数声明
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
   static LPVOID lpPopad, lpPatch;// popad地址(基地址 + 0x18C4E)，补丁地址(基地址 + 0x1000 + 0x9)
   BYTE bInt3 = 0xCC;             // popad指令地址处写入int 3指令的机器码0xCC
   BYTE bOld = 0x61;              // 恢复popad指令的机器码
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
            // 等待调试事件发生
            WaitForDebugEvent(&debugEvent, INFINITE);

            // 进程被创建
            if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
            {
               // lpPopad, lpPatch
               lpPopad = (LPBYTE)(debugEvent.u.CreateProcessInfo.lpBaseOfImage) + 0x18C4E;
               lpPatch = (LPBYTE)(debugEvent.u.CreateProcessInfo.lpBaseOfImage) + 0x1000 + 0x9;
               // popad指令处下int 3断点
               WriteProcessMemory(pi.hProcess, lpPopad, &bInt3, 1, NULL);

               CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
            }

            // 被调试进程中发生异常事件
            else if (debugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
            {
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode)
               {
               case EXCEPTION_BREAKPOINT:          // 断点中断
                  context.ContextFlags = CONTEXT_CONTROL;
                  GetThreadContext(pi.hThread, &context);
                  // int 3指令执行以后才会发生异常，这时候eip已经指向了下一条指令
                  if (context.Eip == (DWORD)((LPBYTE)lpPopad + 1))
                  {
                     // popad指令处的int 3断点改回原popad指令
                     WriteProcessMemory(pi.hProcess, lpPopad, &bOld, 1, NULL);
                     // 内存补丁，JE指令修改为两个NOP指令
                     WriteProcessMemory(pi.hProcess, lpPatch, &wCodeNew, 2, NULL);

                     // 重新执行popad指令
                     context.Eip -= 1;
                     SetThreadContext(pi.hThread, &context);
                  }
                  break;

               case EXCEPTION_SINGLE_STEP:         // 单步中断
                  break;
               }

            }

            // 被调试进程退出
            else if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            {
               break;
            }

            // 处理完一个调试事件以后调用ContinueDebugEvent恢复线程执行并继续等待下个事件
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