#include <windows.h>
#include <Commctrl.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 1秒，10000000个100纳秒
const int nSecond = 10000000;

// 全局变量
PTP_TIMER g_pTpTimer;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 计时器对象回调函数
VOID CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   SYSTEMTIME st = { 0 };
   FILETIME ftLocal = { 0 }, ftUTC = { 0 };
   //ULARGE_INTEGER uli = { 0 };

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // 创建一个计时器对象
      g_pTpTimer = CreateThreadpoolTimer(TimerCallback, NULL, NULL);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_SET:
      case IDC_BTN_RESET:
         //// 自定义一个时间
         //st.wYear = 2021;
         //st.wMonth = 8;
         //st.wDay = 7;
         //st.wHour = 18;
         //st.wMinute = 28;
         //st.wSecond = 0;
         //st.wMilliseconds = 0;

         // 获取日期时间控件的时间
         SendDlgItemMessage(hwndDlg, IDC_DATETIMEPICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
         // 系统时间转换成FILETIME时间
         SystemTimeToFileTime(&st, &ftLocal);
         // 本地FILETIME时间转换成UTC的FILETIME时间
         LocalFileTimeToFileTime(&ftLocal, &ftUTC);

         // 设置计时器对象
         SetThreadpoolTimer(g_pTpTimer, &ftUTC, 5000, 10);

         //// 相对时间的计时器对象
         //uli.QuadPart = (ULONGLONG)-(10 * nSecond);
         //ftUTC.dwHighDateTime = uli.HighPart;
         //ftUTC.dwLowDateTime = uli.LowPart;
         //SetThreadpoolTimer(g_pTpTimer, &ftUTC, 5000, 10);
         break;

      case IDC_BTN_STOP:
         // 停止计时器对象
         SetThreadpoolTimer(g_pTpTimer, NULL, 0, 0);
         break;

      case IDCANCEL:
         // 关闭并释放计时器对象
         SetThreadpoolTimer(g_pTpTimer, NULL, 0, 0);
         WaitForThreadpoolTimerCallbacks(g_pTpTimer, TRUE);
         CloseThreadpoolTimer(g_pTpTimer);
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

VOID CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
   ShellExecute(NULL, TEXT("open"), TEXT("Calc.exe"), NULL, NULL, SW_SHOW);
}