#include <windows.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HWND     g_hwndDlg;
PTP_WORK g_pTpWork;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 工作对象回调函数
VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      // 创建一个工作对象
      g_pTpWork = CreateThreadpoolWork(WorkCallback, NULL, NULL);
      if (g_pTpWork == NULL)
         ExitProcess(0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_SUBMIT:
         SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_RESETCONTENT, 0, 0);

         // 提交工作对象
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      // 关闭工作对象
      if (g_pTpWork != NULL)
         CloseThreadpoolWork(g_pTpWork);
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}

VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
   DWORD dwThreadId = GetCurrentThreadId();
   TCHAR szBuf[64] = { 0 };

   StringCchPrintf(szBuf, _countof(szBuf), TEXT("线程ID[%d]\t开始工作"), dwThreadId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_ADDSTRING, 0, (LPARAM)szBuf);

   Sleep(1000);

   StringCchPrintf(szBuf, _countof(szBuf), TEXT("线程ID[%d]\t工作完成"), dwThreadId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_ADDSTRING, 0, (LPARAM)szBuf);
}