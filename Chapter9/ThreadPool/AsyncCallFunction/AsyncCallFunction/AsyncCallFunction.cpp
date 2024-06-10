#include <windows.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ȫ�ֱ���
HWND     g_hwndDlg;
PTP_WORK g_pTpWork;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ��������ص�����
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

      // ����һ����������
      g_pTpWork = CreateThreadpoolWork(WorkCallback, NULL, NULL);
      if (g_pTpWork == NULL)
         ExitProcess(0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_SUBMIT:
         SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_RESETCONTENT, 0, 0);

         // �ύ��������
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         SubmitThreadpoolWork(g_pTpWork);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      // �رչ�������
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

   StringCchPrintf(szBuf, _countof(szBuf), TEXT("�߳�ID[%d]\t��ʼ����"), dwThreadId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_ADDSTRING, 0, (LPARAM)szBuf);

   Sleep(1000);

   StringCchPrintf(szBuf, _countof(szBuf), TEXT("�߳�ID[%d]\t�������"), dwThreadId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_INFO), LB_ADDSTRING, 0, (LPARAM)szBuf);
}