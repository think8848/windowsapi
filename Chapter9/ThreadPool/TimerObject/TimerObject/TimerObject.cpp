#include <windows.h>
#include <Commctrl.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 1�룬10000000��100����
const int nSecond = 10000000;

// ȫ�ֱ���
PTP_TIMER g_pTpTimer;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ��ʱ������ص�����
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
      // ����һ����ʱ������
      g_pTpTimer = CreateThreadpoolTimer(TimerCallback, NULL, NULL);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_SET:
      case IDC_BTN_RESET:
         //// �Զ���һ��ʱ��
         //st.wYear = 2021;
         //st.wMonth = 8;
         //st.wDay = 7;
         //st.wHour = 18;
         //st.wMinute = 28;
         //st.wSecond = 0;
         //st.wMilliseconds = 0;

         // ��ȡ����ʱ��ؼ���ʱ��
         SendDlgItemMessage(hwndDlg, IDC_DATETIMEPICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
         // ϵͳʱ��ת����FILETIMEʱ��
         SystemTimeToFileTime(&st, &ftLocal);
         // ����FILETIMEʱ��ת����UTC��FILETIMEʱ��
         LocalFileTimeToFileTime(&ftLocal, &ftUTC);

         // ���ü�ʱ������
         SetThreadpoolTimer(g_pTpTimer, &ftUTC, 5000, 10);

         //// ���ʱ��ļ�ʱ������
         //uli.QuadPart = (ULONGLONG)-(10 * nSecond);
         //ftUTC.dwHighDateTime = uli.HighPart;
         //ftUTC.dwLowDateTime = uli.LowPart;
         //SetThreadpoolTimer(g_pTpTimer, &ftUTC, 5000, 10);
         break;

      case IDC_BTN_STOP:
         // ֹͣ��ʱ������
         SetThreadpoolTimer(g_pTpTimer, NULL, 0, 0);
         break;

      case IDCANCEL:
         // �رղ��ͷż�ʱ������
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